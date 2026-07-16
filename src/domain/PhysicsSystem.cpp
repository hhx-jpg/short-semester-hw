#include "domain/PhysicsSystem.h"

#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"

#include <algorithm>

namespace skybound {
namespace {
constexpr int kFrameMs = 16;
}

void PhysicsSystem::updateCharacterPhysics(
    CharacterObject& character,
    int deltaMs,
    SceneId currentScene,
    qreal playableLeft,
    qreal playableRight,
    const QList<TerrainPiece>& terrain,
    const WorldTuning& tuning,
    bool chargePressed,
    SceneSwitchRequest& sceneSwitch) {
    const bool isPlayer = character.kind == QStringLiteral("player");
    const qreal stepScale = deltaMs / static_cast<qreal>(kFrameMs);

    if (isPlayer && (character.state == QStringLiteral("charge") || (character.state == QStringLiteral("skill") && character.attackDirection == QStringLiteral("burst")) || chargePressed)) {
        character.moveDirection = 0;
        character.velocity.setX(0);
    } else if (character.state == QStringLiteral("roll")) {
        character.position.rx() += character.velocity.x() * stepScale;
    } else if (isPlayer) {
        character.velocity.setX(character.moveDirection * tuning.moveSpeed);
        character.position.rx() += character.velocity.x() * stepScale;
    } else {
        character.position.rx() += character.velocity.x() * stepScale;
    }

    character.velocity.setY(std::min(tuning.maxFallVelocity, character.velocity.y() + tuning.gravity * stepScale));
    character.position.ry() += character.velocity.y() * stepScale;

    if (isPlayer && currentScene == SceneId::OriginalFactory && character.position.x() > playableRight - tuning.actorWidth) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::Background2Factory, EntrySide::Left};
    } else if (isPlayer && currentScene == SceneId::Background2Factory && character.position.x() < playableLeft) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::OriginalFactory, EntrySide::Right};
    } else if (isPlayer && currentScene == SceneId::OriginalFactory && character.position.x() < playableLeft) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::CustomMap, EntrySide::Right};
    } else if (isPlayer && currentScene == SceneId::CustomMap && character.position.x() > playableRight - tuning.actorWidth) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::OriginalFactory, EntrySide::Left};
    } else if (isPlayer && currentScene == SceneId::CustomMap && character.position.x() < playableLeft) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::NewForestMap, EntrySide::Right};
    } else if (isPlayer && currentScene == SceneId::NewForestMap && character.position.x() > playableRight - tuning.actorWidth) {
        sceneSwitch = SceneSwitchRequest{true, SceneId::CustomMap, EntrySide::Left};
    }
    character.position.rx() = std::max<qreal>(playableLeft, std::min(playableRight - tuning.actorWidth, character.position.x()));

    CollisionSystem::resolveTerrainCollision(character, terrain, tuning);

    if (character.state != QStringLiteral("attack") && character.state != QStringLiteral("roll") && character.state != QStringLiteral("hit") && character.state != QStringLiteral("skill") && character.state != QStringLiteral("charge")) {
        const int movingFrameCount = character.animationFamily == QStringLiteral("small_bee") ? 4 : 8;
        if (character.velocity.y() < -0.1) {
            CharacterSystem::setState(character, QStringLiteral("jump"), movingFrameCount, 70);
        } else if (character.velocity.y() > 0.1) {
            CharacterSystem::setState(character, QStringLiteral("fall"), movingFrameCount, 70);
        } else if (character.moveDirection != 0) {
            CharacterSystem::setState(character, QStringLiteral("run"), movingFrameCount, 90);
        } else {
            CharacterSystem::setState(character, QStringLiteral("idle"), character.animationFamily == QStringLiteral("small_bee") ? 4 : 1, 90);
        }
    }
}

} // namespace skybound
