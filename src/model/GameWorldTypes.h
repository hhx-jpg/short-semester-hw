#pragma once

#include <QList>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>

namespace skybound {

enum class SceneId {
    OriginalFactory,
    Background2Factory,
    CustomMap,
    NewForestMap,
    ForestMap3,
};

enum class EntrySide {
    Left,
    Right,
};

struct CollisionBox {
    QRectF rect;
    bool active = true;
};

struct CharacterObject {
    QString id;
    QString kind;
    int hp = 100;
    int maxHp = 100;
    int lives = 1;
    QPointF position;
    QPointF velocity;
    bool facingLeft = true;
    bool alive = true;
    QString state = QStringLiteral("idle");
    QString animationKey = QStringLiteral("enemy.idle");
    int frameIndex = 0;
    int frameCount = 1;
    int frameIntervalMs = 90;
    int frameElapsedMs = 0;
    int actionElapsedMs = 0;
    int actionDurationMs = 0;
    QString attackDirection = QStringLiteral("left");
    QString animationFamily = QStringLiteral("enemy");
    QString attackVfxKey;
    int attackSerial = 0;
    int attackDamage = 10;
    int attackCooldownMs = 900;
    int attackCooldownRemainingMs = 0;
    int moveDirection = 0;
    bool rollAttack = false;
    bool aiControlled = false;
    qreal detectionRange = 600;
    qreal attackRange = 95;
    qreal npcMoveSpeed = 2.2;
    qreal charWidth = 90;      // 角色渲染宽度（像素），默认 90，蜗牛等小体型可单独设置
    qreal charHeight = 90;     // 角色渲染高度（像素），默认 90
    CollisionBox hurtbox;
    CollisionBox attackBox;
};

struct TerrainPiece {
    QString id;
    QString kind;
    QRectF rect;
    bool solid = true;
};

struct MapLayer {
    QString id;
    QString imageKey;
    QRectF rect;
    qreal opacity = 1.0;
};

struct WorldTuning {
    qreal gravity = 0.62;
    qreal jumpVelocity = -13.0;
    qreal maxFallVelocity = 16.0;
    qreal moveSpeed = 4.6;
    qreal actorWidth = 90;
    qreal actorHeight = 90;
    qreal playerHurtboxOffsetX = 23;
    qreal playerHurtboxOffsetY = 12;
    qreal playerHurtboxWidth = 44;
    qreal playerHurtboxHeight = 72;
    qreal enemyHurtboxOffsetX = 23;
    qreal enemyHurtboxOffsetY = 12;
    qreal enemyHurtboxWidth = 44;
    qreal enemyHurtboxHeight = 72;
    qreal attackBoxWidth = 138;
    qreal attackBoxHeight = 60;
    qreal attackOffsetLeftX = -94;
    qreal attackOffsetLeftY = 18;
    qreal attackOffsetRightX = 46;
    qreal attackOffsetRightY = 18;
    qreal attackOffsetUpX = 15;
    qreal attackOffsetUpY = -96;
    qreal attackOffsetDownX = 15;
    qreal attackOffsetDownY = 54;
    qreal rollAttackBoxWidth = 240;
    qreal rollAttackBoxHeight = 36;
    qreal burstBoxWidth = 200;
    qreal burstBoxHeight = 200;
    qreal chargeThresholdMs = 350.0;
};

struct WorldEvents {
    QStringList sounds;
    bool damageCountChanged = false;
    bool chargeProgressChanged = false;
    bool viewportChanged = false;
};

struct CombatResult {
    int damageCountDelta = 0;
    QStringList sounds;
    bool playerStatsChanged = false;
};

struct SceneSwitchRequest {
    bool pending = false;
    SceneId scene = SceneId::OriginalFactory;
    EntrySide entrySide = EntrySide::Left;
};

// ──────────────────────────────────────────────
// 怪物出生点配置
//
// 每个 MobSpawn 定义一张地图上一个怪物的：
//   - mobType : 怪物类型字符串，传给 CharacterFactory::createByType()
//              目前支持："small_bee"（小蜜蜂）、"snail"（蜗牛）
//   - x       : 出生位置 X 坐标（相对于场景左边界）
//   - y       : 出生位置 Y 坐标（0 表示自动放置在最宽地形块上）
// ──────────────────────────────────────────────
struct MobSpawn {
    QString mobType;   // 怪物类型："small_bee" / "snail"
    qreal x = 0;       // 出生 X 坐标
    qreal y = 0;       // 出生 Y 坐标（0 = 自动放置在地形上）
};

struct SceneBuildResult {
    QList<MapLayer> mapLayers;
    QList<TerrainPiece> terrain;
    QList<MobSpawn> mobSpawns;   // 本场景的怪物出生点列表
    qreal playableLeft = 0;
    qreal playableRight = 0;
};

} // namespace skybound
