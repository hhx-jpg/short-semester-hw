#include "domain/CharacterSystem.h"

#include <algorithm>

namespace skybound {

QString CharacterSystem::animationForFamilyState(const QString& family, const QString& state) {
    if (family == QStringLiteral("small_bee")) {
        if (state == QStringLiteral("attack")) {
            return QStringLiteral("mob.small_bee.attack");
        }
        if (state == QStringLiteral("hit")) {
            return QStringLiteral("mob.small_bee.hit");
        }
        return QStringLiteral("mob.small_bee.fly");
    }

    // ──────────────────────────────────────────────
    // 蜗牛（snail）动画族：
    //   - idle / 默认状态 → 行走动画（walk）
    //   - hit / hide      → 缩壳动画（hide）：被攻击或主动缩入壳中
    //   - dead            → 死亡动画（dead）
    // ──────────────────────────────────────────────
    if (family == QStringLiteral("snail")) {
        if (state == QStringLiteral("hit") || state == QStringLiteral("hide")) {
            return QStringLiteral("mob.snail.hide");
        }
        if (state == QStringLiteral("dead")) {
            return QStringLiteral("mob.snail.dead");
        }
        return QStringLiteral("mob.snail.walk");
    }

    if (family == QStringLiteral("player")) {
        if (state == QStringLiteral("jump")) {
            return QStringLiteral("player.jump");
        }
        if (state == QStringLiteral("fall")) {
            return QStringLiteral("player.fall");
        }
        if (state == QStringLiteral("run")) {
            return QStringLiteral("player.run");
        }
        if (state == QStringLiteral("roll")) {
            return QStringLiteral("player.dash");
        }
        if (state == QStringLiteral("attack")) {
            return QStringLiteral("player.attack");
        }
        if (state == QStringLiteral("hit")) {
            return QStringLiteral("player.hit");
        }
        if (state == QStringLiteral("charge") || state == QStringLiteral("skill")) {
            return QStringLiteral("player.charge");
        }
        if (state == QStringLiteral("dead")) {
            return QStringLiteral("player.dead");
        }
        return QStringLiteral("player.idle");
    }

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

void CharacterSystem::setState(CharacterObject& character, const QString& state, int frameCount, int frameIntervalMs, int durationMs) {
    if (character.state == state && character.frameCount == frameCount && character.actionDurationMs == durationMs) {
        return;
    }
    character.state = state;
    character.animationKey = animationForFamilyState(character.animationFamily, state);
    character.frameIndex = 0;
    character.frameCount = std::max(1, frameCount);
    character.frameIntervalMs = frameIntervalMs;
    character.frameElapsedMs = 0;
    character.actionElapsedMs = 0;
    character.actionDurationMs = durationMs;
}

void CharacterSystem::updateAnimation(CharacterObject& character, int deltaMs) {
    character.frameElapsedMs += deltaMs;
    if (character.frameIntervalMs > 0 && character.frameElapsedMs >= character.frameIntervalMs) {
        character.frameElapsedMs = 0;
        character.frameIndex = character.frameCount <= 1 ? 0 : (character.frameIndex + 1) % character.frameCount;
    }

    if (character.actionDurationMs <= 0) {
        return;
    }

    character.actionElapsedMs += deltaMs;
    if (character.actionElapsedMs < character.actionDurationMs) {
        return;
    }

    character.actionDurationMs = 0;
    character.actionElapsedMs = 0;
    character.attackBox.active = false;
    character.rollAttack = false;
    if (character.kind == QStringLiteral("player")) {
        character.attackVfxKey.clear();
    }

    const bool airborne = character.velocity.y() != 0;
    const int movingFrameCount = character.animationFamily == QStringLiteral("small_bee") ? 4 : 8;
    if (airborne) {
        setState(character, character.velocity.y() < 0 ? QStringLiteral("jump") : QStringLiteral("fall"), movingFrameCount, 70);
    } else if (character.moveDirection != 0) {
        setState(character, QStringLiteral("run"), movingFrameCount, 90);
    } else {
        setState(character, QStringLiteral("idle"), character.animationFamily == QStringLiteral("small_bee") ? 4 : 1, 90);
    }
}

void CharacterSystem::beginAttack(CharacterObject& attacker, const QString& direction, int frameCount, int frameIntervalMs, int durationMs) {
    QString resolved = direction;
    if (resolved.isEmpty()) {
        resolved = attacker.facingLeft ? QStringLiteral("left") : QStringLiteral("right");
    }

    attacker.attackDirection = resolved;
    attacker.attackSerial += 1;
    attacker.attackBox.active = true;
    setState(attacker, QStringLiteral("attack"), frameCount, frameIntervalMs, durationMs);
}

} // namespace skybound
