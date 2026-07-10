#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class CharacterFactory {
public:
    static CharacterObject createPlayer(qreal x, qreal y, const WorldTuning& tuning);
    static CharacterObject createSmallBee(qreal x, qreal y, const WorldTuning& tuning);
};

} // namespace skybound
