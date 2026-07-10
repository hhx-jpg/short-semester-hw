#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class CharacterSystem {
public:
    static QString animationForFamilyState(const QString& family, const QString& state);
    static void setState(CharacterObject& character, const QString& state, int frameCount, int frameIntervalMs, int durationMs = 0);
    static void updateAnimation(CharacterObject& character, int deltaMs);
    static void beginAttack(CharacterObject& attacker, const QString& direction, int frameCount, int frameIntervalMs, int durationMs);
};

} // namespace skybound
