#include "domain/NpcSystem.h"

#include "domain/CharacterSystem.h"

#include <algorithm>
#include <cmath>

namespace skybound {

// ──────────────────────────────────────────────
// 蜗牛 AI 更新
//
// 蜗牛的移动逻辑：
//   1. 被攻击（hide / hit）或玩家死亡时原地不动
//   2. 玩家在检测范围内时朝玩家缓步靠近
//   3. 距离玩家 < 20px 时停下，防止左右反复横跳
//   4. 距离玩家 > detectionRange 时静止
//
// 额外职责：递减 attackCooldownRemainingMs，
// 供 CombatSystem::checkContactDamage 判断接触伤害冷却。
// ──────────────────────────────────────────────
void NpcSystem::updateSnail(CharacterObject& npc, const CharacterObject* player, int deltaMs, const WorldTuning& tuning) {
    Q_UNUSED(tuning);
    // 递减攻击冷却（用于接触伤害），确保每帧都在减少
    if (npc.attackCooldownRemainingMs > 0) {
        npc.attackCooldownRemainingMs = std::max(0, npc.attackCooldownRemainingMs - deltaMs);
    }

    // 玩家不存在 / 玩家死亡 / 蜗牛正在缩壳或受击 → 静止
    if (!player || !player->alive || npc.state == QStringLiteral("hide") || npc.state == QStringLiteral("hit")) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    // 计算玩家中心与蜗牛中心的水平距离
    const qreal playerCenterX = player->position.x() + player->charWidth / 2.0;
    const qreal npcCenterX = npc.position.x() + npc.charWidth / 2.0;
    const qreal dx = playerCenterX - npcCenterX;
    const qreal distance = std::abs(dx);

    // 超出检测范围 → 静止
    if (distance > npc.detectionRange) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    // 最小停止距离：防止蜗牛贴脸时方向频繁翻转造成振荡
    if (distance < 20.0) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    // 朝玩家方向移动
    npc.facingLeft = dx < 0;
    npc.moveDirection = dx < 0 ? -1 : 1;
    npc.velocity.setX(npc.moveDirection * npc.npcMoveSpeed);
}

void NpcSystem::updateNpc(CharacterObject& npc, const CharacterObject* player, int deltaMs, const WorldTuning& tuning, WorldEvents& events) {
    // 蜗牛走自己的 AI 分支（行为与蜜蜂完全不同）
    if (npc.animationFamily == QStringLiteral("snail")) {
        updateSnail(npc, player, deltaMs, tuning);
        return;
    }

    if (npc.attackCooldownRemainingMs > 0) {
        npc.attackCooldownRemainingMs = std::max(0, npc.attackCooldownRemainingMs - deltaMs);
    }

    if (!player || !player->alive || npc.state == QStringLiteral("hit")) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    const qreal playerCenterX = player->position.x() + tuning.actorWidth / 2.0;
    const qreal npcCenterX = npc.position.x() + tuning.actorWidth / 2.0;
    const qreal dx = playerCenterX - npcCenterX;
    const qreal distance = std::abs(dx);
    if (distance > npc.detectionRange) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    npc.facingLeft = dx < 0;
    if (npc.state == QStringLiteral("attack")) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }

    if (distance <= npc.attackRange) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        if (npc.attackCooldownRemainingMs == 0) {
            CharacterSystem::beginAttack(npc, npc.facingLeft ? QStringLiteral("left") : QStringLiteral("right"), 4, 70, 280);
            npc.attackCooldownRemainingMs = npc.attackCooldownMs;
            events.sounds.push_back(QStringLiteral("enemy.attack"));
        }
        return;
    }

    npc.moveDirection = dx < 0 ? -1 : 1;
    npc.velocity.setX(npc.moveDirection * npc.npcMoveSpeed);
}

} // namespace skybound
