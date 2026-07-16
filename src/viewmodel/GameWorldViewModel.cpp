#include "viewmodel/GameWorldViewModel.h"

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

// 在地形上找一个安全的出生 X 坐标：选取最宽的地形块，在其靠右位置生成
qreal findSpawnXOnTerrain(const QList<TerrainPiece>& terrain, qreal actorWidth, qreal playableRight) {
    qreal bestX = 0;
    qreal bestWidth = 0;
    for (const auto& piece : terrain) {
        if (!piece.solid) continue;
        const qreal w = piece.rect.width();
        if (w > bestWidth) {
            bestWidth = w;
            bestX = piece.rect.left() + w * 0.72;
        }
    }
    return std::clamp(bestX - actorWidth / 2, 0.0, playableRight - actorWidth);
}
} // namespace

GameWorldViewModel::GameWorldViewModel(const ResourceManager& resources, QObject* parent)
    : QObject(parent)
    , resources_(resources) {
    initializeWorld();
}

QVariantList GameWorldViewModel::characters() const {
    QVariantList result;
    for (auto it = characters_.constBegin(); it != characters_.constEnd(); ++it) {
        result.push_back(characterToVariant(it.value()));
    }
    return result;
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
    if (currentScene_ == SceneId::CustomMap) return QStringLiteral("custom_map");
    return QStringLiteral("background2_factory");
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
    applyCombatResult(WorldProcessor::resolveCombat(characters_, resolvedAttackTokens_), events);
    updateMovementSounds(events);
    updateDeathState();

    emitEvents(events);
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
        emit charactersChanged();
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
    emit charactersChanged();
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
    emit charactersChanged();
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
    emit charactersChanged();
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
    emit charactersChanged();
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
    applyCombatResult(CombatSystem::checkAttackHits(*p, characters_, resolvedAttackTokens_), events);
    events.sounds.push_back(isRollAttack ? QStringLiteral("player.attack.2") : QStringLiteral("player.attack.1"));
    emitEvents(events);
    emit charactersChanged();
}

void GameWorldViewModel::playerTakeHit(int damage) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    WorldEvents events;
    applyCombatResult(CombatSystem::applyDamageToPlayer(*p, damage), events);
    updateDeathState();
    emitEvents(events);
    emit charactersChanged();
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
    emit charactersChanged();
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
        emit charactersChanged();
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
    emit charactersChanged();
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
    applyCombatResult(CombatSystem::checkAttackHits(*p, characters_, resolvedAttackTokens_), events);
    events.sounds.push_back(QStringLiteral("player.attack.2"));
    emitEvents(events);
    emit charactersChanged();
}

void GameWorldViewModel::initializeWorld() {
    characters_.clear();
    terrain_.clear();
    mapLayers_.clear();
    currentScene_ = SceneId::OriginalFactory;
    updateMapGeometry();

    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - tuning_.actorHeight : terrain_.front().rect.top() - tuning_.actorHeight;
    const qreal playerX = (playableLeft_ + playableRight_ - tuning_.actorWidth) / 2.0;
    const qreal beeX = findSpawnXOnTerrain(terrain_, tuning_.actorWidth, playableRight_);

    const auto playerCharacter = CharacterFactory::createPlayer(playerX, spawnY, tuning_);
    const auto bee = CharacterFactory::createSmallBee(beeX, spawnY, tuning_);
    characters_.insert(playerCharacter.id, playerCharacter);
    characters_.insert(bee.id, bee);
}

void GameWorldViewModel::updateMapGeometry() {
    mapAspect_ = (currentScene_ == SceneId::OriginalFactory || currentScene_ == SceneId::CustomMap || currentScene_ == SceneId::NewForestMap) ? 1695.0 / 725.0 : 2360.0 / 725.0;
    mapWidth_ = std::max(viewportWidth_, viewportHeight_ * mapAspect_);
    mapHeight_ = mapWidth_ / mapAspect_;
    mapX_ = (viewportWidth_ - mapWidth_) / 2.0;
    mapY_ = viewportHeight_ - mapHeight_;

    const auto scene = SceneBuilder::build(currentScene_, viewportWidth_, viewportHeight_, mapX_, mapY_, mapWidth_, mapHeight_);
    mapLayers_ = scene.mapLayers;
    terrain_ = scene.terrain;
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

    const qreal beeX = findSpawnXOnTerrain(terrain_, tuning_.actorWidth, playableRight_);
    const auto bee = CharacterFactory::createSmallBee(beeX, spawnY, tuning_);
    characters_.insert(bee.id, bee);

    emit viewportChanged();
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
    emit charactersChanged();
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

QVariantMap GameWorldViewModel::characterToVariant(const CharacterObject& character) const {
    QVariantMap item;
    item["id"] = character.id;
    item["kind"] = character.kind;
    item["hp"] = character.hp;
    item["maxHp"] = character.maxHp;
    item["lives"] = character.lives;
    item["x"] = character.position.x();
    item["y"] = character.position.y();
    item["width"] = tuning_.actorWidth;
    item["height"] = tuning_.actorHeight;
    item["position"] = pointToVariant(character.position);
    item["velocity"] = pointToVariant(character.velocity);
    item["facingLeft"] = character.facingLeft;
    item["alive"] = character.alive;
    item["state"] = character.state;
    const QVariantMap sprite = animationPresentation(character.animationKey, character.frameIndex);
    const QString fallbackVfxKey = character.kind == QStringLiteral("enemy")
        ? QStringLiteral("mob.small_bee.vfx.attack_%1").arg(character.attackDirection)
        : QStringLiteral("player.vfx.attack.%1").arg(character.attackDirection);
    const QString vfxKey = character.attackVfxKey.isEmpty() ? fallbackVfxKey : character.attackVfxKey;
    const QVariantMap attackVfx = animationPresentation(vfxKey, character.frameIndex);
    item["spriteSource"] = sprite.value("source");
    item["spriteIsSheet"] = sprite.value("isSheet");
    item["spriteFrameCount"] = sprite.value("frameCount");
    item["spriteFrameIndex"] = sprite.value("frameIndex");
    item["spriteFrameWidth"] = sprite.value("frameWidth");
    item["spriteFrameHeight"] = sprite.value("frameHeight");
    item["frameIndex"] = character.frameIndex;
    item["frameCount"] = character.frameCount;
    item["attackDirection"] = character.attackDirection;
    item["vfxSource"] = attackVfx.value("source");
    item["vfxIsSheet"] = attackVfx.value("isSheet");
    item["vfxFrameCount"] = attackVfx.value("frameCount");
    item["vfxFrameIndex"] = attackVfx.value("frameIndex");
    item["vfxFrameWidth"] = attackVfx.value("frameWidth");
    item["vfxFrameHeight"] = attackVfx.value("frameHeight");
    item["attackVfxSize"] = std::max(character.attackBox.rect.width(), character.attackBox.rect.height());
    item["hurtbox"] = boxToVariant(character.hurtbox);
    item["attackBox"] = boxToVariant(character.attackBox);
    return item;
}

} // namespace skybound
