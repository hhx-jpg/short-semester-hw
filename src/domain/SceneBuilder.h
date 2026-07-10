#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class SceneBuilder {
public:
    static SceneBuildResult build(
        SceneId scene,
        qreal viewportWidth,
        qreal viewportHeight,
        qreal mapX,
        qreal mapY,
        qreal mapWidth,
        qreal mapHeight);
};

} // namespace skybound
