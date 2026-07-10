#pragma once

#include "model/GameWorldTypes.h"

#include <QHash>
#include <QSet>

namespace skybound {

class CombatSystem {
public:
    static void checkAttackHits(
        CharacterObject& attacker,
        QHash<QString, CharacterObject>& characters,
        QSet<QString>& resolvedAttackTokens,
        int& damageCount,
        WorldEvents& events);

    static void checkPlayerAttackHits(
        QHash<QString, CharacterObject>& characters,
        QSet<QString>& resolvedAttackTokens,
        int& damageCount,
        WorldEvents& events);
};

} // namespace skybound
