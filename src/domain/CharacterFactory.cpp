#include "domain/CharacterFactory.h"

#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"

#include <algorithm>

namespace skybound {

CharacterObject CharacterFactory::createPlayer(qreal x, qreal y, const WorldTuning& tuning) {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.hp = 100;
    player.maxHp = 100;
    player.lives = 3;
    player.animationFamily = QStringLiteral("player");
    player.position = QPointF(x, y);
    player.facingLeft = true;
    CharacterSystem::setState(player, QStringLiteral("idle"), 1, 90);
    CollisionSystem::updateCollisionBoxes(player, tuning);
    return player;
}

CharacterObject CharacterFactory::createSmallBee(qreal x, qreal y, const WorldTuning& tuning) {
    CharacterObject bee;
    bee.id = QStringLiteral("small_bee_1");
    bee.kind = QStringLiteral("enemy");
    bee.animationFamily = QStringLiteral("small_bee");
    bee.hp = 30;
    bee.maxHp = 30;
    bee.lives = 1;
    bee.aiControlled = true;
    bee.attackDamage = 10;
    bee.attackCooldownMs = 900;
    bee.detectionRange = 600;
    bee.attackRange = 95;
    bee.npcMoveSpeed = 2.2;
    bee.position = QPointF(x, y);
    bee.facingLeft = true;
    CharacterSystem::setState(bee, QStringLiteral("idle"), 4, 90);
    CollisionSystem::updateCollisionBoxes(bee, tuning);
    return bee;
}

// ──────────────────────────────────────────────
// 创建蜗牛敌人
//
// 蜗牛是一种慢速地面怪物，行为特征：
//   - 主动朝玩家靠近（检测范围 400px）
//   - 身体触碰玩家时造成接触伤害（10 HP，每 500ms 冷却）
//   - 对普通攻击免疫，被攻击时缩入壳中（hide 动画）
//   - 唯一的消灭方式：玩家从上方踩踏（碰触壳顶 → 击杀 + 弹跳）
//
// 参数：
//   x, y  — 初始位置（左上角坐标）
//   tuning — 世界调参（传入以计算碰撞箱）
// ──────────────────────────────────────────────
CharacterObject CharacterFactory::createSnail(qreal x, qreal y, const WorldTuning& tuning) {
    CharacterObject snail;
    snail.id = QStringLiteral("snail_1");
    snail.kind = QStringLiteral("enemy");
    snail.animationFamily = QStringLiteral("snail");
    snail.hp = 1;                       // HP 仅用于标识存活，实际伤害逻辑不走 HP
    snail.maxHp = 1;
    snail.lives = 1;
    snail.aiControlled = true;
    snail.attackDamage = 10;            // 接触伤害值
    snail.attackCooldownMs = 500;       // 接触伤害冷却（ms），避免每帧扣血
    snail.detectionRange = 400;         // 检测到玩家的距离
    snail.attackRange = 0;              // 不主动攻击，靠接触伤害系统
    snail.npcMoveSpeed = 1.0;           // 慢速移动
    snail.charWidth = 32;               // 蜗牛渲染宽度（像素），与精灵表帧宽一致
    snail.charHeight = 32;              // 蜗牛实际渲染高度（像素）
    snail.position = QPointF(x, y);
    snail.spawnX = x;               // 记录出生 X 坐标（巡逻用）
    snail.patrolRange = 80;         // 巡逻范围半宽
    snail.facingLeft = true;
    CharacterSystem::setState(snail, QStringLiteral("idle"), 12, 120);  // 12 帧行走循环
    CollisionSystem::updateCollisionBoxes(snail, tuning);
    return snail;
}

// ──────────────────────────────────────────────
// 按类型字符串创建怪物（工厂分发入口）
//
// 遍历已知怪物类型，匹配后调用对应的 createXxx 方法。
// 未知类型返回空对象（id 为空），调用方可用以下方式检查：
//   CharacterObject mob = CharacterFactory::createByType(...);
//   if (!mob.id.isEmpty()) { characters_.insert(mob.id, mob); }
// ──────────────────────────────────────────────
CharacterObject CharacterFactory::createByType(const QString& type, qreal x, qreal y, const WorldTuning& tuning) {
    if (type == QStringLiteral("small_bee")) {
        return createSmallBee(x, y, tuning);
    }
    if (type == QStringLiteral("snail")) {
        return createSnail(x, y, tuning);
    }
    // 未知类型：返回默认构造的空对象
    return CharacterObject{};
}

} // namespace skybound
