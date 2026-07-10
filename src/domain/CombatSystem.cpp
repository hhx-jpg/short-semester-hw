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

void applyPlayerDamage(CharacterObject& playerCharacter, int damage, WorldEvents& events) {
    playerCharacter.hp = std::max(0, playerCharacter.hp - std::max(0, damage));
    if (playerCharacter.hp == 0) {
        playerCharacter.alive = false;
        CharacterSystem::setState(playerCharacter, QStringLiteral("dead"), 1, 90);
        events.sounds.push_back(QStringLiteral("player.dead"));
        return;
    }

    if (playerCharacter.state == QStringLiteral("hit")) {
        playerCharacter.state = QString();
        playerCharacter.actionDurationMs = 0;
    }
    CharacterSystem::setState(playerCharacter, QStringLiteral("hit"), 4, 90, 360);
    events.sounds.push_back(QStringLiteral("player.hurt"));
}
} // namespace

void CombatSystem::checkAttackHits(
    CharacterObject& attacker,
    QHash<QString, CharacterObject>& characters,
    QSet<QString>& resolvedAttackTokens,
    int& damageCount,
    WorldEvents& events) {
    if (!attacker.alive || !CollisionSystem::canUseBox(attacker.attackBox)) {
        return;
    }

    const QString token = QStringLiteral("%1-attack-%2").arg(attacker.id).arg(attacker.attackSerial);
    if (resolvedAttackTokens.contains(token)) {
        return;
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
            ++damageCount;
            if (target.hp == 0) {
                target.alive = false;
                CharacterSystem::setState(target, QStringLiteral("dead"), 1, 90);
            } else {
                CharacterSystem::setState(target, QStringLiteral("hit"), target.animationFamily == QStringLiteral("small_bee") ? 4 : 1, 90, 360);
            }
            events.damageCountChanged = true;
            events.sounds.push_back(QStringLiteral("enemy.hurt.1"));
            break;
        }
        return;
    }

    auto* p = player(characters);
    if (!p || !p->alive || !CollisionSystem::canUseBox(p->hurtbox) || !attacker.attackBox.rect.intersects(p->hurtbox.rect)) {
        return;
    }

    resolvedAttackTokens.insert(token);
    applyPlayerDamage(*p, attacker.attackDamage, events);
}

void CombatSystem::checkPlayerAttackHits(QHash<QString, CharacterObject>& characters, QSet<QString>& resolvedAttackTokens, int& damageCount, WorldEvents& events) {
    auto* p = player(characters);
    if (p) {
        checkAttackHits(*p, characters, resolvedAttackTokens, damageCount, events);
    }
}

} // namespace skybound
