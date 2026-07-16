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

    // ── 原始工厂地图：生成 1 只蜜蜂 + 1 只蜗牛（测试用） ──
    // 蜜蜂出生在地面靠右位置，蜗牛在蜜蜂左侧 200px
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), viewportWidth * 0.72, 0});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"),    viewportWidth * 0.72 - 200, 0});
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

    // ── 背景2（工厂地图2）：在顶部大平台靠右位置生成 1 只蜜蜂 ──
    // 平台 bg2_top_platform 顶面 y=492，角色高 90px，
    // 因此 spawnY = 492 - 90 = 402，脚底刚好站在平台上
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), 956, 402});
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

    // ── 自定义地图：3 只怪物分布在三个不同高度 ──
    //   蜜蜂 (148) → 站在地面，spawnY = groundY - 90
    //   蜜蜂 (505) → 站在中间平台，spawnY = (groundY - 117) - 90
    //   蜗牛 (749) → 站在阶梯2，   spawnY = (groundY - 232) - 32
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), 148, groundY - 90});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), 505, groundY - 117 - 90});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"),    749, groundY - 232 - 32});
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

    // ── 新森林地图：3 只蜗牛分布在三个平台上 ──
    //   (172, 539) → nf_left_platform  顶面 571，spawnY = 571 - 32 = 539
    //   (466, 486) → nf_mid_platform   顶面 518，spawnY = 518 - 32 = 486
    //   (862, 484) → nf_right_mid_platform 顶面 516，spawnY = 516 - 32 = 484
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"), 172, 539});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"), 466, 486});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"), 862, 484});
    return result;
}

SceneBuildResult buildForest3Scene(qreal viewportWidth, qreal viewportHeight, qreal mapX, qreal mapY, qreal mapWidth, qreal mapHeight) {
    SceneBuildResult result;
    result.mapLayers.push_back(MapLayer{QStringLiteral("forest3_background"), QStringLiteral("scene.forest3.background"), QRectF(mapX, mapY, mapWidth, mapHeight), 1.0});
    result.playableLeft = 0;
    result.playableRight = viewportWidth;
    const qreal groundY = mapY + mapHeight * 0.865;
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_ground"), QStringLiteral("ground"), QRectF(result.playableLeft, groundY, result.playableRight - result.playableLeft, viewportHeight - groundY), true});

    // 1: (1, 589, 142, 37)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat1"), QStringLiteral("platform"), QRectF(1, 589, 142, 37), true});
    // 2: (145, 510, 957, 47)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat2"), QStringLiteral("platform"), QRectF(145, 510, 957, 47), true});
    // 3: (1131, 454, 69, 37)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat3"), QStringLiteral("platform"), QRectF(1131, 454, 69, 37), true});
    // 4: (583, 399, 520, 42)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat4"), QStringLiteral("platform"), QRectF(583, 399, 520, 42), true});
    // 5: (460, 363, 91, 37)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat5"), QStringLiteral("platform"), QRectF(460, 363, 91, 37), true});
    // 6: (173, 339, 259, 40)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat6"), QStringLiteral("platform"), QRectF(173, 339, 259, 40), true});
    // 7: (1, 297, 157, 43)
    result.terrain.push_back(TerrainPiece{QStringLiteral("f3_plat7"), QStringLiteral("platform"), QRectF(1, 297, 157, 43), true});

    // ── 森林3地图 ──
    // 原有 1 只蜜蜂（默认 auto 位置）
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), viewportWidth * 0.72, 0});
    // 新增 2 蜗牛 + 1 蜜蜂
    // (322)🐌 → f3_plat2 顶面 510, spawnY = 510 - 32 = 478
    // (968)🐌 → f3_plat4 顶面 399, spawnY = 399 - 32 = 367
    // (546)🐝 → f3_plat2 顶面 510, spawnY = 510 - 90 = 420
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"),     322, 478});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("snail"),     968, 367});
    result.mobSpawns.push_back(MobSpawn{QStringLiteral("small_bee"), 546, 420});
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
    if (scene == SceneId::ForestMap3) {
        return buildForest3Scene(viewportWidth, viewportHeight, mapX, mapY, mapWidth, mapHeight);
    }
    return buildBackground2Scene(viewportWidth, mapX, mapY, mapWidth, mapHeight);
}

} // namespace skybound
