#include "domain/CharacterFactory.h"

#include <QtTest>

namespace skybound {

class TestCharacterFactory : public QObject {
    Q_OBJECT

private slots:
    void createPlayer_hasCorrectDefaults();
    void createSmallBee_hasCorrectDefaults();
    void createSnail_hasCorrectDefaults();
    void createByType_unknownReturnsEmpty();
};

void TestCharacterFactory::createPlayer_hasCorrectDefaults() {
    WorldTuning tuning;
    const auto player = CharacterFactory::createPlayer(100, 200, tuning);

    QCOMPARE(player.id, QStringLiteral("player"));
    QCOMPARE(player.kind, QStringLiteral("player"));
    QCOMPARE(player.hp, 100);
    QCOMPARE(player.maxHp, 100);
    QCOMPARE(player.lives, 3);
    QCOMPARE(player.animationFamily, QStringLiteral("player"));
    QCOMPARE(player.position, QPointF(100, 200));
    QVERIFY(player.hurtbox.active);
}

void TestCharacterFactory::createSmallBee_hasCorrectDefaults() {
    WorldTuning tuning;
    const auto bee = CharacterFactory::createSmallBee(300, 400, tuning);

    QVERIFY(bee.id.startsWith(QStringLiteral("small_bee")));
    QCOMPARE(bee.kind, QStringLiteral("enemy"));
    QCOMPARE(bee.animationFamily, QStringLiteral("small_bee"));
    QCOMPARE(bee.hp, 30);
    QCOMPARE(bee.maxHp, 30);
    QVERIFY(bee.aiControlled);
    QCOMPARE(bee.attackDamage, 10);
    QCOMPARE(bee.detectionRange, 600);
    QCOMPARE(bee.npcMoveSpeed, 2.2);
    QCOMPARE(bee.position, QPointF(300, 400));
}

void TestCharacterFactory::createSnail_hasCorrectDefaults() {
    WorldTuning tuning;
    const auto snail = CharacterFactory::createSnail(200, 300, tuning);

    QVERIFY(snail.id.startsWith(QStringLiteral("snail")));
    QCOMPARE(snail.kind, QStringLiteral("enemy"));
    QCOMPARE(snail.animationFamily, QStringLiteral("snail"));
    QCOMPARE(snail.hp, 1);
    QCOMPARE(snail.charWidth, 46);
    QCOMPARE(snail.charHeight, 32);
    QVERIFY(snail.aiControlled);
    QCOMPARE(snail.attackDamage, 10);
    QCOMPARE(snail.attackCooldownMs, 500);
    QCOMPARE(snail.detectionRange, 400);
    QCOMPARE(snail.npcMoveSpeed, 1.0);
    QCOMPARE(snail.spawnX, 200);
    QCOMPARE(snail.patrolRange, 80);
    QCOMPARE(snail.position, QPointF(200, 300));
    QCOMPARE(snail.state, QStringLiteral("idle"));
    QCOMPARE(snail.frameCount, 12);
}

void TestCharacterFactory::createByType_unknownReturnsEmpty() {
    WorldTuning tuning;
    const auto unknown = CharacterFactory::createByType(QStringLiteral("dragon"), 0, 0, tuning);
    QVERIFY(unknown.id.isEmpty());
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestCharacterFactory)

#include "TestCharacterFactory.moc"
