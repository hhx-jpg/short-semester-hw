#include "domain/SceneBuilder.h"

#include <QtTest>

namespace skybound {

class TestSceneBuilder : public QObject {
    Q_OBJECT

private slots:
    void buildOriginalScene_hasTerrainAndMobs();
    void buildCustomMapScene_hasThreeMobs();
};

void TestSceneBuilder::buildOriginalScene_hasTerrainAndMobs() {
    const auto result = SceneBuilder::build(SceneId::OriginalFactory, 1200, 760, 0, 0, 1200, 760);

    QCOMPARE(result.mapLayers.size(), 1);
    QCOMPARE(result.mapLayers[0].imageKey, QStringLiteral("scene.factory.background"));

    QCOMPARE(result.terrain.size(), 3);
    QCOMPARE(result.terrain[0].id, QStringLiteral("original_ground"));

    QCOMPARE(result.mobSpawns.size(), 2);
    QCOMPARE(result.mobSpawns[0].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[1].mobType, QStringLiteral("snail"));

    QCOMPARE(result.playableLeft, 0);
    QCOMPARE(result.playableRight, 1200);
}

void TestSceneBuilder::buildCustomMapScene_hasThreeMobs() {
    const auto result = SceneBuilder::build(SceneId::CustomMap, 1200, 760, 0, 0, 1200, 760);

    QCOMPARE(result.mobSpawns.size(), 3);
    QCOMPARE(result.mobSpawns[0].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[1].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[2].mobType, QStringLiteral("snail"));

    QVERIFY(result.terrain.size() >= 4);
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestSceneBuilder)

#include "TestSceneBuilder.moc"
