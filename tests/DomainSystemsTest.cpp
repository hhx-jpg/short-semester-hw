#include "domain/CollisionSystem.h"
#include "domain/CombatSystem.h"
#include "domain/CharacterSystem.h"
#include "domain/NpcSystem.h"
#include "domain/CharacterFactory.h"
#include "domain/SceneBuilder.h"

#include <QtTest>

namespace skybound {

// ══════════════════════════════════════════════════════════════
// 单元测试：领域层核心系统
// ══════════════════════════════════════════════════════════════

class DomainSystemsTest : public QObject {
    Q_OBJECT

private slots:
    // ── CollisionSystem ──
    void resolvesDownwardTerrainCollision();
    void updateCollisionBoxes_snailHasCustomHurtbox();
    void updateCollisionBoxes_attackActivatesBox();
    void canUseBox_inactiveReturnsFalse();
    void canUseBox_emptyRectReturnsFalse();

    // ── CombatSystem ──
    void resolvesOneHitPerAttackSerial();
    void damagesAndDefeatsEnemy();
    void damagesPlayerThroughDomainResult();
    void checkAttackHits_enemyHitsPlayer();
    void checkAttackHits_ignoresDeadTarget();
    void checkContactDamage_snailHurtsPlayer();
    void checkContactDamage_hidingSnailDoesNoDamage();
    void checkStompKill_stompsSnail();
    void checkStompKill_notStompingWhenPlayerRising();

    // ── CharacterSystem ──
    void animationForFamilyState_snailWalk();
    void animationForFamilyState_snailHit();
    void animationForFamilyState_player();
    void setState_skipsWhenSame();
    void setState_resetsFrameIndex();
    void updateAnimation_advancesFrame();
    void beginAttack_incrementsSerial();

    // ── NpcSystem ──
    void updateSnail_startsMovingRight();
    void updateSnail_reversesAtRightBound();
    void updateSnail_stopsWhenDead();
    void updateSnail_stopsWhenHiding();
    void updateSnail_cooldownDecreases();
    void updateNpc_beeChasesPlayer();

    // ── CharacterFactory ──
    void createPlayer_hasCorrectDefaults();
    void createSmallBee_hasCorrectDefaults();
    void createSnail_hasCorrectDefaults();
    void createByType_unknownReturnsEmpty();

    // ── SceneBuilder ──
    void buildOriginalScene_hasTerrainAndMobs();
    void buildCustomMapScene_hasThreeMobs();
};

// ══════════════════════════════════════════════════════════════
// CollisionSystem 测试
// ══════════════════════════════════════════════════════════════

void DomainSystemsTest::resolvesDownwardTerrainCollision() {
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

void DomainSystemsTest::updateCollisionBoxes_snailHasCustomHurtbox() {
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

void DomainSystemsTest::updateCollisionBoxes_attackActivatesBox() {
    WorldTuning tuning;
    CharacterObject player;
    player.kind = QStringLiteral("player");
    player.state = QStringLiteral("attack");
    player.attackDirection = QStringLiteral("left");
    // attackSerial > 0 表示这是个有效的攻击状态
    player.position = QPointF(200, 300);

    CollisionSystem::updateCollisionBoxes(player, tuning);

    QVERIFY(player.attackBox.active);
    QCOMPARE(player.attackBox.rect.width(), tuning.attackBoxWidth);
    QCOMPARE(player.attackBox.rect.height(), tuning.attackBoxHeight);
}

void DomainSystemsTest::canUseBox_inactiveReturnsFalse() {
    CollisionBox box{QRectF(0, 0, 50, 50), false};
    QVERIFY(!CollisionSystem::canUseBox(box));
}

void DomainSystemsTest::canUseBox_emptyRectReturnsFalse() {
    CollisionBox box{QRectF(), true};
    QVERIFY(!CollisionSystem::canUseBox(box));
}

// ══════════════════════════════════════════════════════════════
// CombatSystem 测试
// ══════════════════════════════════════════════════════════════

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

void DomainSystemsTest::checkAttackHits_enemyHitsPlayer() {
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

void DomainSystemsTest::checkAttackHits_ignoresDeadTarget() {
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

    // 无命中（目标已死亡）
    QCOMPARE(result.damageCountDelta, 0);
}

void DomainSystemsTest::checkContactDamage_snailHurtsPlayer() {
    // 蜗牛与玩家 hurtbox 重叠 → 接触伤害
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

void DomainSystemsTest::checkContactDamage_hidingSnailDoesNoDamage() {
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
    snail.state = QStringLiteral("hide");  // 缩壳 → 不造成伤害
    snail.attackCooldownRemainingMs = 0;
    snail.hurtbox = CollisionBox{QRectF(110, 110, 42, 32), true};

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkContactDamage(characters);

    QVERIFY(!result.playerStatsChanged);  // 无伤害
    QCOMPARE(characters[player.id].hp, 100);
}

void DomainSystemsTest::checkStompKill_stompsSnail() {
    // 玩家从上方落下，脚底碰到蜗牛头顶 → 踩踏击杀
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.alive = true;
    player.velocity = QPointF(0, 5);       // 下落
    player.position = QPointF(100, 110);    // 玩家脚底 = 200，前一帧脚底 = 195 (< 蜗牛头顶 200)
    player.charHeight = 90;
    player.hurtbox = CollisionBox{QRectF(95, 140, 44, 72), true};

    CharacterObject snail;
    snail.id = QStringLiteral("snail");
    snail.animationFamily = QStringLiteral("snail");
    snail.kind = QStringLiteral("enemy");
    snail.alive = true;
    snail.hurtbox = CollisionBox{QRectF(100, 200, 42, 32), true};  // 头顶 y=200

    QHash<QString, CharacterObject> characters;
    characters.insert(player.id, player);
    characters.insert(snail.id, snail);

    const auto result = CombatSystem::checkStompKill(characters);

    QCOMPARE(result.damageCountDelta, 1);
    QVERIFY(!characters[snail.id].alive);
    QCOMPARE(characters[snail.id].state, QStringLiteral("dead"));
    // 玩家获得向上弹跳
    QVERIFY(characters[player.id].velocity.y() < 0);
}

void DomainSystemsTest::checkStompKill_notStompingWhenPlayerRising() {
    // 玩家正在上升（velocity.y <= 0）→ 不应触发踩踏
    CharacterObject player;
    player.id = QStringLiteral("player");
    player.kind = QStringLiteral("player");
    player.alive = true;
    player.velocity = QPointF(0, -5);      // 上升
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

    QCOMPARE(result.damageCountDelta, 0);  // 无击杀
    QVERIFY(characters[snail.id].alive);   // 蜗牛存活
}

// ══════════════════════════════════════════════════════════════
// CharacterSystem 测试
// ══════════════════════════════════════════════════════════════

void DomainSystemsTest::animationForFamilyState_snailWalk() {
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("idle")),
             QStringLiteral("mob.snail.walk"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("run")),
             QStringLiteral("mob.snail.walk"));
}

void DomainSystemsTest::animationForFamilyState_snailHit() {
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("hit")),
             QStringLiteral("mob.snail.hide"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("hide")),
             QStringLiteral("mob.snail.hide"));
    QCOMPARE(CharacterSystem::animationForFamilyState(QStringLiteral("snail"), QStringLiteral("dead")),
             QStringLiteral("mob.snail.dead"));
}

void DomainSystemsTest::animationForFamilyState_player() {
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

void DomainSystemsTest::setState_skipsWhenSame() {
    CharacterObject character;
    character.state = QStringLiteral("idle");
    character.frameCount = 8;
    character.actionDurationMs = 0;

    // 调用 setState 传入相同参数 → 不修改
    CharacterSystem::setState(character, QStringLiteral("idle"), 8, 90, 0);

    QCOMPARE(character.state, QStringLiteral("idle"));
    QCOMPARE(character.frameIndex, 0);  // 初始值
}

void DomainSystemsTest::setState_resetsFrameIndex() {
    CharacterObject character;
    character.state = QStringLiteral("run");
    character.frameIndex = 5;

    CharacterSystem::setState(character, QStringLiteral("idle"), 1, 90);

    QCOMPARE(character.state, QStringLiteral("idle"));
    QCOMPARE(character.frameIndex, 0);
    QCOMPARE(character.animationKey, QStringLiteral("enemy.idle")); // 未设 animationFamily，默认 "enemy"
}

void DomainSystemsTest::updateAnimation_advancesFrame() {
    CharacterObject character;
    character.animationFamily = QStringLiteral("small_bee");
    character.state = QStringLiteral("idle");
    character.frameCount = 4;
    character.frameIntervalMs = 90;
    character.frameElapsedMs = 0;
    character.frameIndex = 0;
    character.actionDurationMs = 0;
    character.animationKey = CharacterSystem::animationForFamilyState(character.animationFamily, character.state);

    // 经过 90ms → 前进一帧
    CharacterSystem::updateAnimation(character, 90);

    QCOMPARE(character.frameIndex, 1);
    QCOMPARE(character.frameElapsedMs, 0);
}

void DomainSystemsTest::beginAttack_incrementsSerial() {
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

// ══════════════════════════════════════════════════════════════
// NpcSystem 测试
// ══════════════════════════════════════════════════════════════

void DomainSystemsTest::updateSnail_startsMovingRight() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.spawnX = 400;
    snail.patrolRange = 80;
    snail.npcMoveSpeed = 1.0;
    snail.moveDirection = 0;  // 刚出生，未设方向
    snail.position = QPointF(400, 100);
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    // 应向右侧移动
    QCOMPARE(snail.moveDirection, 1);
    QVERIFY(snail.velocity.x() > 0);
    QCOMPARE(snail.velocity.x(), 1.0);
}

void DomainSystemsTest::updateSnail_reversesAtRightBound() {
    CharacterObject snail;
    snail.animationFamily = QStringLiteral("snail");
    snail.alive = true;
    snail.state = QStringLiteral("idle");
    snail.spawnX = 400;
    snail.patrolRange = 80;
    snail.npcMoveSpeed = 1.0;
    snail.moveDirection = 1;   // 向右走
    snail.position = QPointF(480, 100);  // 到达右边界 (400+80=480)
    snail.attackCooldownRemainingMs = 0;

    NpcSystem::updateSnail(snail, nullptr, 16, WorldTuning{});

    QCOMPARE(snail.moveDirection, -1);  // 掉头向左
    QCOMPARE(snail.facingLeft, true);
}

void DomainSystemsTest::updateSnail_stopsWhenDead() {
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

void DomainSystemsTest::updateSnail_stopsWhenHiding() {
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

void DomainSystemsTest::updateSnail_cooldownDecreases() {
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

void DomainSystemsTest::updateNpc_beeChasesPlayer() {
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
    player.position = QPointF(300, 200);  // 玩家在蜜蜂左边 200px

    WorldEvents events;
    NpcSystem::updateNpc(bee, &player, 16, WorldTuning{}, events);

    // 蜜蜂应朝玩家（左侧）移动
    QCOMPARE(bee.moveDirection, -1);
    QVERIFY(bee.velocity.x() < 0);
    QVERIFY(bee.facingLeft);
}

// ══════════════════════════════════════════════════════════════
// CharacterFactory 测试
// ══════════════════════════════════════════════════════════════

void DomainSystemsTest::createPlayer_hasCorrectDefaults() {
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

void DomainSystemsTest::createSmallBee_hasCorrectDefaults() {
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

void DomainSystemsTest::createSnail_hasCorrectDefaults() {
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

void DomainSystemsTest::createByType_unknownReturnsEmpty() {
    WorldTuning tuning;
    const auto unknown = CharacterFactory::createByType(QStringLiteral("dragon"), 0, 0, tuning);
    QVERIFY(unknown.id.isEmpty());  // 未知类型返回空对象
}

// ══════════════════════════════════════════════════════════════
// SceneBuilder 测试
// ══════════════════════════════════════════════════════════════

void DomainSystemsTest::buildOriginalScene_hasTerrainAndMobs() {
    const auto result = SceneBuilder::build(SceneId::OriginalFactory, 1200, 760, 0, 0, 1200, 760);

    // 应有 1 张背景
    QCOMPARE(result.mapLayers.size(), 1);
    QCOMPARE(result.mapLayers[0].imageKey, QStringLiteral("scene.factory.background"));

    // 应有 3 个地形块（地面 + 左平台 + 右平台）
    QCOMPARE(result.terrain.size(), 3);
    QCOMPARE(result.terrain[0].id, QStringLiteral("original_ground"));

    // 应有 2 个怪物出生点（1 蜜蜂 + 1 蜗牛）
    QCOMPARE(result.mobSpawns.size(), 2);
    QCOMPARE(result.mobSpawns[0].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[1].mobType, QStringLiteral("snail"));

    // 可玩区域
    QCOMPARE(result.playableLeft, 0);
    QCOMPARE(result.playableRight, 1200);
}

void DomainSystemsTest::buildCustomMapScene_hasThreeMobs() {
    const auto result = SceneBuilder::build(SceneId::CustomMap, 1200, 760, 0, 0, 1200, 760);

    // 应有 3 个怪物出生点（2 蜜蜂 + 1 蜗牛）
    QCOMPARE(result.mobSpawns.size(), 3);
    QCOMPARE(result.mobSpawns[0].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[1].mobType, QStringLiteral("small_bee"));
    QCOMPARE(result.mobSpawns[2].mobType, QStringLiteral("snail"));

    // 应有至少 4 个地形块（地面 + 中间平台 + 阶梯1 + 阶梯2）
    QVERIFY(result.terrain.size() >= 4);
}

} // namespace skybound

QTEST_APPLESS_MAIN(skybound::DomainSystemsTest)

#include "DomainSystemsTest.moc"
