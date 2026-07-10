#include "viewmodel/GameWorldViewModel.h"

#include <QVariantMap>
#include <algorithm>
#include <cmath>

namespace skybound {

namespace {
constexpr int kFrameMs = 16;

QString animationForState(const QString& state) {
    if (state == QStringLiteral("jump") || state == QStringLiteral("fall")) {
        return QStringLiteral("enemy.jump");
    }
    if (state == QStringLiteral("run")) {
        return QStringLiteral("enemy.run");
    }
    if (state == QStringLiteral("roll")) {
        return QStringLiteral("enemy.roll");
    }
    if (state == QStringLiteral("attack")) {
        return QStringLiteral("enemy.attack");
    }
    if (state == QStringLiteral("hit")) {
        return QStringLiteral("player.hit");
    }
    if (state == QStringLiteral("skill")) {
        return QStringLiteral("enemy.skill");
    }
    if (state == QStringLiteral("dead")) {
        return QStringLiteral("enemy.dead");
    }
    return QStringLiteral("enemy.idle");
}
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

    // Terrain collision boxes (green)
    for (const auto& piece : terrain_) {
        QVariantMap box = rectToVariant(piece.rect);
        box["boxType"] = QStringLiteral("terrain");
        box["kind"] = piece.kind;
        box["active"] = true;
        result.push_back(box);
    }

    // Character hurt/attack boxes (white)
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
    auto* p = const_cast<GameWorldViewModel*>(this)->player();
    return p ? p->hp : 0;
}

int GameWorldViewModel::playerMaxHp() const {
    auto* p = const_cast<GameWorldViewModel*>(this)->player();
    return p ? p->maxHp : 0;
}

int GameWorldViewModel::playerHeartCount() const {
    auto* p = const_cast<GameWorldViewModel*>(this)->player();
    return p ? p->hp / 10 : 0;
}

void GameWorldViewModel::reset() {
    damageCount_ = 0;
    resolvedAttackTokens_.clear();
    initializeWorld();
    emit damageCountChanged();
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
    pendingSceneSwitch_ = false;

    for (auto it = characters_.begin(); it != characters_.end(); ++it) {
        auto& character = it.value();
        if (!character.alive) {
            continue;
        }
        updateCharacterAnimation(character, dt);
        updatePhysics(character, dt);
        updateCollisionBoxes(character);
    }

    if (pendingSceneSwitch_) {
        switchToScene(pendingScene_, pendingEntrySide_);
        pendingSceneSwitch_ = false;
    }

    checkPlayerAttackHits();
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
        setCharacterState(*p, QStringLiteral("run"), 8, 90);
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
        setCharacterState(*p, QStringLiteral("idle"), 1, 90);
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

    p->velocity.setY(jumpVelocity_);
    setCharacterState(*p, QStringLiteral("jump"), 8, 70);
    emit soundRequested(QStringLiteral("player.jump"));
    emit worldChanged();
}

void GameWorldViewModel::playerRoll() {
    auto* p = player();
    if (!p || !p->alive || p->state == QStringLiteral("roll")) {
        return;
    }

    p->velocity.setX((p->facingLeft ? -1 : 1) * moveSpeed_ * 1.8);
    setCharacterState(*p, QStringLiteral("roll"), 8, 55, 360);
    emit soundRequested(QStringLiteral("player.roll"));
    emit worldChanged();
}

void GameWorldViewModel::playerAttack(const QString& direction) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    QString resolved = direction;
    if (resolved.isEmpty()) {
        resolved = p->facingLeft ? QStringLiteral("left") : QStringLiteral("right");
    }
    p->attackDirection = resolved;
    p->attackSerial += 1;
    p->attackBox.active = true;
    setCharacterState(*p, QStringLiteral("attack"), 5, 55, 275);
    updateCollisionBoxes(*p);
    checkPlayerAttackHits();
    emit soundRequested(QStringLiteral("player.attack.1"));
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
        setCharacterState(*p, QStringLiteral("dead"), 1, 90);
        emit soundRequested(QStringLiteral("player.dead"));
    } else {
        if (p->state == QStringLiteral("hit")) {
            p->state = QString();
            p->actionDurationMs = 0;
        }
        setCharacterState(*p, QStringLiteral("hit"), 4, 90, 360);
        emit soundRequested(QStringLiteral("player.hurt"));
    }
    emit worldChanged();
}

void GameWorldViewModel::playerCastSkill(const QString& skillId) {
    auto* p = player();
    if (!p || !p->alive) {
        return;
    }

    p->attackDirection = p->facingLeft ? QStringLiteral("left") : QStringLiteral("right");
    p->attackSerial += 1;
    p->attackBox.active = true;
    setCharacterState(*p, QStringLiteral("skill"), 8, 65, 520);
    emit soundRequested(QStringLiteral("player.attack.2"));
    Q_UNUSED(skillId);
    emit worldChanged();
}

void GameWorldViewModel::initializeWorld() {
    characters_.clear();
    terrain_.clear();
    mapLayers_.clear();
    currentScene_ = SceneId::OriginalFactory;
    updateMapGeometry();

    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - actorHeight_ : terrain_.front().rect.top() - actorHeight_;
    CharacterObject playerCharacter;
    playerCharacter.id = QStringLiteral("player");
    playerCharacter.kind = QStringLiteral("player");
    playerCharacter.hp = 100;
    playerCharacter.maxHp = 100;
    playerCharacter.lives = 3;
    playerCharacter.position = QPointF((playableLeft_ + playableRight_ - actorWidth_) / 2.0, spawnY);
    playerCharacter.facingLeft = true;
    setCharacterState(playerCharacter, QStringLiteral("idle"), 1, 90);
    updateCollisionBoxes(playerCharacter);
    characters_.insert(playerCharacter.id, playerCharacter);

    CharacterObject enemy;
    enemy.id = QStringLiteral("enemy_1");
    enemy.kind = QStringLiteral("enemy");
    enemy.hp = 100;
    enemy.maxHp = 100;
    enemy.lives = 1;
    enemy.position = QPointF(std::min(playableRight_ - actorWidth_, playableLeft_ + (playableRight_ - playableLeft_) * 0.72), spawnY);
    enemy.facingLeft = false;
    setCharacterState(enemy, QStringLiteral("idle"), 1, 90);
    updateCollisionBoxes(enemy);
    characters_.insert(enemy.id, enemy);
}

void GameWorldViewModel::updateMapGeometry() {
    mapAspect_ = currentScene_ == SceneId::OriginalFactory ? 1695.0 / 725.0 : 2360.0 / 725.0;
    mapWidth_ = std::max(viewportWidth_, viewportHeight_ * mapAspect_);
    mapHeight_ = mapWidth_ / mapAspect_;
    mapX_ = (viewportWidth_ - mapWidth_) / 2.0;
    mapY_ = viewportHeight_ - mapHeight_;

    mapLayers_.clear();
    terrain_.clear();
    if (currentScene_ == SceneId::OriginalFactory) {
        buildOriginalScene();
    } else {
        buildBackground2Scene();
    }
}

void GameWorldViewModel::buildOriginalScene() {
    mapLayers_.push_back(MapLayer{QStringLiteral("factory_background"), QStringLiteral("scene.factory.background"), QRectF(mapX_, mapY_, mapWidth_, mapHeight_), 1.0});
    playableLeft_ = 0;
    playableRight_ = viewportWidth_;
    const qreal groundY = mapY_ + mapHeight_ * 0.865;
    terrain_.push_back(TerrainPiece{QStringLiteral("original_ground"), QStringLiteral("ground"), QRectF(playableLeft_, groundY, playableRight_ - playableLeft_, viewportHeight_ - groundY), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("original_platform_left"), QStringLiteral("platform"), QRectF(viewportWidth_ * 0.18, groundY - 145, 230, 24), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("original_platform_right"), QStringLiteral("platform"), QRectF(viewportWidth_ * 0.62, groundY - 105, 260, 24), true});
}

void GameWorldViewModel::buildBackground2Scene() {
    mapLayers_.push_back(MapLayer{QStringLiteral("factory_background2"), QStringLiteral("scene.factory.background2"), QRectF(mapX_, mapY_, mapWidth_, mapHeight_), 1.0});
    playableLeft_ = std::max<qreal>(0, mapX_);
    playableRight_ = std::min<qreal>(viewportWidth_, mapX_ + mapWidth_);

    // Terrain collision boxes
    terrain_.push_back(TerrainPiece{QStringLiteral("bg2_ground"), QStringLiteral("platform"), QRectF(0, 673, 698, 91), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("bg2_step1"), QStringLiteral("platform"), QRectF(423, 628, 89, 46), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("bg2_step2"), QStringLiteral("platform"), QRectF(469, 583, 88, 42), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("bg2_step3"), QStringLiteral("platform"), QRectF(560, 539, 89, 41), true});
    terrain_.push_back(TerrainPiece{QStringLiteral("bg2_top_platform"), QStringLiteral("platform"), QRectF(605, 492, 594, 41), true});
}

void GameWorldViewModel::requestSceneSwitch(SceneId scene, EntrySide entrySide) {
    pendingSceneSwitch_ = true;
    pendingScene_ = scene;
    pendingEntrySide_ = entrySide;
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

    const qreal spawnX = entrySide == EntrySide::Left ? playableLeft_ + 24 : playableRight_ - actorWidth_ - 24;
    const qreal spawnY = terrain_.isEmpty() ? viewportHeight_ - actorHeight_ : terrain_.front().rect.top() - actorHeight_;
    characters_.clear();

    playerSnapshot.id = QStringLiteral("player");
    playerSnapshot.kind = QStringLiteral("player");
    playerSnapshot.position = QPointF(spawnX, spawnY);
    playerSnapshot.velocity = QPointF(0, 0);
    playerSnapshot.moveDirection = 0;
    playerSnapshot.attackBox.active = false;
    setCharacterState(playerSnapshot, QStringLiteral("idle"), 1, 90);
    updateCollisionBoxes(playerSnapshot);
    characters_.insert(playerSnapshot.id, playerSnapshot);

    CharacterObject enemy;
    enemy.id = QStringLiteral("enemy_1");
    enemy.kind = QStringLiteral("enemy");
    enemy.hp = 100;
    enemy.maxHp = 100;
    enemy.lives = 1;
    enemy.position = QPointF(std::min(playableRight_ - actorWidth_, playableLeft_ + (playableRight_ - playableLeft_) * 0.72), spawnY);
    enemy.facingLeft = false;
    setCharacterState(enemy, QStringLiteral("idle"), 1, 90);
    updateCollisionBoxes(enemy);
    characters_.insert(enemy.id, enemy);

    emit viewportChanged();
    emit worldChanged();
}

QRectF GameWorldViewModel::imageRectToWorld(qreal x1, qreal y1, qreal x2, qreal y2, qreal imageWidth, qreal imageHeight) const {
    const qreal left = mapX_ + x1 * mapWidth_ / imageWidth;
    const qreal right = mapX_ + x2 * mapWidth_ / imageWidth;
    const qreal top = mapY_ + (imageHeight - y2) * mapHeight_ / imageHeight;
    const qreal bottom = mapY_ + (imageHeight - y1) * mapHeight_ / imageHeight;
    return QRectF(left, top, right - left, bottom - top);
}

void GameWorldViewModel::updateCharacterAnimation(CharacterObject& character, int deltaMs) {
    character.frameElapsedMs += deltaMs;
    if (character.frameIntervalMs > 0 && character.frameElapsedMs >= character.frameIntervalMs) {
        character.frameElapsedMs = 0;
        character.frameIndex = character.frameCount <= 1 ? 0 : (character.frameIndex + 1) % character.frameCount;
    }

    if (character.actionDurationMs > 0) {
        character.actionElapsedMs += deltaMs;
        if (character.actionElapsedMs >= character.actionDurationMs) {
            character.actionDurationMs = 0;
            character.actionElapsedMs = 0;
            character.attackBox.active = false;
            const bool airborne = character.velocity.y() != 0;
            if (airborne) {
                setCharacterState(character, character.velocity.y() < 0 ? QStringLiteral("jump") : QStringLiteral("fall"), 8, 70);
            } else if (character.moveDirection != 0) {
                setCharacterState(character, QStringLiteral("run"), 8, 90);
            } else {
                setCharacterState(character, QStringLiteral("idle"), 1, 90);
            }
        }
    }
}

void GameWorldViewModel::updatePhysics(CharacterObject& character, int deltaMs) {
    if (character.kind != QStringLiteral("player")) {
        return;
    }

    const qreal stepScale = deltaMs / static_cast<qreal>(kFrameMs);
    if (character.state == QStringLiteral("roll")) {
        character.position.rx() += character.velocity.x() * stepScale;
    } else {
        character.velocity.setX(character.moveDirection * moveSpeed_);
        character.position.rx() += character.velocity.x() * stepScale;
    }

    character.velocity.setY(std::min(maxFallVelocity_, character.velocity.y() + gravity_ * stepScale));
    character.position.ry() += character.velocity.y() * stepScale;

    if (currentScene_ == SceneId::OriginalFactory && character.position.x() > playableRight_ - actorWidth_) {
        requestSceneSwitch(SceneId::Background2Factory, EntrySide::Left);
    } else if (currentScene_ == SceneId::Background2Factory && character.position.x() < playableLeft_) {
        requestSceneSwitch(SceneId::OriginalFactory, EntrySide::Right);
    }
    character.position.rx() = std::max<qreal>(playableLeft_, std::min(playableRight_ - actorWidth_, character.position.x()));

    resolveTerrainCollision(character);

    if (character.state != QStringLiteral("attack") && character.state != QStringLiteral("roll") && character.state != QStringLiteral("hit") && character.state != QStringLiteral("skill")) {
        if (character.velocity.y() < -0.1) {
            setCharacterState(character, QStringLiteral("jump"), 8, 70);
        } else if (character.velocity.y() > 0.1) {
            setCharacterState(character, QStringLiteral("fall"), 8, 70);
        } else if (character.moveDirection != 0) {
            setCharacterState(character, QStringLiteral("run"), 8, 90);
        } else {
            setCharacterState(character, QStringLiteral("idle"), 1, 90);
        }
    }
}

void GameWorldViewModel::updateCollisionBoxes(CharacterObject& character) {
    const bool isEnemy = character.kind == QStringLiteral("enemy");
    const qreal hurtOffsetX = isEnemy ? enemyHurtboxOffsetX_ : playerHurtboxOffsetX_;
    const qreal hurtOffsetY = isEnemy ? enemyHurtboxOffsetY_ : playerHurtboxOffsetY_;
    const qreal hurtWidth = isEnemy ? enemyHurtboxWidth_ : playerHurtboxWidth_;
    const qreal hurtHeight = isEnemy ? enemyHurtboxHeight_ : playerHurtboxHeight_;

    character.hurtbox = CollisionBox{QRectF(character.position.x() + hurtOffsetX, character.position.y() + hurtOffsetY, hurtWidth, hurtHeight), character.alive};

    if (character.kind != QStringLiteral("player") || (character.state != QStringLiteral("attack") && character.state != QStringLiteral("skill"))) {
        character.attackBox = CollisionBox{QRectF(), false};
        return;
    }

    qreal boxWidth = attackBoxWidth_;
    qreal boxHeight = attackBoxHeight_;
    qreal offsetX = attackOffsetLeftX_;
    qreal offsetY = attackOffsetLeftY_;

    if (character.attackDirection == QStringLiteral("right")) {
        offsetX = attackOffsetRightX_;
        offsetY = attackOffsetRightY_;
    } else if (character.attackDirection == QStringLiteral("up")) {
        boxWidth = attackBoxHeight_;
        boxHeight = attackBoxWidth_;
        offsetX = attackOffsetUpX_;
        offsetY = attackOffsetUpY_;
    } else if (character.attackDirection == QStringLiteral("down")) {
        boxWidth = attackBoxHeight_;
        boxHeight = attackBoxWidth_;
        offsetX = attackOffsetDownX_;
        offsetY = attackOffsetDownY_;
    }

    character.attackBox = CollisionBox{QRectF(character.position.x() + offsetX, character.position.y() + offsetY, boxWidth, boxHeight), true};
}

void GameWorldViewModel::resolveTerrainCollision(CharacterObject& character) {
    QRectF body(character.position.x(), character.position.y(), actorWidth_, actorHeight_);
    const qreal centerX = body.center().x();

    for (const auto& piece : terrain_) {
        if (!piece.solid || character.velocity.y() < 0) {
            continue;
        }

        if (piece.kind == QStringLiteral("stairs") && centerX >= piece.rect.left() && centerX <= piece.rect.right()) {
            const qreal t = std::clamp((centerX - piece.rect.left()) / piece.rect.width(), 0.0, 1.0);
            const qreal stairTop = piece.rect.top() + t * piece.rect.height();
            if (body.bottom() >= stairTop - 8 && body.top() < stairTop) {
                character.position.setY(stairTop - actorHeight_);
                character.velocity.setY(0);
                body.moveTop(character.position.y());
            }
            continue;
        }

        // Compute current hurtbox from character position (updateCollisionBoxes hasn't run yet this frame)
        const qreal hOffX = character.kind == QStringLiteral("enemy") ? enemyHurtboxOffsetX_ : playerHurtboxOffsetX_;
        const qreal hOffY = character.kind == QStringLiteral("enemy") ? enemyHurtboxOffsetY_ : playerHurtboxOffsetY_;
        const qreal hW = character.kind == QStringLiteral("enemy") ? enemyHurtboxWidth_ : playerHurtboxWidth_;
        const qreal hH = character.kind == QStringLiteral("enemy") ? enemyHurtboxHeight_ : playerHurtboxHeight_;
        const QRectF hr(character.position.x() + hOffX, character.position.y() + hOffY, hW, hH);
        const qreal previousBottom = hr.bottom() - character.velocity.y();
        if (previousBottom <= piece.rect.top() && hr.bottom() >= piece.rect.top() && hr.right() > piece.rect.left() && hr.left() < piece.rect.right()) {
            character.position.setY(piece.rect.top() - hOffY - hH);
            character.velocity.setY(0);
            body.moveTop(character.position.y());
        }

        // Left edge blocking using HURTBOX (white debug box) instead of full body
        if (piece.kind != QStringLiteral("stairs")) {
            const qreal overlapTop = std::max(hr.top(), piece.rect.top());
            const qreal overlapBottom = std::min(hr.bottom(), piece.rect.bottom());
            const qreal overlapHeight = overlapBottom - overlapTop;
            if (overlapHeight > hr.height() * 0.2) {
                const qreal previousRight = hr.right() - character.velocity.x();
                if (previousRight <= piece.rect.left() && hr.right() > piece.rect.left() && hr.left() < piece.rect.right()) {
                    const qreal pushX = piece.rect.left() - hOffX - hW;
                    character.position.setX(pushX);
                    character.velocity.setX(0);
                }
            }
        }
    }
}

void GameWorldViewModel::setCharacterState(CharacterObject& character, const QString& state, int frameCount, int frameIntervalMs, int durationMs) {
    if (character.state == state && character.frameCount == frameCount && character.actionDurationMs == durationMs) {
        return;
    }
    character.state = state;
    character.animationKey = animationForState(state);
    character.frameIndex = 0;
    character.frameCount = std::max(1, frameCount);
    character.frameIntervalMs = frameIntervalMs;
    character.frameElapsedMs = 0;
    character.actionElapsedMs = 0;
    character.actionDurationMs = durationMs;
}

void GameWorldViewModel::checkPlayerAttackHits() {
    auto* p = player();
    if (!p || !canUseBox(p->attackBox)) {
        return;
    }
    const QString token = QStringLiteral("player-attack-%1").arg(p->attackSerial);
    if (resolvedAttackTokens_.contains(token)) {
        return;
    }

    for (auto it = characters_.begin(); it != characters_.end(); ++it) {
        auto& target = it.value();
        if (target.id == p->id || target.kind != QStringLiteral("enemy") || !target.alive || !canUseBox(target.hurtbox)) {
            continue;
        }
        if (!p->attackBox.rect.intersects(target.hurtbox.rect)) {
            continue;
        }
        resolvedAttackTokens_.insert(token);
        ++damageCount_;
        setCharacterState(target, QStringLiteral("hit"), 1, 90, 180);
        emit damageCountChanged();
        emit soundRequested(QStringLiteral("enemy.hurt.1"));
        break;
    }
}

GameWorldViewModel::CharacterObject* GameWorldViewModel::player() {
    auto it = characters_.find(QStringLiteral("player"));
    return it == characters_.end() ? nullptr : &it.value();
}

const GameWorldViewModel::CharacterObject* GameWorldViewModel::player() const {
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
    item["width"] = actorWidth_;
    item["height"] = actorHeight_;
    item["position"] = pointToVariant(character.position);
    item["velocity"] = pointToVariant(character.velocity);
    item["facingLeft"] = character.facingLeft;
    item["alive"] = character.alive;
    item["state"] = character.state;
    item["animationKey"] = character.animationKey;
    item["frameIndex"] = character.frameIndex;
    item["frameCount"] = character.frameCount;
    item["attackDirection"] = character.attackDirection;
    item["attackVfxSize"] = std::max(character.attackBox.rect.width(), character.attackBox.rect.height());
    item["hurtbox"] = boxToVariant(character.hurtbox);
    item["attackBox"] = boxToVariant(character.attackBox);
    return item;
}

bool GameWorldViewModel::canUseBox(const CollisionBox& box) const {
    return box.active && box.rect.width() > 0 && box.rect.height() > 0;
}

} // namespace skybound
