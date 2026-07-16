#include "domain/CombatSystem.h"

#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"

#include <QLineF>
#include <algorithm>

namespace skybound {
namespace {
/// 从角色哈希表中查找玩家对象
CharacterObject* player(QHash<QString, CharacterObject>& characters) {
    auto it = characters.find(QStringLiteral("player"));
    return it == characters.end() ? nullptr : &it.value();
}

/// 检测线段是否被地形阻挡
/// 用多点采样替代 line-rect 边缘相交检测，防止细长地形块的角落穿透
/// 在 from→to 连线上均匀采样 N 个点，检查是否有任何点落在 solid 地形内部
bool isLineBlockedByTerrain(const QPointF& from, const QPointF& to, const QList<TerrainPiece>& terrain) {
    for (const auto& piece : terrain) {
        if (!piece.solid) continue;
        const QRectF& r = piece.rect;
        // 粗略包围盒过滤：地形完全在两端之外则跳过
        if (r.right() < std::min(from.x(), to.x()) || r.left() > std::max(from.x(), to.x())) {
            continue;
        }
        if (r.bottom() < std::min(from.y(), to.y()) || r.top() > std::max(from.y(), to.y())) {
            continue;
        }
        // 多点采样：在连线上均匀取 8 个点，检查是否落在 terrain 矩形内
        constexpr int kSamples = 8;
        for (int i = 0; i <= kSamples; ++i) {
            const qreal t = qreal(i) / qreal(kSamples);
            const QPointF pt(
                from.x() + (to.x() - from.x()) * t,
                from.y() + (to.y() - from.y()) * t);
            if (r.contains(pt)) {
                return true;
            }
        }
    }
    return false;
}
} // namespace

CombatResult CombatSystem::applyDamageToPlayer(CharacterObject& playerCharacter, int damage, const QPointF& attackerPos) {
    CombatResult result;
    if (!playerCharacter.alive) {
        return result;
    }

    playerCharacter.hp = std::max(0, playerCharacter.hp - std::max(0, damage));
    result.playerStatsChanged = true;
    if (playerCharacter.hp == 0) {
        // 玩家死亡
        playerCharacter.alive = false;
        playerCharacter.moveDirection = 0;
        playerCharacter.velocity = QPointF(0, 0);
        playerCharacter.attackBox.active = false;
        playerCharacter.hurtbox.active = false;
        playerCharacter.attackVfxKey.clear();
        CharacterSystem::setState(playerCharacter, QStringLiteral("dead"), 1, 90);
        result.sounds.push_back(QStringLiteral("player.dead"));
        return result;
    }

    // 玩家受伤后的受击状态 + 击退
    if (playerCharacter.state == QStringLiteral("hit")) {
        playerCharacter.state = QString();
        playerCharacter.actionDurationMs = 0;
    }
    CharacterSystem::setState(playerCharacter, QStringLiteral("hit"), 4, 90, 360);
    // 击退：远离攻击者
    const qreal knockDir = (attackerPos.x() < playerCharacter.position.x()) ? 1.0 : -1.0;
    playerCharacter.velocity.setX(knockDir * 8.0);
    playerCharacter.velocity.setY(-3.0);
    result.sounds.push_back(QStringLiteral("player.hurt"));
    return result;
}

CombatResult CombatSystem::checkAttackHits(
    CharacterObject& attacker,
    QHash<QString, CharacterObject>& characters,
    const QList<TerrainPiece>& terrain,
    QSet<QString>& resolvedAttackTokens) {
    CombatResult result;
    if (!attacker.alive || !CollisionSystem::canUseBox(attacker.attackBox)) {
        return result;
    }

    const QString token = QStringLiteral("%1-attack-%2").arg(attacker.id).arg(attacker.attackSerial);
    if (resolvedAttackTokens.contains(token)) {
        return result;
    }

    if (attacker.kind == QStringLiteral("player")) {
        // ── 玩家攻击 → 检查是否命中敌人 ──
        for (auto it = characters.begin(); it != characters.end(); ++it) {
            auto& target = it.value();
            if (target.id == attacker.id || target.kind != QStringLiteral("enemy") || !target.alive || !CollisionSystem::canUseBox(target.hurtbox)) {
                continue;
            }
            if (!attacker.attackBox.rect.intersects(target.hurtbox.rect)) {
                continue;
            }

            // 穿墙检测：攻击者到目标的连线被地形阻挡则跳过
            {
                const QPointF attackerCenter(attacker.position.x() + attacker.charWidth / 2.0, attacker.position.y() + attacker.charHeight / 2.0);
                const QPointF targetCenter(target.position.x() + target.charWidth / 2.0, target.position.y() + target.charHeight / 2.0);
                if (isLineBlockedByTerrain(attackerCenter, targetCenter, terrain)) {
                    continue;
                }
            }

            // ──────────────────────────────────────────────
            // 蜗牛受击：先给击退，再触发缩壳动画
            // ──────────────────────────────────────────────
            if (target.animationFamily == QStringLiteral("snail")) {
                // 击退：远离攻击者
                {
                    const qreal knockDir = (attacker.position.x() < target.position.x()) ? 1.0 : -1.0;
                    target.velocity.setX(knockDir * 6.0);
                    target.velocity.setY(-3.0);
                }
                CharacterSystem::setState(target, QStringLiteral("hide"), 12, 60, 480);  // 缩壳 480ms
                result.sounds.push_back(QStringLiteral("enemy.hurt.1"));                 // 播放受击音效（无伤害）
                return result;
            }

            // 普通敌人 — 扣血逻辑 + 击退
            resolvedAttackTokens.insert(token);
            target.hp = std::max(0, target.hp - attacker.attackDamage);
            result.damageCountDelta = 1;
            // 击退：远离攻击者
            {
                const qreal knockDir = (attacker.position.x() < target.position.x()) ? 1.0 : -1.0;
                target.velocity.setX(knockDir * 8.0);
                target.velocity.setY(-3.0);
            }
            if (target.hp == 0) {
                target.alive = false;
                CharacterSystem::setState(target, QStringLiteral("dead"), 1, 90);
            } else {
                CharacterSystem::setState(target, QStringLiteral("hit"), target.animationFamily == QStringLiteral("small_bee") ? 4 : 1, 90, 360);
            }
            result.sounds.push_back(QStringLiteral("enemy.hurt.1"));
            return result;
        }
        return result;
    }

    // ── 敌人攻击 → 检查是否命中玩家 ──
    auto* p = player(characters);
    if (!p || !p->alive || !CollisionSystem::canUseBox(p->hurtbox) || !attacker.attackBox.rect.intersects(p->hurtbox.rect)) {
        return result;
    }

    // 穿墙检测（敌人攻击玩家）
    {
        const QPointF attackerCenter(attacker.position.x() + attacker.charWidth / 2.0, attacker.position.y() + attacker.charHeight / 2.0);
        const QPointF playerCenter(p->position.x() + p->charWidth / 2.0, p->position.y() + p->charHeight / 2.0);
        if (isLineBlockedByTerrain(attackerCenter, playerCenter, terrain)) {
            return result;
        }
    }

    resolvedAttackTokens.insert(token);
    return applyDamageToPlayer(*p, attacker.attackDamage, attacker.position);
}

// ──────────────────────────────────────────────
// 蜗牛接触伤害检查
//
// 蜗牛不主动攻击，而是通过身体触碰玩家来造成伤害。
// 该函数遍历所有存活的蜗牛：
//   1. 跳过缩壳（hide）/ 死亡 / 冷却中的蜗牛
//   2. 检查蜗牛 hurtbox 与玩家 hurtbox 是否重叠
//   3. 重叠 → 对玩家造成伤害，并设置冷却
//
// 冷却由 NpcSystem::updateSnail 每帧递减。
// ──────────────────────────────────────────────
CombatResult CombatSystem::checkContactDamage(
    QHash<QString, CharacterObject>& characters) {
    CombatResult result;
    auto* p = player(characters);
    if (!p || !p->alive) {
        return result;
    }

    for (auto it = characters.begin(); it != characters.end(); ++it) {
        auto& character = it.value();
        if (character.animationFamily != QStringLiteral("snail") || !character.alive) {
            continue;
        }
        if (character.state == QStringLiteral("hide") || character.state == QStringLiteral("dead")) {
            continue;  // 缩壳或死亡状态下不造成伤害
        }
        if (character.attackCooldownRemainingMs > 0) {
            continue;  // 冷却中，不重复扣血
        }

        // 蜗牛 hurtbox 与玩家 hurtbox 是否重叠（碰撞检测）
        if (!CollisionSystem::canUseBox(character.hurtbox) || !CollisionSystem::canUseBox(p->hurtbox)) {
            continue;
        }
        if (!character.hurtbox.rect.intersects(p->hurtbox.rect)) {
            continue;
        }

        // 造成接触伤害并进入冷却
        character.attackCooldownRemainingMs = character.attackCooldownMs;
        result = applyDamageToPlayer(*p, character.attackDamage, character.position);
        break;  // 每帧只触发一次接触伤害
    }
    return result;
}

// ──────────────────────────────────────────────
// 踩踏击杀检测
//
// 蜗牛唯一的消灭方式：玩家从上方落到蜗牛壳顶时踩踏击杀。
// 实现原理：
//   1. 玩家必须正在下落（velocity.y > 0）
//   2. 玩家脚底（pos.y + charHeight）与蜗牛 hurtbox 顶部水平重叠
//   3. 玩家上一帧的脚底 Y 在蜗牛头顶上方，
//      本帧的脚底 Y 落到了蜗牛头顶位置
//   4. 满足条件 → 蜗牛死亡 + 玩家向上弹跳（模拟"踩"的动作）
//
// 使用 previousPlayerBottom 来防止玩家站在蜗牛上时每帧触发。
// ──────────────────────────────────────────────
CombatResult CombatSystem::checkStompKill(
    QHash<QString, CharacterObject>& characters) {
    CombatResult result;
    auto* p = player(characters);
    if (!p || !p->alive || p->velocity.y() <= 0) {
        return result;  // 玩家不处于下落状态 → 不可能踩踏
    }

    // 计算玩家本帧与上一帧的脚底 Y 坐标
    const qreal playerBottom = p->position.y() + p->charHeight;
    const qreal previousPlayerBottom = playerBottom - p->velocity.y();  // 减去本帧位移 ≈ 上一帧位置

    for (auto it = characters.begin(); it != characters.end(); ++it) {
        auto& snail = it.value();
        if (snail.animationFamily != QStringLiteral("snail") || !snail.alive) {
            continue;
        }
        if (!CollisionSystem::canUseBox(snail.hurtbox)) {
            continue;
        }

        const qreal snailTop = snail.hurtbox.rect.top();

        // 水平方向是否有重叠（玩家脚底在蜗牛正上方）
        const bool horizontalOverlap = p->hurtbox.rect.right() > snail.hurtbox.rect.left()
                                    && p->hurtbox.rect.left() < snail.hurtbox.rect.right();
        if (!horizontalOverlap) {
            continue;
        }

        // 玩家上一帧脚底在蜗牛头顶上方，且本帧脚底碰到/穿过蜗牛头顶
        if (previousPlayerBottom <= snailTop && playerBottom >= snailTop) {
            // 踩踏击杀
            snail.alive = false;
            snail.hp = 0;
            snail.velocity = QPointF(0, 0);
            snail.hurtbox.active = false;
            CharacterSystem::setState(snail, QStringLiteral("dead"), 12, 60, 240);  // 播放死亡动画

            // 玩家获得向上的反弹力（"踩"的反作用力）
            p->velocity.setY(-8.0);

            result.damageCountDelta = 1;       // 计入击杀计数
            result.sounds.push_back(QStringLiteral("enemy.hurt.1"));
            result.playerStatsChanged = true;
            break;
        }
    }
    return result;
}

} // namespace skybound
