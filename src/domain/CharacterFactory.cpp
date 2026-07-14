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

} // namespace skybound
