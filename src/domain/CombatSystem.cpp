#include "domain/CombatSystem.h"

#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"

#include <algorithm>

namespace skybound {
namespace {
CharacterObject* player(QHash<QString, CharacterObject>& characters) {
    auto it = characters.find(QStringLiteral("player"));
    return it == characters.end() ? nullptr : &it.value();
}
} // namespace

CombatResult CombatSystem::applyDamageToPlayer(CharacterObject& playerCharacter, int damage) {
    CombatResult result;
    if (!playerCharacter.alive) {
        return result;
    }

    playerCharacter.hp = std::max(0, playerCharacter.hp - std::max(0, damage));
    result.playerStatsChanged = true;
    if (playerCharacter.hp == 0) {
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

    if (playerCharacter.state == QStringLiteral("hit")) {
        playerCharacter.state = QString();
        playerCharacter.actionDurationMs = 0;
    }
    CharacterSystem::setState(playerCharacter, QStringLiteral("hit"), 4, 90, 360);
    result.sounds.push_back(QStringLiteral("player.hurt"));
    return result;
}

CombatResult CombatSystem::checkAttackHits(
    CharacterObject& attacker,
    QHash<QString, CharacterObject>& characters,
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
        for (auto it = characters.begin(); it != characters.end(); ++it) {
            auto& target = it.value();
            if (target.id == attacker.id || target.kind != QStringLiteral("enemy") || !target.alive || !CollisionSystem::canUseBox(target.hurtbox)) {
                continue;
            }
            if (!attacker.attackBox.rect.intersects(target.hurtbox.rect)) {
                continue;
            }

            resolvedAttackTokens.insert(token);
            target.hp = std::max(0, target.hp - attacker.attackDamage);
            result.damageCountDelta = 1;
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

    auto* p = player(characters);
    if (!p || !p->alive || !CollisionSystem::canUseBox(p->hurtbox) || !attacker.attackBox.rect.intersects(p->hurtbox.rect)) {
        return result;
    }

    resolvedAttackTokens.insert(token);
    return applyDamageToPlayer(*p, attacker.attackDamage);
}

} // namespace skybound
