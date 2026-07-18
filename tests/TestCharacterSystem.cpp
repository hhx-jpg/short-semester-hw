#include "domain/CharacterSystem.h"

#include <QtTest>

namespace skybound {

class TestCharacterSystem : public QObject {
    Q_OBJECT

private slots:
    void animationForFamilyState_snailWalk();
    void animationForFamilyState_snailHit();
    void animationForFamilyState_player();
    void setState_skipsWhenSame();
    void setState_resetsFrameIndex();
    void updateAnimation_advancesFrame();
    void beginAttack_incrementsSerial();
};

void TestCharacterSystem::animationForFamilyState_snailWalk() {
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("idle")),
             QStringLiteral("mob.snail.walk"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("run")),
             QStringLiteral("mob.snail.walk"));
}

void TestCharacterSystem::animationForFamilyState_snailHit() {
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("hit")),
             QStringLiteral("mob.snail.hide"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("hide")),
             QStringLiteral("mob.snail.hide"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("dead")),
             QStringLiteral("mob.snail.dead"));
}

void TestCharacterSystem::animationForFamilyState_player() {
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("idle")),
             QStringLiteral("player.idle"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("run")),
             QStringLiteral("player.run"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("jump")),
             QStringLiteral("player.jump"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("attack")),
             QStringLiteral("player.attack"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("dead")),
             QStringLiteral("player.dead"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("player"), QStringLiteral("hit")),
             QStringLiteral("player.hit"));
}

void TestCharacterSystem::setState_skipsWhenSame() {
    CharacterObject character;
    character.state = QStringLiteral("idle");
    character.frameCount = 8;
    character.actionDurationMs = 0;

    CharacterSystem::setState(character, QStringLiteral("idle"), 8, 90, 0);

    QCOMPARE(character.state, QStringLiteral("idle"));
    QCOMPARE(character.frameIndex, 0);
}

void TestCharacterSystem::setState_resetsFrameIndex() {
    CharacterObject character;
    character.state = QStringLiteral("run");
    character.frameIndex = 5;

    CharacterSystem::setState(character, QStringLiteral("idle"), 1, 90);

    QCOMPARE(character.state, QStringLiteral("idle"));
    QCOMPARE(character.frameIndex, 0);
    QCOMPARE(character.animationKey, QStringLiteral("enemy.idle"));
}

void TestCharacterSystem::updateAnimation_advancesFrame() {
    CharacterObject character;
    character.animationFamily = QStringLiteral("small_bee");
    character.state = QStringLiteral("idle");
    character.frameCount = 4;
    character.frameIntervalMs = 90;
    character.frameElapsedMs = 0;
    character.frameIndex = 0;
    character.actionDurationMs = 0;
    character.animationKey = CharacterSystem::animationForFamilyState(character.animationFamily, character.state);

    CharacterSystem::updateAnimation(character, 90);

    QCOMPARE(character.frameIndex, 1);
    QCOMPARE(character.frameElapsedMs, 0);
}

void TestCharacterSystem::beginAttack_incrementsSerial() {
    CharacterObject attacker;
    attacker.id = QStringLiteral("player");
    attacker.kind = QStringLiteral("player");
    attacker.attackSerial = 0;
    attacker.facingLeft = true;

    CharacterSystem::beginAttack(attacker, QStringLiteral("left"), 4, 70, 280);

    QCOMPARE(attacker.attackSerial, 1);
    QCOMPARE(attacker.state, QStringLiteral("attack"));
    QVERIFY(attacker.attackBox.active);
    QCOMPARE(attacker.attackDirection, QStringLiteral("left"));
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestCharacterSystem)

#include "TestCharacterSystem.moc"
