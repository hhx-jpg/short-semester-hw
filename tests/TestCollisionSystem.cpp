#include "domain/CollisionSystem.h"

#include <QtTest>

namespace skybound {

class TestCollisionSystem : public QObject {
    Q_OBJECT

private slots:
    void resolvesDownwardTerrainCollision();
    void updateCollisionBoxes_snailHasCustomHurtbox();
    void updateCollisionBoxes_attackActivatesBox();
    void canUseBox_inactiveReturnsFalse();
    void canUseBox_emptyRectReturnsFalse();
};

void TestCollisionSystem::resolvesDownwardTerrainCollision() {
    WorldTuning tuning;
    CharacterObject character;
    character.kind = QStringLiteral("player");
    character.position = QPointF(100, 72); // y=72 → hurtbox.bottom()=156, previousBottom=146, 穿越地形顶面150
    character.velocity = QPointF(0, 10);

    const QList<TerrainPiece> terrain{
        TerrainPiece{QStringLiteral("ground"), QStringLiteral("ground"), QRectF(0, 150, 500, 100), true},
    };

    CollisionSystem::resolveTerrainCollision(character, terrain, tuning);

    QCOMPARE(character.position.y(), 150.0 - tuning.playerHurtboxOffsetY - tuning.playerHurtboxHeight);
    QCOMPARE(character.velocity.y(), 0.0);
}

void TestCollisionSystem::updateCollisionBoxes_snailHasCustomHurtbox() {
    WorldTuning tuning;
    CharacterObject snail;
    snail.kind = QStringLiteral("enemy");
    snail.animationFamily = QStringLiteral("snail");
    snail.charWidth = 46;
    snail.charHeight = 32;
    snail.position = QPointF(100, 200);

    CollisionSystem::updateCollisionBoxes(snail, tuning);

    // 蜗牛 hurtbox 固定为 42×32，偏移 -5, 0
    QCOMPARE(snail.hurtbox.rect.x(), 100 - 5);
    QCOMPARE(snail.hurtbox.rect.y(), 200);
    QCOMPARE(snail.hurtbox.rect.width(), 42);
    QCOMPARE(snail.hurtbox.rect.height(), 32);
    QVERIFY(snail.hurtbox.active);
}

void TestCollisionSystem::updateCollisionBoxes_attackActivatesBox() {
    WorldTuning tuning;
    CharacterObject player;
    player.kind = QStringLiteral("player");
    player.state = QStringLiteral("attack");
    player.attackDirection = QStringLiteral("left");
    player.position = QPointF(200, 300);

    CollisionSystem::updateCollisionBoxes(player, tuning);

    QVERIFY(player.attackBox.active);
    QCOMPARE(player.attackBox.rect.width(), tuning.attackBoxWidth);
    QCOMPARE(player.attackBox.rect.height(), tuning.attackBoxHeight);
}

void TestCollisionSystem::canUseBox_inactiveReturnsFalse() {
    CollisionBox box{QRectF(0, 0, 50, 50), false};
    QVERIFY(!CollisionSystem::canUseBox(box));
}

void TestCollisionSystem::canUseBox_emptyRectReturnsFalse() {
    CollisionBox box{QRectF(), true};
    QVERIFY(!CollisionSystem::canUseBox(box));
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestCollisionSystem)

#include "TestCollisionSystem.moc"
