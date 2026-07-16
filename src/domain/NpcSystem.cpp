#include "domain/NpcSystem.h"

#include "domain/CharacterSystem.h"

#include <algorithm>
#include <cmath>

namespace skybound {

// ──────────────────────────────────────────────
// 蜗牛 AI 更新 — 平台往返巡逻
//
// 蜗牛在出生点附近的平台上左右来回走动，不追击玩家。
// 碰到平台边界（leftBound / rightBound）时自动转向。
// ──────────────────────────────────────────────
void NpcSystem::updateSnail(CharacterObject& npc, const CharacterObject* player, int deltaMs, const WorldTuning& tuning) {
    Q_UNUSED(player);
    Q_UNUSED(tuning);
    // 递减攻击冷却（用于接触伤害），确保每帧都在减少
    if (npc.attackCooldownRemainingMs > 0) {
        npc.attackCooldownRemainingMs = std::max(0, npc.attackCooldownRemainingMs - deltaMs);
    }

    // 缩壳或死亡 → 静止
    if (npc.state == QStringLiteral("hide") || npc.state == QStringLiteral("dead")) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }
    // 受击 → 保留击退速度
    if (npc.state == QStringLiteral("hit")) {
        npc.moveDirection = 0;
        return;
    }

    // ── 平台往返巡逻 ──
    const qreal leftBound = npc.spawnX - npc.patrolRange;
    const qreal rightBound = npc.spawnX + npc.patrolRange;

    // 到达左边界 → 向右转
    if (npc.position.x() <= leftBound) {
        npc.facingLeft = false;
        npc.moveDirection = 1;
    }
    // 到达右边界 → 向左转
    else if (npc.position.x() >= rightBound) {
        npc.facingLeft = true;
        npc.moveDirection = -1;
    }
    // 在范围内，保持当前方向
    else if (npc.moveDirection == 0) {
        // 刚出生，默认向右走
        npc.facingLeft = false;
        npc.moveDirection = 1;
    }

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

    if (!player || !player->alive) {
        npc.moveDirection = 0;
        npc.velocity.setX(0);
        return;
    }
    if (npc.state == QStringLiteral("hit")) {
        npc.moveDirection = 0;
        // hit 状态保留 velocity（击退效果）
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
