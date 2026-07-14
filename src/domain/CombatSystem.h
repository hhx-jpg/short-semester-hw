#pragma once

#include "model/GameWorldTypes.h"

#include <QHash>
#include <QSet>

namespace skybound {

class CombatSystem {
public:
    static CombatResult checkAttackHits(
        CharacterObject& attacker,
        QHash<QString, CharacterObject>& characters,
        QSet<QString>& resolvedAttackTokens);

    static CombatResult applyDamageToPlayer(CharacterObject& playerCharacter, int damage);
};

} // namespace skybound
