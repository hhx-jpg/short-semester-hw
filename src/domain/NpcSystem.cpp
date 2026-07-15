#include "domain/NpcSystem.h"

#include "domain/CharacterSystem.h"

#include <algorithm>
#include <cmath>

namespace skybound {

void NpcSystem::updateNpc(CharacterObject& npc, const CharacterObject* player, int deltaMs, const WorldTuning& tuning, WorldEvents& events) {
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
