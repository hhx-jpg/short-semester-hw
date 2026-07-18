#include "domain/NpcSystem.h"
#include "model/GameWorldTypes.h"

#include <QtTest>

namespace skybound {

class TestNpcSystem : public QObject {
    Q_OBJECT

private slots:
    void updateSnail_startsMovingRight();
    void updateSnail_reversesAtRightBound();
    void updateSnail_stopsWhenDead();
    void updateSnail_stopsWhenHiding();
    void updateSnail_cooldownDecreases();
    void updateNpc_beeChasesPlayer();
};

void TestNpcSystem::updateSnail_startsMovingRight() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.spawnX = 400;
    snail.patrolRange = 80;
    snail.npcMoveSpeed = 1.0;
    snail.moveDirection = 0;
    snail.position = QPointF(400, 100);
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    QCOMPARE(snail.moveDirection, 1);
    QVERIFY(snail.velocity.x() > 0);
    QCOMPARE(snail.velocity.x(), 1.0);
}

void TestNpcSystem::updateSnail_reversesAtRightBound() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.spawnX = 400;
    snail.patrolRange = 80;
    snail.npcMoveSpeed = 1.0;
    snail.moveDirection = 1;
    snail.position = QPointF(480, 100);
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    QCOMPARE(snail.moveDirection, -1);
    QCOMPARE(snail.facingLeft, true);
}

void TestNpcSystem::updateSnail_stopsWhenDead() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("dead");
    snail.moveDirection = 1;
    snail.velocity = QPointF(1, 0);
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    QCOMPARE(snail.moveDirection, 0);
    QCOMPARE(snail.velocity.x(), 0.0);
}

void TestNpcSystem::updateSnail_stopsWhenHiding() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("hide");
    snail.moveDirection = 1;
    snail.velocity = QPointF(1, 0);
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    QCOMPARE(snail.moveDirection, 0);
    QCOMPARE(snail.velocity.x(), 0.0);
}

void TestNpcSystem::updateSnail_cooldownDecreases() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.attackCooldownRemainingMs = 500;
    snail.position = QPointF(400, 100);
    snail.spawnX = 400;
    snail.patrolRange = 80;
    snail.npcMoveSpeed = 1.0;
    snail.moveDirection = 0;

    NpcSystem::updateSnail(snail, nullptr, 100, WorldTuning{});

    QCOMPARE(snail.attackCooldownRemainingMs, 400);
}

void TestNpcSystem::updateNpc_beeChasesPlayer() {
    CharacterObject bee;
    bee.id = QStringLiteral("bee");
    bee.animationFamily = QStringLiteral("small_bee");
    bee.kind = QStringLiteral("enemy");
    bee.alive = true;
    bee.state = QStringLiteral("idle");
    bee.detectionRange = 600;
    bee.attackRange = 95;
    bee.npcMoveSpeed = 2.2;
    bee.attackCooldownRemainingMs = 0;
    bee.position = QPointF(500, 200);
    bee.velocity = QPointF(0, 0);
    bee.moveDirection = 0;

    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.alive = true;
    player.position = QPointF(300, 200);

    WorldEvents events;
    NpcSystem::updateNpc(bee, &player, 16, WorldTuning{}, events);

    QCOMPARE(bee.moveDirection, -1);
    QVERIFY(bee.velocity.x() < 0);
    QVERIFY(bee.facingLeft);
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestNpcSystem)

#include "TestNpcSystem.moc"
