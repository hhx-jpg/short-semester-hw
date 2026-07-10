#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class PhysicsSystem {
public:
    static void updateCharacterPhysics(
        CharacterObject& character,
        int deltaMs,
        SceneId currentScene,
        qreal playableLeft,
        qreal playableRight,
        const QList<TerrainPiece>& terrain,
        const WorldTuning& tuning,
        bool chargePressed,
        SceneSwitchRequest& sceneSwitch);
};

} // namespace skybound
