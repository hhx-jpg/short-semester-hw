#include "viewmodel/GameWorldViewModel.h"
#include "viewmodel/CharacterListModel.h"

#include "domain/CharacterFactory.h"
#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"
#include "domain/CombatSystem.h"
#include "domain/SceneBuilder.h"
#include "domain/WorldProcessor.h"
#include "service/ResourceManager.h"

#include <QVariantMap>
#include <algorithm>
#include <cmath>

namespace skybound {

namespace {

constexpr int kFrameMs = 16;

} // namespace

GameWorldViewModel::GameWorldViewModel(const ResourceManager& resources, QObject* parent)
    : QObject(parent)
    , resources_(resources)
    , characterModel_(new CharacterListModel(this)) {
    initializeWorld();
}

QVariantList GameWorldViewModel::mapLayers() const {
    QVariantList result;
    for (const auto& layer : mapLayers_) {
        QVariantMap item;
        item["id"] = layer.id;
        item["source"] = resources_.image(layer.imageKey);
        item["x"] = layer.rect.x();
        item["y"] = layer.rect.y();
        item["width"] = layer.rect.width();
        item["height"] = layer.rect.height();
        item["opacity"] = layer.opacity;
        result.push_back(item);
    }
    return result;
}

QVariantList GameWorldViewModel::terrain() const {
    QVariantList result;
    for (const auto& piece : terrain_) {
        QVariantMap item;
        item["id"] = piece.id;
        item["kind"] = piece.kind;
        item["x"] = piece.rect.x();
        item["y"] = piece.rect.y();
        item["width"] = piece.rect.width();
        item["height"] = piece.rect.height();
        item["solid"] = piece.solid;
        result.push_back(item);
    }
    return result;
}

QVariantList GameWorldViewModel::debugBoxes() const {
    QVariantList result;

    for (const auto& piece : terrain_) {
        QVariantMap box = rectToVariant(piece.rect);
        box["boxType"] = QStringLiteral("terrain");
        box["kind"] = piece.kind;
        box["active"] = true;
        result.push_back(box);
    }

    for (auto it = characters_.constBegin(); it != characters_.constEnd(); ++it) {
        const auto& character = it.value();
        QVariantMap hurtbox = rectToVariant(character.hurtbox.rect);
        hurtbox["characterId"] = character.id;
        hurtbox["boxType"] = QStringLiteral("hurtbox");
        hurtbox["kind"] = character.kind;
        hurtbox["active"] = character.alive && character.hurtbox.active;
        result.push_back(hurtbox);

        QVariantMap attackBox = rectToVariant(character.attackBox.rect);
        attackBox["characterId"] = character.id;
        attackBox["boxType"] = QStringLiteral("attackBox");
        attackBox["kind"] = character.kind;
        attackBox["active"] = character.alive && character.attackBox.active;
        result.push_back(attackBox);
    }
    return result;
}

int GameWorldViewModel::damageCount() const {
    return damageCount_;
}

QString GameWorldViewModel::currentScene() const {
    if (currentScene_ == SceneId::OriginalFactory) return QStringLiteral("original_factory");
    if (currentScene_ == SceneId::Background2Factory) return QStringLiteral("background2_factory");
    if (currentScene_ == SceneId::CustomMap) return QStringLiteral("custom_map");
    if (currentScene_ == SceneId::NewForestMap) return QStringLiteral("new_forest_map");
    if (currentScene_ == SceneId::ForestMap3) return QStringLiteral("forest_map3");
    return QStringLiteral("original_factory");
}

qreal GameWorldViewModel::mapAspect() const {
    return mapAspect_;
}

qreal GameWorldViewModel::mapWidth() const {
    return mapWidth_;
}

qreal GameWorldViewModel::mapHeight() const {
    return mapHeight_;
}

qreal GameWorldViewModel::mapX() const {
    return mapX_;
}

qreal GameWorldViewModel::mapY() const {
    return mapY_;
}

int GameWorldViewModel::playerHp() const {
    const auto* p = player();
    return p ? p->hp : 0;
}

int GameWorldViewModel::playerMaxHp() const {
    const auto* p = player();
    return p ? p->maxHp : 0;
}

int GameWorldViewModel::playerHeartCount() const {
    const auto* p = player();
    return p ? p->hp / 10 : 0;
}

qreal GameWorldViewModel::chargeProgress() const {
    return chargeProgress_;
}

QString GameWorldViewModel::gameState() const {
    return gameState_;
}

void GameWorldViewModel::startGame() {
    reset();
    if (gameState_ != QStringLiteral("playing")) {
        gameState_ = QStringLiteral("playing");
        emit gameStateChanged();
        emit soundRequested(QStringLiteral("bgm.factory.start"));
    }
}

void GameWorldViewModel::returnToStartMenu() {
    if (gameState_ == QStringLiteral("start")) {
        return;
    }

    chargePressed_ = false;
    chargeProgress_ = 0.0;
    aimingUp_ = false;
    aimingDown_ = false;
    playerRunningSoundActive_ = false;
    enemyRunningSoundActive_ = false;
    emit soundRequested(QStringLiteral("player.run.stop"));
    emit soundRequested(QStringLiteral("enemy.run.stop"));
    emit soundRequested(QStringLiteral("bgm.factory.stop"));
    gameState_ = QStringLiteral("start");
    emit chargeProgressChanged();
    emit gameStateChanged();
}

bool GameWorldViewModel::playerFacingLeft() const {
    const auto* p = player();
    return p ? p->facingLeft : true;
}

void GameWorldViewModel::reset() {
    damageCount_ = 0;
    chargePressed_ = false;
    chargeProgress_ = 0.0;
    aimingUp_ = false;
    aimingDown_ = false;
    resolvedAttackTokens_.clear();
    playerRunningSoundActive_ = false;
    enemyRunningSoundActive_ = false;
    initializeWorld();
    emit damageCountChanged();
    emit chargeProgressChanged();
    notifyWorldDataChanged(true);
    emit worldChanged();
}

void GameWorldViewModel::setViewport(qreal width, qreal height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    viewportWidth_ = width;
    viewportHeight_ = height;
    updateMapGeometry();
    emit viewportChanged();
    notifyWorldDataChanged(true);
    emit worldChanged();
}

void GameWorldViewModel::tick(int deltaMs) {
    if (gameState_ != QStringLiteral("playing")) {
        return;
    }

    const int dt = deltaMs <= 0 ? kFrameMs : deltaMs;
    WorldEvents events;

    updateCharge(dt, events);
    const SceneSwitchRequest sceneSwitch = WorldProcessor::advanceActors(
        characters_, dt, currentScene_, playableLeft_, playableRight_, terrain_, tuning_, chargePressed_, events);

    const bool sceneChanged = sceneSwitch.pending;
    applySceneSwitch(sceneSwitch);
    applyCombatResult(WorldProcessor::resolveCombat(characters_, terrain_, resolvedAttackTokens_), events);
    updateMovementSounds(events);

    // ──────────────────────────────────────────────
    // 坠落出地图底部判定
    //
    // 如果角色（玩家或怪物）的顶部 Y 坐标超过了视口底部以下 100px，
    // 判定为"坠落出地图"，直接置为死亡状态。
    //
    // 死亡后角色会被 WorldProcessor::advanceActors 跳过更新，
    // 玩家死亡会触发 updateDeathState() 切换到死亡画面。
    //
    // viewportHeight_ + 100 作为阈值而非 viewportHeight_ 本身，
    // 提供约 100px 的视觉缓冲，让角色完全掉出屏幕后再死亡，
    // 避免在屏幕边缘闪死。
    // ──────────────────────────────────────────────
    const qreal deathY = viewportHeight_ + 100;  // 死亡阈值 = 视口底部 + 100px 缓冲
    for (auto it = characters_.begin(); it != characters_.end(); ++it) {
        auto& character = it.value();
        if (character.alive && character.position.y() > deathY) {
            character.alive = false;       // 标记死亡（跳过后续更新）
            character.hp = 0;              // 血量归零
            character.velocity = QPointF(0, 0);  // 停止运动
        }
    }

    updateDeathState();

    // ──────────────────────────────────────────────
    // 通关判定
    //
    // 玩家在森林3地图且到达最左上角平台（f3_plat7）时触发通关。
    // 判定条件：
    //   1. 当前场景为 ForestMap3
    //   2. 玩家存活
    //   3. 玩家 X 坐标在平台水平范围内
    //   4. 玩家脚底 Y 坐标接近平台顶面（±8px 容差）
    //
    // 触发后切换到 "win" 状态，弹出通关界面。
    // ──────────────────────────────────────────────
    if (gameState_ == QStringLiteral("playing")) {
        checkWinCondition();
    }

    emitEvents(events);

    // 通过 CharacterListModel（QAbstractListModel）同步角色数据，
    // 只对实际变化的角色发射 dataChanged，避免 Repeater 每帧重建 delegate
    characterModel_->syncFromCharacters(characters_, resources_);

    notifyWorldDataChanged(sceneChanged);
    emit worldChanged();
}

void GameWorldViewModel::playerRun(int direction) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }
    if (chargePressed_ || p->state == QStringLiteral("charge")) {
        p->moveDirection = 0;
        p->velocity.setX(0);
        return;
    }

    p->moveDirection = std::clamp(direction, -1, 1);
    if (p->moveDirection < 0) {
        p->facingLeft = true;
    } else if (p->moveDirection > 0) {
        p->facingLeft = false;
    }

    if ((p->state == QStringLiteral("idle") || p->state == QStringLiteral("run")) && p->moveDirection != 0) {
        CharacterSystem::setState(*p, QStringLiteral("run"), 8, 90);
    }
}

void GameWorldViewModel::playerStopRun(int direction) {
    auto* p = player();
    if (!p) {
        return;
    }

    if ((direction < 0 && p->moveDirection < 0) || (direction > 0 && p->moveDirection > 0)) {
        p->moveDirection = 0;
    }
    if (p->state == QStringLiteral("run") && p->moveDirection == 0) {
        CharacterSystem::setState(*p, QStringLiteral("idle"), 1, 90);
    }
}

void GameWorldViewModel::playerJump() {
    auto* p = player();
    if (!p || !p->alive || chargePressed_ || p->state == QStringLiteral("charge")) {
        return;
    }

    const bool onSurface = std::abs(p->velocity.y()) < 0.01;
    if (!onSurface || p->state == QStringLiteral("jump") || p->state == QStringLiteral("fall")) {
        return;
    }

    p->velocity.setY(tuning_.jumpVelocity);
    CharacterSystem::setState(*p, QStringLiteral("jump"), 8, 70);
    emit soundRequested(QStringLiteral("player.jump"));
}

void GameWorldViewModel::playerRoll() {
    auto* p = player();
    if (!p || !p->alive || chargePressed_ || p->state == QStringLiteral("charge") || p->state == QStringLiteral("roll")) {
        return;
    }

    p->attackVfxKey.clear();
    p->velocity.setX((p->facingLeft ? -1 : 1) * tuning_.moveSpeed * 1.8);
    CharacterSystem::setState(*p, QStringLiteral("roll"), 2, 70, 220);
    emit soundRequested(QStringLiteral("player.roll"));
}

void GameWorldViewModel::playerAttack(const QString& direction) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    const bool isRollAttack = p->state == QStringLiteral("roll");
    p->rollAttack = isRollAttack;
    p->attackVfxKey.clear();

    QString resolved = direction;
    if (resolved.isEmpty()) {
        resolved = p->facingLeft ? QStringLiteral("left") : QStringLiteral("right");
    }
    CharacterSystem::beginAttack(*p, resolved, isRollAttack ? 8 : 8, isRollAttack ? 65 : 55, isRollAttack ? 520 : 440);
    if (isRollAttack) {
        p->state = QStringLiteral("skill");
        p->animationKey = CharacterSystem::animationForFamilyState(p->animationFamily, p->state);
    }
    CollisionSystem::updateCollisionBoxes(*p, tuning_);

    WorldEvents events;
    applyCombatResult(CombatSystem::checkAttackHits(*p, characters_, terrain_, resolvedAttackTokens_), events);
    events.sounds.push_back(isRollAttack ? QStringLiteral("player.attack.2") : QStringLiteral("player.attack.1"));
    emitEvents(events);
}

void GameWorldViewModel::playerTakeHit(int damage) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    WorldEvents events;
    applyCombatResult(CombatSystem::applyDamageToPlayer(*p, damage, p->position), events);
    updateDeathState();
    emitEvents(events);
}

void GameWorldViewModel::playerCastSkill(const QString& skillId) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    CharacterSystem::beginAttack(*p, p->facingLeft ? QStringLiteral("left") : QStringLiteral("right"), 8, 65, 520);
    p->state = QStringLiteral("skill");
    p->animationKey = CharacterSystem::animationForFamilyState(p->animationFamily, p->state);
    emit soundRequested(QStringLiteral("player.attack.2"));
    Q_UNUSED(skillId);
}

void GameWorldViewModel::setAimUpPressed(bool pressed) {
    aimingUp_ = pressed;
}

void GameWorldViewModel::setAimDownPressed(bool pressed) {
    aimingDown_ = pressed;
}

void GameWorldViewModel::setChargePressed(bool pressed) {
    auto* p = player();
    if (pressed) {
        if (!p || !p->alive || p->state == QStringLiteral("attack") || p->state == QStringLiteral("roll") || p->state == QStringLiteral("hit") || p->state == QStringLiteral("skill")) {
            chargePressed_ = false;
            return;
        }

        chargePressed_ = true;
        p->moveDirection = 0;
        p->velocity.setX(0);
        p->rollAttack = false;
        CharacterSystem::setState(*p, QStringLiteral("charge"), 10, 80);
        return;
    }

    chargePressed_ = false;
    chargeProgress_ = 0.0;
    if (p && p->alive && p->state == QStringLiteral("charge")) {
        if (p->velocity.y() < -0.1) {
            CharacterSystem::setState(*p, QStringLiteral("jump"), 10, 70);
        } else if (p->velocity.y() > 0.1) {
            CharacterSystem::setState(*p, QStringLiteral("fall"), 10, 70);
        } else {
            CharacterSystem::setState(*p, QStringLiteral("idle"), 10, 90);
        }
    }
    emit chargeProgressChanged();
}

void GameWorldViewModel::releaseAttack() {
    const bool wasCharged = chargeProgress_ >= 1.0;
    setChargePressed(false);
    if (wasCharged) {
        playerBurstAttack();
        return;
    }

    if (aimingUp_) {
        playerAttack(QStringLiteral("up"));
    } else if (aimingDown_) {
        playerAttack(QStringLiteral("down"));
    } else {
        playerAttack(QString{});
    }
}

void GameWorldViewModel::playerBurstAttack() {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    p->moveDirection = 0;
    p->velocity.setX(0);
    p->rollAttack = false;
    p->attackVfxKey = QStringLiteral("player.vfx.burst");
    CharacterSystem::beginAttack(*p, QStringLiteral("burst"), 9, 65, 585);
    p->state = QStringLiteral("skill");
    p->animationKey = CharacterSystem::animationForFamilyState(p->animationFamily, p->state);
    CollisionSystem::updateCollisionBoxes(*p, tuning_);

    WorldEvents events;
    applyCombatResult(CombatSystem::checkAttackHits(*p, characters_, terrain_, resolvedAttackTokens_), events);
    events.sounds.push_back(QStringLiteral("player.attack.2"));
    emitEvents(events);
}

void GameWorldViewModel::initializeWorld() {
    characters_.clear();
    terrain_.clear();
    mapLayers_.clear();
    mobSpawns_.clear();
    currentScene_ = SceneId::OriginalFactory;
    updateMapGeometry();

    // 在所有地形块中，选取第一个地形块的顶部作为出生 Y 坐标
    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - tuning_.actorHeight : terrain_.front().rect.top() - tuning_.actorHeight;
    const qreal playerX = (playableLeft_ + playableRight_ - tuning_.actorWidth) / 2.0;

    // 创建玩家角色
    const auto playerCharacter = CharacterFactory::createPlayer(playerX, spawnY, tuning_);
    characters_.insert(playerCharacter.id, playerCharacter);

    // 从当前场景的 mobSpawns 配置批量生成怪物
    spawnMobs(mobSpawns_);
}

void GameWorldViewModel::updateMapGeometry() {
    mapAspect_ = (currentScene_ == SceneId::OriginalFactory || currentScene_ == SceneId::CustomMap || currentScene_ == SceneId::NewForestMap || currentScene_ == SceneId::ForestMap3) ? 1695.0 / 725.0 : 2360.0 / 725.0;
    mapWidth_ = std::max(viewportWidth_, viewportHeight_ * mapAspect_);
    mapHeight_ = mapWidth_ / mapAspect_;
    mapX_ = (viewportWidth_ - mapWidth_) / 2.0;
    mapY_ = viewportHeight_ - mapHeight_;

    const auto scene = SceneBuilder::build(currentScene_, viewportWidth_, viewportHeight_, mapX_, mapY_, mapWidth_, mapHeight_);
    mapLayers_ = scene.mapLayers;
    terrain_ = scene.terrain;
    mobSpawns_ = scene.mobSpawns;  // 保存本场景的怪物出生点配置，用于后续生成怪物
    playableLeft_ = scene.playableLeft;
    playableRight_ = scene.playableRight;
}

void GameWorldViewModel::switchToScene(SceneId scene, EntrySide entrySide) {
    if (currentScene_ == scene) {
        return;
    }

    CharacterObject playerSnapshot;
    if (auto* p = player()) {
        playerSnapshot = *p;
    }

    currentScene_ = scene;
    updateMapGeometry();

    const qreal spawnX = entrySide == EntrySide::Left ? playableLeft_ + 24 : playableRight_ - tuning_.actorWidth - 24;
    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - tuning_.actorHeight : terrain_.front().rect.top() - tuning_.actorHeight;
    characters_.clear();

    playerSnapshot.id = QStringLiteral("player");
    playerSnapshot.kind = QStringLiteral("player");
    playerSnapshot.position = QPointF(spawnX, spawnY);
    playerSnapshot.velocity = QPointF(0, 0);
    playerSnapshot.moveDirection = 0;
    playerSnapshot.attackBox.active = false;
    CharacterSystem::setState(playerSnapshot, QStringLiteral("idle"), 1, 90);
    CollisionSystem::updateCollisionBoxes(playerSnapshot, tuning_);
    characters_.insert(playerSnapshot.id, playerSnapshot);

    // 从新场景的 mobSpawns 配置批量生成怪物
    spawnMobs(mobSpawns_);

    emit viewportChanged();
}

// ──────────────────────────────────────────────
// 按配置批量生成怪物
//
// 遍历 mobSpawns 列表，对每条配置：
//   1. 调用 CharacterFactory::createByType() 按类型创建怪物对象
//   2. 如果创建成功（id 非空），插入到 characters_ 哈希表中
//   3. 自动处理 ID 冲突：同类型多个怪物时追加数字后缀
//      （例如第二只蜗牛 ID 变为 "snail_2"）
//
// 自动放置逻辑：
//   如果 spawn.y == 0，则将怪物放置在最宽地形块的顶部，
//   确保怪物不会生成在半空中或地下。
//
// 参数：
//   spawns — 怪物出生点配置列表，来自 SceneBuildResult.mobSpawns
// ──────────────────────────────────────────────
void GameWorldViewModel::spawnMobs(const QList<MobSpawn>& spawns) {
    // 找最宽的地形块，用于自动计算 Y 坐标
    qreal bestTerrainTop = viewportHeight_ - tuning_.actorHeight;
    if (!terrain_.isEmpty()) {
        qreal bestWidth = 0;
        for (const auto& piece : terrain_) {
            if (piece.solid && piece.rect.width() > bestWidth) {
                bestWidth = piece.rect.width();
                bestTerrainTop = piece.rect.top();
            }
        }
    }

    // 用于生成唯一 ID 的计数器（按怪物类型分别计数）
    QHash<QString, int> typeCounters;

    for (const auto& spawn : spawns) {
        qreal x = spawn.x;
        // y == 0 时自动放置在最宽地形块的顶部
        qreal y = (spawn.y == 0) ? bestTerrainTop - tuning_.actorHeight : spawn.y;

        auto mob = CharacterFactory::createByType(spawn.mobType, x, y, tuning_);
        // 创建成功则插入到角色列表；未知类型返回空对象，静默跳过
        if (!mob.id.isEmpty()) {
            // ── ID 去重：同类型怪物共用同一个 base ID（如 "snail_1"），
            //    第二只起追加计数后缀，防止哈希表覆盖
            const int counter = ++typeCounters[spawn.mobType];
            if (counter > 1) {
                mob.id = spawn.mobType + QStringLiteral("_") + QString::number(counter);
            }
            characters_.insert(mob.id, mob);
        }
    }
}

void GameWorldViewModel::applySceneSwitch(const SceneSwitchRequest& sceneSwitch) {
    if (sceneSwitch.pending) {
        switchToScene(sceneSwitch.scene, sceneSwitch.entrySide);
    }
}

void GameWorldViewModel::updateCharge(int deltaMs, WorldEvents& events) {
    if (!chargePressed_) {
        return;
    }

    chargeProgress_ = std::min<qreal>(1.0, chargeProgress_ + deltaMs / tuning_.chargeThresholdMs);
    events.chargeProgressChanged = true;
}

void GameWorldViewModel::updateMovementSounds(WorldEvents& events) {
    const auto* p = player();
    const bool playerRunning = p && p->alive && p->state == QStringLiteral("run");
    if (playerRunning != playerRunningSoundActive_) {
        playerRunningSoundActive_ = playerRunning;
        events.sounds.push_back(playerRunning ? QStringLiteral("player.run.start") : QStringLiteral("player.run.stop"));
    }

    bool enemyRunning = false;
    for (auto it = characters_.constBegin(); it != characters_.constEnd(); ++it) {
        const auto& character = it.value();
        if (character.kind == QStringLiteral("enemy") && character.alive && character.state == QStringLiteral("run")) {
            enemyRunning = true;
            break;
        }
    }
    if (enemyRunning != enemyRunningSoundActive_) {
        enemyRunningSoundActive_ = enemyRunning;
        events.sounds.push_back(enemyRunning ? QStringLiteral("enemy.run.start") : QStringLiteral("enemy.run.stop"));
    }
}

void GameWorldViewModel::applyCombatResult(const CombatResult& result, WorldEvents& events) {
    if (result.damageCountDelta != 0) {
        damageCount_ += result.damageCountDelta;
        events.damageCountChanged = true;
    }
    if (result.playerStatsChanged) {
        emit playerStatsChanged();
    }
    events.sounds.append(result.sounds);
}

void GameWorldViewModel::updateDeathState() {
    const auto* p = player();
    if (gameState_ == QStringLiteral("playing") && (!p || !p->alive)) {
        chargePressed_ = false;
        chargeProgress_ = 0.0;
        aimingUp_ = false;
        aimingDown_ = false;
        playerRunningSoundActive_ = false;
        enemyRunningSoundActive_ = false;
        emit soundRequested(QStringLiteral("player.run.stop"));
        emit soundRequested(QStringLiteral("enemy.run.stop"));
        emit soundRequested(QStringLiteral("bgm.factory.stop"));
        gameState_ = QStringLiteral("dead");
        emit chargeProgressChanged();
        emit gameStateChanged();
    }
}

// ──────────────────────────────────────────────
// 通关判定
//
// 检测玩家是否到达森林地图3最左上角的平台（f3_plat7）。
// 该平台碰撞箱为 QRectF(1, 297, 157, 43)，顶面 y = 297。
//
// 判定通过后，将游戏状态切换为 "win"，
// 停止所有音效并弹出通关界面。
// ──────────────────────────────────────────────
void GameWorldViewModel::checkWinCondition() {
    if (currentScene_ != SceneId::ForestMap3) {
        return;
    }

    const auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    // f3_plat7 碰撞箱：QRectF(1, 297, 157, 43)
    // 玩家脚底 y = position.y + charHeight
    constexpr qreal kPlatLeft   = 1.0;
    constexpr qreal kPlatRight  = 158.0;   // 1 + 157
    constexpr qreal kPlatTop    = 297.0;
    constexpr qreal kTolerance  = 8.0;     // Y 方向容差（像素）

    const qreal playerLeft = p->position.x();
    const qreal playerFeetY = p->position.y() + p->charHeight;

    const bool inXRange = playerLeft >= kPlatLeft && playerLeft <= kPlatRight;
    const bool onSurface = std::abs(playerFeetY - kPlatTop) <= kTolerance;

    if (inXRange && onSurface) {
        // ── 触发通关 ──
        chargePressed_ = false;
        chargeProgress_ = 0.0;
        aimingUp_ = false;
        aimingDown_ = false;
        playerRunningSoundActive_ = false;
        enemyRunningSoundActive_ = false;
        emit soundRequested(QStringLiteral("player.run.stop"));
        emit soundRequested(QStringLiteral("enemy.run.stop"));
        emit soundRequested(QStringLiteral("bgm.factory.stop"));
        gameState_ = QStringLiteral("win");
        emit chargeProgressChanged();
        emit gameStateChanged();
    }
}

void GameWorldViewModel::emitEvents(const WorldEvents& events) {
    for (const auto& sound : events.sounds) {
        emit soundRequested(sound);
    }
    if (events.damageCountChanged) {
        emit damageCountChanged();
    }
    if (events.chargeProgressChanged) {
        emit chargeProgressChanged();
    }
    if (events.viewportChanged) {
        emit viewportChanged();
    }
}

void GameWorldViewModel::notifyWorldDataChanged(bool sceneChanged) {
    emit debugBoxesChanged();
    if (sceneChanged) {
        emit mapLayersChanged();
        emit terrainChanged();
        emit currentSceneChanged();
    }
}

QVariantMap GameWorldViewModel::animationPresentation(const QString& key, int frameIndex) const {
    QVariantMap presentation;
    const bool isSheet = resources_.isSpriteSheetAnimation(key);
    const int frameCount = isSheet ? resources_.spriteSheetFrameCount(key) : resources_.animationFrameCount(key);
    presentation["isSheet"] = isSheet;
    presentation["source"] = isSheet ? resources_.spriteSheetImage(key) : resources_.animationFrame(key, frameIndex);
    presentation["frameCount"] = frameCount;
    presentation["frameIndex"] = frameCount > 0 ? std::clamp(frameIndex, 0, frameCount - 1) : 0;
    presentation["frameWidth"] = isSheet ? resources_.spriteSheetFrameWidth(key) : 0;
    presentation["frameHeight"] = isSheet ? resources_.spriteSheetFrameHeight(key) : 0;
    return presentation;
}

CharacterObject* GameWorldViewModel::player() {
    auto it = characters_.find(QStringLiteral("player"));
    return it == characters_.end() ? nullptr : &it.value();
}

const CharacterObject* GameWorldViewModel::player() const {
    auto it = characters_.constFind(QStringLiteral("player"));
    return it == characters_.constEnd() ? nullptr : &it.value();
}

QVariantMap GameWorldViewModel::rectToVariant(const QRectF& rect) const {
    QVariantMap result;
    result["x"] = rect.x();
    result["y"] = rect.y();
    result["width"] = rect.width();
    result["height"] = rect.height();
    return result;
}

QVariantMap GameWorldViewModel::pointToVariant(const QPointF& point) const {
    QVariantMap result;
    result["x"] = point.x();
    result["y"] = point.y();
    return result;
}

QVariantMap GameWorldViewModel::boxToVariant(const CollisionBox& box) const {
    QVariantMap result = rectToVariant(box.rect);
    result["active"] = box.active;
    return result;
}

} // namespace skybound
