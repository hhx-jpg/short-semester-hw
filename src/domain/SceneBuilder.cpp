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

SceneBuildResult buildCustomMapScene(qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    SceneBuildResult result;
    result.mapLayers.push_back(MapLayer{QStringLiteral("custom_background"), QStringLiteral("scene.custom.background"), QRectF(mapX, mapY, mapWidth, mapHeight), 1.0});
    result.playableLeft = 0;
    result.playableRight = viewportWidth;

    // 地面高度与原始工厂保持一致（mapHeight 的 86.5%）
    const qreal groundY = mapY + mapHeight * 0.865;

    // 地面
    result.terrain.push_back(TerrainPiece{QStringLiteral("custom_ground"), QStringLiteral("ground"), QRectF(result.playableLeft, groundY, result.playableRight - result.playableLeft, viewportHeight - groundY), true});

    // 中间三分之一区域的主平台（距地面 +117px，安全可跳）
    result.terrain.push_back(TerrainPiece{QStringLiteral("custom_mid_platform"), QStringLiteral("platform"), QRectF(350, groundY - 117, 500, 24), true});

    // 阶梯1：从中间平台右侧向右上方延伸（距中间平台 +60px）
    result.terrain.push_back(TerrainPiece{QStringLiteral("custom_stair_1"), QStringLiteral("platform"), QRectF(850, groundY - 177, 130, 24), true});

    // 阶梯2：从阶梯1向左上方延伸（距阶梯1 +55px）
    result.terrain.push_back(TerrainPiece{QStringLiteral("custom_stair_2"), QStringLiteral("platform"), QRectF(720, groundY - 232, 130, 24), true});

    return result;
}

SceneBuildResult buildNewForestScene(qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    SceneBuildResult result;
    result.mapLayers.push_back(MapLayer{QStringLiteral("new_forest_background"), QStringLiteral("scene.new_forest.background"), QRectF(mapX, mapY, mapWidth, mapHeight), 1.0});
    result.playableLeft = 0;
    result.playableRight = viewportWidth;

    // 左地面 (0, 659, 171, 101)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_left_ground"), QStringLiteral("ground"), QRectF(0, 659, 171, 101), true});

    // 左平台 (89, 571, 177, 42)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_left_platform"), QStringLiteral("platform"), QRectF(89, 571, 177, 42), true});

    // 中平台 (293, 518, 352, 36)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_mid_platform"), QStringLiteral("platform"), QRectF(293, 518, 352, 36), true});

    // 右中台 (734, 516, 259, 41)  — 左下角(734,557)到右上角(993,516)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_right_mid_platform"), QStringLiteral("platform"), QRectF(734, 516, 259, 41), true});

    // 右上台 (1050, 560, 150, 38)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_right_upper_platform"), QStringLiteral("platform"), QRectF(1050, 560, 150, 38), true});

    // 右地面 (1034, 654, 166, 106)
    result.terrain.push_back(TerrainPiece{QStringLiteral("nf_right_ground"), QStringLiteral("ground"), QRectF(1034, 654, 166, 106), true});

    return result;
}
} // namespace

SceneBuildResult SceneBuilder::build(SceneId scene, qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    if (scene == SceneId::OriginalFactory) {
        return buildOriginalScene(viewportWidth, viewportHeight, mapX, mapY, mapWidth, mapHeight);
    }
    if (scene == SceneId::CustomMap) {
        return buildCustomMapScene(viewportWidth, viewportHeight, mapX, mapY, mapWidth, mapHeight);
    }
    if (scene == SceneId::NewForestMap) {
        return buildNewForestScene(viewportWidth, viewportHeight, mapX, mapY, mapWidth, mapHeight);
    }
    return buildBackground2Scene(viewportWidth, mapX, mapY, mapWidth, mapHeight);
}

} // namespace skybound
