#include "domain/CombatSystem.h"

#include <QtTest>

namespace skybound {

class TestCombatSystem : public QObject {
    Q_OBJECT

private slots:
    void resolvesOneHitPerAttackSerial();
    void damagesAndDefeatsEnemy();
    void damagesPlayerThroughDomainResult();
    void checkAttackHits_enemyHitsPlayer();
    void checkAttackHits_ignoresDeadTarget();
    void checkContactDamage_snailHurtsPlayer();
    void checkContactDamage_hidingSnailDoesNoDamage();
    void checkStompKill_stompsSnail();
    void checkStompKill_notStompingWhenPlayerRising();
};

void TestCombatSystem::resolvesOneHitPerAttackSerial() {
    CharacterObject attacker;
    attacker.id = QStringLiteral("player");
    attacker.kind = QStringLiteral("player");
    attacker.attackSerial = 1;
    attacker.attackDamage = 10;
    attacker.attackBox = CollisionBox{QRectF(0, 0, 100, 100), true};

    CharacterObject enemy;
    enemy.id = QStringLiteral("bee");
    enemy.kind = QStringLiteral("enemy");
    enemy.hp = 30;
    enemy.hurtbox = CollisionBox{QRectF(20, 20, 30, 30), true};
    enemy.animationFamily = QStringLiteral("small_bee");

    QHash<QString, CharacterObject> characters;
    characters.insert(attacker.id, attacker);
    characters.insert(enemy.id, enemy);
    QSet<QString> resolvedTokens;
    QList<TerrainPiece> emptyTerrain;

    auto first = CombatSystem::checkAttackHits(characters[attacker.id], characters, emptyTerrain, resolvedTokens);
    auto second = CombatSystem::checkAttackHits(characters[attacker.id], characters, emptyTerrain, resolvedTokens);

    QCOMPARE(first.damageCountDelta, 1);
    QCOMPARE(second.damageCountDelta, 0);
    QCOMPARE(characters[enemy.id].hp, 20);
}

void TestCombatSystem::damagesAndDefeatsEnemy() {
    CharacterObject attacker;
    attacker.id = QStringLiteral("player");
    attacker.kind = QStringLiteral("player");
    attacker.attackSerial = 7;
    attacker.attackDamage = 30;
    attacker.attackBox = CollisionBox{QRectF(0, 0, 100, 100), true};

    CharacterObject enemy;
    enemy.id = QStringLiteral("bee");
    enemy.kind = QStringLiteral("enemy");
    enemy.hp = 30;
    enemy.hurtbox = CollisionBox{QRectF(20, 20, 30, 30), true};
    enemy.animationFamily = QStringLiteral("small_bee");

    QHash<QString, CharacterObject> characters;
    characters.insert(attacker.id, attacker);
    characters.insert(enemy.id, enemy);
    QSet<QString> resolvedTokens;
    QList<TerrainPiece> emptyTerrain;

    const auto result = CombatSystem::checkAttackHits(characters[attacker.id], characters, emptyTerrain, resolvedTokens);

    QCOMPARE(result.damageCountDelta, 1);
    QVERIFY(!characters[enemy.id].alive);
    QCOMPARE(characters[enemy.id].state, QStringLiteral("dead"));
}

void TestCombatSystem::damagesPlayerThroughDomainResult() {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.hp = 10;
    player.maxHp = 10;

    const QPointF dummyAttackerPos(0, 0);
    const auto result = CombatSystem::applyDamageToPlayer(player, 10, dummyAttackerPos);

    QVERIFY(result.playerStatsChanged);
    QVERIFY(!player.alive);
    QCOMPARE(player.hp, 0);
    QCOMPARE(player.state, QStringLiteral("dead"));
    QCOMPARE(result.sounds, QStringList{QStringLiteral("player.dead")});
}

void TestCombatSystem::checkAttackHits_enemyHitsPlayer() {
    CharacterObject enemy;
    enemy.id = QStringLiteral("bee");
    enemy.kind = QStringLiteral("enemy");
    enemy.attackSerial = 1;
    enemy.attackDamage = 10;
    enemy.attackBox = CollisionBox{QRectF(0, 0, 100, 100), true};
    enemy.charWidth = 32;
    enemy.charHeight = 32;
    enemy.position = QPointF(0, 0);

    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.hp = 100;
    player.maxHp = 100;
    player.alive = true;
    player.hurtbox = CollisionBox{QRectF(20, 20, 50, 50), true};
    player.charWidth = 90;
    player.charHeight = 90;
    player.position = QPointF(30, 30);
    player.state = QStringLiteral("idle");

    QHash<QString, CharacterObject> characters;
    characters.insert(enemy.id, enemy);
    characters.insert(player.id, player);
    QSet<QString> resolvedTokens;
    QList<TerrainPiece> emptyTerrain;

    const auto result = CombatSystem::checkAttackHits(characters[enemy.id], characters, emptyTerrain, resolvedTokens);

    QVERIFY(result.playerStatsChanged);
    QCOMPARE(characters[player.id].hp, 90);
    QCOMPARE(characters[player.id].state, QStringLiteral("hit"));
}

void TestCombatSystem::checkAttackHits_ignoresDeadTarget() {
    CharacterObject attacker;
    attacker.id = QStringLiteral("player");
    attacker.kind = QStringLiteral("player");
    attacker.attackSerial = 3;
    attacker.attackDamage = 10;
    attacker.attackBox = CollisionBox{QRectF(0, 0, 100, 100), true};

    CharacterObject deadEnemy;
    deadEnemy.id = QStringLiteral("dead_bee");
    deadEnemy.kind = QStringLiteral("enemy");
    deadEnemy.hp = 0;
    deadEnemy.alive = false;
    deadEnemy.hurtbox = CollisionBox{QRectF(20, 20, 30, 30), true};

    QHash<QString, CharacterObject> characters;
    characters.insert(attacker.id, attacker);
    characters.insert(deadEnemy.id, deadEnemy);
    QSet<QString> resolvedTokens;
    QList<TerrainPiece> emptyTerrain;

    const auto result = CombatSystem::checkAttackHits(characters[attacker.id], characters, emptyTerrain, resolvedTokens);

    QCOMPARE(result.damageCountDelta, 0);
}

void TestCombatSystem::checkContactDamage_snailHurtsPlayer() {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.hp = 100;
    player.maxHp = 100;
    player.alive = true;
    player.hurtbox = CollisionBox{QRectF(100, 100, 44, 72), true};
    player.charWidth = 90;
    player.charHeight = 90;
    player.position = QPointF(77, 88);

    CharacterObject snail;
    snail.id = QStringLiteral("snail");
    snail.animationFamily = QStringLiteral("snail");
    snail.kind = QStringLiteral("enemy");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.attackDamage = 10;
    snail.attackCooldownMs = 500;
    snail.attackCooldownRemainingMs = 0;
    snail.hurtbox = CollisionBox{QRectF(110, 110, 42, 32), true};
    snail.position = QPointF(115, 110);

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkContactDamage(characters);

    QVERIFY(result.playerStatsChanged);
    QCOMPARE(characters[player.id].hp, 90);
    QVERIFY(characters[snail.id].attackCooldownRemainingMs > 0);
}

void TestCombatSystem::checkContactDamage_hidingSnailDoesNoDamage() {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.hp = 100;
    player.maxHp = 100;
    player.alive = true;
    player.hurtbox = CollisionBox{QRectF(100, 100, 44, 72), true};

    CharacterObject snail;
    snail.id = QStringLiteral("snail");
    snail.animationFamily = QStringLiteral("snail");
    snail.kind = QStringLiteral("enemy");
    snail.alive = true;
    snail.state = QStringLiteral("hide");
    snail.attackCooldownRemainingMs = 0;
    snail.hurtbox = CollisionBox{QRectF(110, 110, 42, 32), true};

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkContactDamage(characters);

    QVERIFY(!result.playerStatsChanged);
    QCOMPARE(characters[player.id].hp, 100);
}

void TestCombatSystem::checkStompKill_stompsSnail() {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.alive = true;
    player.velocity = QPointF(0, 5);
    player.position = QPointF(100, 110);
    player.charHeight = 90;
    player.hurtbox = CollisionBox{QRectF(95, 140, 44, 72), true};

    CharacterObject snail;
    snail.id = QStringLiteral("snail");
    snail.animationFamily = QStringLiteral("snail");
    snail.kind = QStringLiteral("enemy");
    snail.alive = true;
    snail.hurtbox = CollisionBox{QRectF(100, 200, 42, 32), true};

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkStompKill(characters);

    QCOMPARE(result.damageCountDelta, 1);
    QVERIFY(!characters[snail.id].alive);
    QCOMPARE(characters[snail.id].state, QStringLiteral("dead"));
    QVERIFY(characters[player.id].velocity.y() < 0);
}

void TestCombatSystem::checkStompKill_notStompingWhenPlayerRising() {
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.alive = true;
    player.velocity = QPointF(0, -5);
    player.position = QPointF(100, 130);
    player.charHeight = 90;
    player.hurtbox = CollisionBox{QRectF(95, 140, 44, 72), true};

    CharacterObject snail;
    snail.id = QStringLiteral("snail");
    snail.animationFamily = QStringLiteral("snail");
    snail.kind = QStringLiteral("enemy");
    snail.alive = true;
    snail.hurtbox = CollisionBox{QRectF(100, 200, 42, 32), true};

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkStompKill(characters);

    QCOMPARE(result.damageCountDelta, 0);
    QVERIFY(characters[snail.id].alive);
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::TestCombatSystem)

#include "TestCombatSystem.moc"
