#pragma once

#include "model/GameWorldTypes.h"

#include <QHash>
#include <QSet>

namespace skybound {

class WorldProcessor {
public:
    static SceneSwitchRequest advanceActors(
        QHash<QString, CharacterObject>& characters,
        int deltaMs,
        SceneId currentScene,
        qreal playableLeft,
        qreal playableRight,
        const QList<TerrainPiece>& terrain,
        const WorldTuning& tuning,
        bool chargePressed,
        WorldEvents& events);

    static CombatResult resolveCombat(
        QHash<QString, CharacterObject>& characters,
        const QList<TerrainPiece>& terrain,
        QSet<QString>& resolvedAttackTokens);
};

} // namespace skybound
