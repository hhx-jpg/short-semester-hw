#include "domain/WorldProcessor.h"

#include "domain/CharacterSystem.h"
#include "domain/CollisionSystem.h"
#include "domain/CombatSystem.h"
#include "domain/NpcSystem.h"
#include "domain/PhysicsSystem.h"

#include <cmath>

namespace skybound {
namespace {
CharacterObject* player(QHash<QString, CharacterObject>& characters) {
    auto it = characters.find(QStringLiteral("player"));
    return it == characters.end() ? nullptr : &it.value();
}
} // namespace

SceneSwitchRequest WorldProcessor::advanceActors(
    QHash<QString, CharacterObject>& characters,
    int deltaMs,
    SceneId currentScene,
    qreal playableLeft,
    qreal playableRight,
    const QList<TerrainPiece>& terrain,
    const WorldTuning& tuning,
    bool chargePressed,
    WorldEvents& events) {
    SceneSwitchRequest sceneSwitch;
    for (auto it = characters.begin(); it != characters.end(); ++it) {
        auto& character = it.value();
        if (!character.alive) {
            continue;
        }

        if (character.aiControlled) {
            NpcSystem::updateNpc(character, player(characters), deltaMs, tuning, events);
        }

        CharacterSystem::updateAnimation(character, deltaMs);
        const qreal verticalVelocityBeforePhysics = character.velocity.y();
        PhysicsSystem::updateCharacterPhysics(character, deltaMs, currentScene, playableLeft, playableRight, terrain, tuning, chargePressed, sceneSwitch);
        if (character.kind == QStringLiteral("player") && verticalVelocityBeforePhysics > 0.1 && std::abs(character.velocity.y()) < 0.01) {
            events.sounds.push_back(QStringLiteral("player.land"));
        }
        CollisionSystem::updateCollisionBoxes(character, tuning);
    }
    return sceneSwitch;
}

CombatResult WorldProcessor::resolveCombat(
    QHash<QString, CharacterObject>& characters,
    QSet<QString>& resolvedAttackTokens) {
    CombatResult combined;
    for (auto it = characters.begin(); it != characters.end(); ++it) {
        const CombatResult result = CombatSystem::checkAttackHits(it.value(), characters, resolvedAttackTokens);
        combined.damageCountDelta += result.damageCountDelta;
        combined.sounds.append(result.sounds);
        combined.playerStatsChanged = combined.playerStatsChanged || result.playerStatsChanged;
    }

    // ── 蜗牛接触伤害：蜗牛 body 触碰玩家 → 扣血 ──
    const CombatResult contactResult = CombatSystem::checkContactDamage(characters);
    combined.damageCountDelta += contactResult.damageCountDelta;
    combined.sounds.append(contactResult.sounds);
    combined.playerStatsChanged = combined.playerStatsChanged || contactResult.playerStatsChanged;

    // ── 踩踏击杀：玩家下落时踩到蜗牛壳顶 → 击杀 + 弹跳 ──
    const CombatResult stompResult = CombatSystem::checkStompKill(characters);
    combined.damageCountDelta += stompResult.damageCountDelta;
    combined.sounds.append(stompResult.sounds);
    combined.playerStatsChanged = combined.playerStatsChanged || stompResult.playerStatsChanged;

    return combined;
}

} // namespace skybound
