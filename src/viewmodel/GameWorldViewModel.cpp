#include "viewmodel/GameWorldViewModel.h"

#include "domain/CharacterFactory.h"
#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"
#include "domain/CombatSystem.h"
#include "domain/NpcSystem.h"
#include "domain/PhysicsSystem.h"
#include "domain/SceneBuilder.h"

#include <QVariantMap>
#include <algorithm>
#include <cmath>

namespace skybound {

namespace {
constexpr int kFrameMs = 16;
} // namespace

GameWorldViewModel::GameWorldViewModel(QObject* parent)
    : QObject(parent) {
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
        item["imageKey"] = layer.imageKey;
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
    return currentScene_ == SceneId::OriginalFactory ? QStringLiteral("original_factory") : QStringLiteral("background2_factory");
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

void GameWorldViewModel::reset() {
    damageCount_ = 0;
    chargePressed_ = false;
    chargeProgress_ = 0.0;
    resolvedAttackTokens_.clear();
    initializeWorld();
    emit damageCountChanged();
    emit chargeProgressChanged();
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
    emit worldChanged();
}

void GameWorldViewModel::tick(int deltaMs) {
    const int dt = deltaMs <= 0 ? kFrameMs : deltaMs;
    WorldEvents events;
    SceneSwitchRequest sceneSwitch;

    updateCharge(dt, events);

    for (auto it = characters_.begin(); it != characters_.end(); ++it) {
        auto& character = it.value();
        if (!character.alive) {
            continue;
        }

        if (character.aiControlled) {
            NpcSystem::updateNpc(character, player(), dt, tuning_, events);
        }

        CharacterSystem::updateAnimation(character, dt);
        PhysicsSystem::updateCharacterPhysics(character, dt, currentScene_, playableLeft_, playableRight_, terrain_, tuning_, chargePressed_, sceneSwitch);
        CollisionSystem::updateCollisionBoxes(character, tuning_);
    }

    applySceneSwitch(sceneSwitch);

    for (auto it = characters_.begin(); it != characters_.end(); ++it) {
        CombatSystem::checkAttackHits(it.value(), characters_, resolvedAttackTokens_, damageCount_, events);
    }

    emitEvents(events);
    emit worldChanged();
}

void GameWorldViewModel::playerRun(int direction) {
    auto* p = player();
    if (!p || !p->alive) {
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
    emit worldChanged();
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
    emit worldChanged();
}

void GameWorldViewModel::playerJump() {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    const bool onSurface = std::abs(p->velocity.y()) < 0.01;
    if (!onSurface || p->state == QStringLiteral("jump") || p->state == QStringLiteral("fall")) {
        return;
    }

    p->velocity.setY(tuning_.jumpVelocity);
    CharacterSystem::setState(*p, QStringLiteral("jump"), 8, 70);
    emit soundRequested(QStringLiteral("player.jump"));
    emit worldChanged();
}

void GameWorldViewModel::playerRoll() {
    auto* p = player();
    if (!p || !p->alive || p->state == QStringLiteral("roll")) {
        return;
    }

    p->velocity.setX((p->facingLeft ? -1 : 1) * tuning_.moveSpeed * 1.8);
    CharacterSystem::setState(*p, QStringLiteral("roll"), 8, 55, 360);
    emit soundRequested(QStringLiteral("player.roll"));
    emit worldChanged();
}

void GameWorldViewModel::playerAttack(const QString& direction) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    const bool isRollAttack = p->state == QStringLiteral("roll");
    p->rollAttack = isRollAttack;

    QString resolved = direction;
    if (resolved.isEmpty()) {
        resolved = p->facingLeft ? QStringLiteral("left") : QStringLiteral("right");
    }
    CharacterSystem::beginAttack(*p, resolved, isRollAttack ? 8 : 5, isRollAttack ? 65 : 55, isRollAttack ? 520 : 275);
    if (isRollAttack) {
        p->state = QStringLiteral("skill");
        p->animationKey = CharacterSystem::animationForFamilyState(p->animationFamily, p->state);
    }
    CollisionSystem::updateCollisionBoxes(*p, tuning_);

    WorldEvents events;
    CombatSystem::checkAttackHits(*p, characters_, resolvedAttackTokens_, damageCount_, events);
    events.sounds.push_back(isRollAttack ? QStringLiteral("player.attack.2") : QStringLiteral("player.attack.1"));
    emitEvents(events);
    emit worldChanged();
}

void GameWorldViewModel::playerTakeHit(int damage) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    p->hp = std::max(0, p->hp - std::max(0, damage));
    if (p->hp == 0) {
        p->alive = false;
        CharacterSystem::setState(*p, QStringLiteral("dead"), 1, 90);
        emit soundRequested(QStringLiteral("player.dead"));
    } else {
        if (p->state == QStringLiteral("hit")) {
            p->state = QString();
            p->actionDurationMs = 0;
        }
        CharacterSystem::setState(*p, QStringLiteral("hit"), 4, 90, 360);
        emit soundRequested(QStringLiteral("player.hurt"));
    }
    emit worldChanged();
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
    emit worldChanged();
}

void GameWorldViewModel::setChargePressed(bool pressed) {
    chargePressed_ = pressed;
    if (pressed) {
        if (auto* p = player()) {
            p->moveDirection = 0;
            p->velocity.setX(0);
        }
        return;
    }

    chargeProgress_ = 0.0;
    emit chargeProgressChanged();
}

void GameWorldViewModel::playerBurstAttack() {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    p->moveDirection = 0;
    p->velocity.setX(0);
    p->rollAttack = false;
    CharacterSystem::beginAttack(*p, QStringLiteral("burst"), 8, 65, 400);
    p->state = QStringLiteral("skill");
    p->animationKey = CharacterSystem::animationForFamilyState(p->animationFamily, p->state);
    CollisionSystem::updateCollisionBoxes(*p, tuning_);

    WorldEvents events;
    CombatSystem::checkAttackHits(*p, characters_, resolvedAttackTokens_, damageCount_, events);
    events.sounds.push_back(QStringLiteral("player.attack.2"));
    emitEvents(events);
    emit worldChanged();
}

void GameWorldViewModel::initializeWorld() {
    characters_.clear();
    terrain_.clear();
    mapLayers_.clear();
    currentScene_ = SceneId::OriginalFactory;
    updateMapGeometry();

    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - tuning_.actorHeight : terrain_.front().rect.top() - tuning_.actorHeight;
    const qreal playerX = (playableLeft_ + playableRight_ - tuning_.actorWidth) / 2.0;
    const qreal beeX = std::min(playableRight_ - tuning_.actorWidth, playableLeft_ + (playableRight_ - playableLeft_) * 0.72);

    const auto playerCharacter = CharacterFactory::createPlayer(playerX, spawnY, tuning_);
    const auto bee = CharacterFactory::createSmallBee(beeX, spawnY, tuning_);
    characters_.insert(playerCharacter.id, playerCharacter);
    characters_.insert(bee.id, bee);
}

void GameWorldViewModel::updateMapGeometry() {
    mapAspect_ = currentScene_ == SceneId::OriginalFactory ? 1695.0 / 725.0 : 2360.0 / 725.0;
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

    const qreal beeX = std::min(playableRight_ - tuning_.actorWidth, playableLeft_ + (playableRight_ - playableLeft_) * 0.72);
    const auto bee = CharacterFactory::createSmallBee(beeX, spawnY, tuning_);
    characters_.insert(bee.id, bee);

    emit viewportChanged();
    emit worldChanged();
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
    item["animationKey"] = character.animationKey;
    item["frameIndex"] = character.frameIndex;
    item["frameCount"] = character.frameCount;
    item["attackDirection"] = character.attackDirection;
    item["attackVfxKey"] = character.attackVfxKey;
    item["attackVfxSize"] = std::max(character.attackBox.rect.width(), character.attackBox.rect.height());
    item["hurtbox"] = boxToVariant(character.hurtbox);
    item["attackBox"] = boxToVariant(character.attackBox);
    return item;
}

} // namespace skybound
