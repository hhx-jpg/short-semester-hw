#include "domain/SceneBuilder.h"

#include <algorithm>

namespace skybound {
namespace {
SceneBuildResult buildOriginalScene(qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    SceneBuildResult result;
    result.mapLayers.push_back(MapLayer{QStringLiteral("factory_background"), QStringLiteral("scene.factory.background"), QRectF(mapX, mapY, mapWidth, mapHeight), 1.0});
    result.playableLeft = 0;
    result.playableRight = viewportWidth;
    const qreal groundY = mapY + mapHeight * 0.865;
    result.terrain.push_back(TerrainPiece{QStringLiteral("original_ground"), QStringLiteral("ground"), QRectF(result.playableLeft, groundY, result.playableRight - result.playableLeft, viewportHeight - groundY), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("original_platform_left"), QStringLiteral("platform"), QRectF(viewportWidth * 0.18, groundY - 145, 230, 24), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("original_platform_right"), QStringLiteral("platform"), QRectF(viewportWidth * 0.62, groundY - 105, 260, 24), true});
    return result;
}

SceneBuildResult buildBackground2Scene(qreal viewportWidth, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    SceneBuildResult result;
    result.mapLayers.push_back(MapLayer{QStringLiteral("factory_background2"), QStringLiteral("scene.factory.background2"), QRectF(mapX, mapY, mapWidth, mapHeight), 1.0});
    result.playableLeft = std::max<qreal>(0, mapX);
    result.playableRight = std::min<qreal>(viewportWidth, mapX + mapWidth);
    result.terrain.push_back(TerrainPiece{QStringLiteral("bg2_ground"), QStringLiteral("platform"), QRectF(0, 673, 698, 91), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("bg2_step1"), QStringLiteral("platform"), QRectF(423, 628, 89, 46), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("bg2_step2"), QStringLiteral("platform"), QRectF(469, 583, 88, 42), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("bg2_step3"), QStringLiteral("platform"), QRectF(560, 539, 89, 41), true});
    result.terrain.push_back(TerrainPiece{QStringLiteral("bg2_top_platform"), QStringLiteral("platform"), QRectF(605, 492, 594, 41), true});
    return result;
}
} // namespace

SceneBuildResult SceneBuilder::build(SceneId scene, qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    if (scene == SceneId::OriginalFactory) {
        return buildOriginalScene(viewportWidth, viewportHeight, mapX, mapY, mapWidth, mapHeight);
    }
    return buildBackground2Scene(viewportWidth, mapX, mapY, mapWidth, mapHeight);
}

} // namespace skybound
