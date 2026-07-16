#include "domain/CollisionSystem.h"
#include "domain/CombatSystem.h"

#include <QtTest>

namespace skybound {

class DomainSystemsTest : public QObject {
    Q_OBJECT

private slots:
    void resolvesDownwardTerrainCollision();
    void resolvesOneHitPerAttackSerial();
    void damagesAndDefeatsEnemy();
    void damagesPlayerThroughDomainResult();
};

void DomainSystemsTest::resolvesDownwardTerrainCollision() {
    WorldTuning tuning;
    CharacterObject character;
    character.kind = QStringLiteral("player");
    character.position = QPointF(100, 90);
    character.velocity = QPointF(0, 10);

    const QList<TerrainPiece> terrain{
        TerrainPiece{QStringLiteral("ground"), QStringLiteral("ground"), QRectF(0, 150, 500, 100), true},
    };

    CollisionSystem::resolveTerrainCollision(character, terrain, tuning);

    QCOMPARE(character.position.y(), 150.0 - tuning.playerHurtboxOffsetY - tuning.playerHurtboxHeight);
    QCOMPARE(character.velocity.y(), 0.0);
}

void DomainSystemsTest::resolvesOneHitPerAttackSerial() {
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

    QHash<QString, CharacterObject> characters;
    characters.insert(attacker.id, attacker);
    characters.insert(enemy.id, enemy);
    QSet<QString> resolvedTokens;

    auto first = CombatSystem::checkAttackHits(characters[attacker.id], characters, resolvedTokens);
    auto second = CombatSystem::checkAttackHits(characters[attacker.id], characters, resolvedTokens);

    QCOMPARE(first.damageCountDelta, 1);
    QCOMPARE(second.damageCountDelta, 0);
    QCOMPARE(characters[enemy.id].hp, 20);
}

void DomainSystemsTest::damagesAndDefeatsEnemy() {
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

    QHash<QString, CharacterObject> characters;
    characters.insert(attacker.id, attacker);
    characters.insert(enemy.id, enemy);
    QSet<QString> resolvedTokens;

    const auto result = CombatSystem::checkAttackHits(characters[attacker.id], characters, resolvedTokens);

    QCOMPARE(result.damageCountDelta, 1);
    QVERIFY(!characters[enemy.id].alive);
    QCOMPARE(characters[enemy.id].state, QStringLiteral("dead"));
}

void DomainSystemsTest::damagesPlayerThroughDomainResult() {
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

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::DomainSystemsTest)

#include "DomainSystemsTest.moc"
