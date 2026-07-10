#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class NpcSystem {
public:
    static void updateNpc(CharacterObject& npc, const CharacterObject* player, int deltaMs, const WorldTuning& tuning, WorldEvents& events);
};

} // namespace skybound
