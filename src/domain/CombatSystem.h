#pragma once

#include "model/GameWorldTypes.h"

#include <QHash>
#include <QSet>

namespace skybound {

class CombatSystem {
public:
    static CombatResult checkAttackHits(
        CharacterObject& attacker,
        QHash<QString, CharacterObject>& characters,
        const QList<TerrainPiece>& terrain,
        QSet<QString>& resolvedAttackTokens);

    static CombatResult applyDamageToPlayer(CharacterObject& playerCharacter, int damage, const QPointF& attackerPos);

    // ──────────────────────────────────────────────
    // 蜗牛接触伤害
    // 蜗牛的 hurtbox 与玩家 hurtbox 重叠时，对玩家造成接触伤害。
    // 伤害有冷却（attackCooldownMs），避免每帧重复扣血。
    // ──────────────────────────────────────────────
    static CombatResult checkContactDamage(
        QHash<QString, CharacterObject>& characters);

    // ──────────────────────────────────────────────
    // 踩踏击杀检测
    // 玩家从上方下落时脚底碰到蜗牛头顶 → 击杀蜗牛 + 玩家弹跳。
    // 通过比较玩家上一帧和本帧的底部 Y 坐标来判断"踩上去"的动作。
    // ──────────────────────────────────────────────
    static CombatResult checkStompKill(
        QHash<QString, CharacterObject>& characters);
};

} // namespace skybound
