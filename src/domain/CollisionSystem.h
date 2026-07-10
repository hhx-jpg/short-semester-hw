#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class CollisionSystem {
public:
    static void updateCollisionBoxes(CharacterObject& character, const WorldTuning& tuning);
    static void resolveTerrainCollision(CharacterObject& character, const QList<TerrainPiece>& terrain, const WorldTuning& tuning);
    static bool canUseBox(const CollisionBox& box);
};

} // namespace skybound
