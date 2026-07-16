#include "domain/CollisionSystem.h"

#include <algorithm>

namespace skybound {

void CollisionSystem::updateCollisionBoxes(CharacterObject& character, const WorldTuning& tuning) {
    const bool isEnemy = character.kind == QStringLiteral("enemy");
    qreal hurtOffsetX, hurtOffsetY, hurtWidth, hurtHeight;

    // ──────────────────────────────────────────────
    // 蜗牛 hurtbox 特殊处理
    // 蜗牛渲染尺寸 32×32，与精灵表帧尺寸一致。
    // hurtbox 固定为 42×32，比渲染尺寸稍大以保证碰撞判定。
    // ──────────────────────────────────────────────
    if (character.animationFamily == QStringLiteral("snail")) {
        hurtOffsetX = -5;                    // 向左扩展 5px，让 hurtbox 覆盖渲染区域
        hurtOffsetY = 0;                     // 顶部对齐
        hurtWidth = 42;                      // 固定宽度 42px（覆盖实际身体范围）
        hurtHeight = character.charHeight;   // 高度 = 32
    } else {
        hurtOffsetX = isEnemy ? tuning.enemyHurtboxOffsetX : tuning.playerHurtboxOffsetX;
        hurtOffsetY = isEnemy ? tuning.enemyHurtboxOffsetY : tuning.playerHurtboxOffsetY;
        hurtWidth = isEnemy ? tuning.enemyHurtboxWidth : tuning.playerHurtboxWidth;
        hurtHeight = isEnemy ? tuning.enemyHurtboxHeight : tuning.playerHurtboxHeight;
    }

    character.hurtbox = CollisionBox{QRectF(character.position.x() + hurtOffsetX, character.position.y() + hurtOffsetY, hurtWidth, hurtHeight), character.alive};

    if (character.state != QStringLiteral("attack") && character.state != QStringLiteral("skill")) {
        character.attackBox = CollisionBox{QRectF(), false};
        return;
    }

    qreal boxWidth = tuning.attackBoxWidth;
    qreal boxHeight = tuning.attackBoxHeight;
    qreal offsetX = tuning.attackOffsetLeftX;
    qreal offsetY = tuning.attackOffsetLeftY;

    if (character.attackDirection == QStringLiteral("burst")) {
        offsetX = (tuning.actorWidth - tuning.burstBoxWidth) / 2;
        offsetY = (tuning.actorHeight - tuning.burstBoxHeight) / 2;
        boxWidth = tuning.burstBoxWidth;
        boxHeight = tuning.burstBoxHeight;
    } else if (character.rollAttack) {
        if (character.attackDirection == QStringLiteral("right")) {
            offsetX = tuning.attackOffsetRightX;
            offsetY = tuning.attackOffsetRightY + (tuning.attackBoxHeight - tuning.rollAttackBoxHeight) / 2;
            boxWidth = tuning.rollAttackBoxWidth;
            boxHeight = tuning.rollAttackBoxHeight;
        } else if (character.attackDirection == QStringLiteral("up")) {
            boxWidth = tuning.rollAttackBoxHeight;
            boxHeight = tuning.rollAttackBoxWidth;
            offsetX = tuning.attackOffsetUpX + (tuning.attackBoxHeight - tuning.rollAttackBoxHeight) / 2;
            offsetY = tuning.attackOffsetUpY - (tuning.rollAttackBoxWidth - tuning.attackBoxWidth);
        } else if (character.attackDirection == QStringLiteral("down")) {
            boxWidth = tuning.rollAttackBoxHeight;
            boxHeight = tuning.rollAttackBoxWidth;
            offsetX = tuning.attackOffsetDownX + (tuning.attackBoxHeight - tuning.rollAttackBoxHeight) / 2;
            offsetY = tuning.attackOffsetDownY;
        } else {
            offsetX = tuning.attackOffsetLeftX - (tuning.rollAttackBoxWidth - tuning.attackBoxWidth);
            offsetY = tuning.attackOffsetLeftY + (tuning.attackBoxHeight - tuning.rollAttackBoxHeight) / 2;
            boxWidth = tuning.rollAttackBoxWidth;
            boxHeight = tuning.rollAttackBoxHeight;
        }
    } else {
        if (character.attackDirection == QStringLiteral("right")) {
            offsetX = tuning.attackOffsetRightX;
            offsetY = tuning.attackOffsetRightY;
        } else if (character.attackDirection == QStringLiteral("up")) {
            boxWidth = tuning.attackBoxHeight;
            boxHeight = tuning.attackBoxWidth;
            offsetX = tuning.attackOffsetUpX;
            offsetY = tuning.attackOffsetUpY;
        } else if (character.attackDirection == QStringLiteral("down")) {
            boxWidth = tuning.attackBoxHeight;
            boxHeight = tuning.attackBoxWidth;
            offsetX = tuning.attackOffsetDownX;
            offsetY = tuning.attackOffsetDownY;
        }
    }

    character.attackBox = CollisionBox{QRectF(character.position.x() + offsetX, character.position.y() + offsetY, boxWidth, boxHeight), true};
}

void CollisionSystem::resolveTerrainCollision(CharacterObject& character, const QList<TerrainPiece>& terrain, const WorldTuning& tuning) {
    QRectF body(character.position.x(), character.position.y(), tuning.actorWidth, tuning.actorHeight);
    const qreal centerX = body.center().x();

    for (const auto& piece : terrain) {
        if (!piece.solid || character.velocity.y() < 0) {
            continue;
        }

        if (piece.kind == QStringLiteral("stairs") && centerX >= piece.rect.left() && centerX <= piece.rect.right()) {
            const qreal t = std::clamp((centerX - piece.rect.left()) / piece.rect.width(), 0.0, 1.0);
            const qreal stairTop = piece.rect.top() + t * piece.rect.height();
            if (body.bottom() >= stairTop - 8 && body.top() < stairTop) {
                character.position.setY(stairTop - tuning.actorHeight);
                character.velocity.setY(0);
                body.moveTop(character.position.y());
            }
            continue;
        }

        qreal hOffX, hOffY, hW, hH;
        // 蜗牛体型小，地形碰撞也使用固定较大体积，防止与地面/平台错误对齐
        if (character.animationFamily == QStringLiteral("snail")) {
            hOffX = -5;
            hOffY = 0;
            hW = 42;
            hH = character.charHeight;
        } else {
            hOffX = character.kind == QStringLiteral("enemy") ? tuning.enemyHurtboxOffsetX : tuning.playerHurtboxOffsetX;
            hOffY = character.kind == QStringLiteral("enemy") ? tuning.enemyHurtboxOffsetY : tuning.playerHurtboxOffsetY;
            hW = character.kind == QStringLiteral("enemy") ? tuning.enemyHurtboxWidth : tuning.playerHurtboxWidth;
            hH = character.kind == QStringLiteral("enemy") ? tuning.enemyHurtboxHeight : tuning.playerHurtboxHeight;
        }
        const QRectF hr(character.position.x() + hOffX, character.position.y() + hOffY, hW, hH);
        const qreal previousBottom = hr.bottom() - character.velocity.y();
        if (previousBottom <= piece.rect.top() && hr.bottom() >= piece.rect.top() && hr.right() > piece.rect.left() && hr.left() < piece.rect.right()) {
            character.position.setY(piece.rect.top() - hOffY - hH);
            character.velocity.setY(0);
            body.moveTop(character.position.y());
        }

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

bool CollisionSystem::canUseBox(const CollisionBox& box) {
    return box.active && box.rect.width() > 0 && box.rect.height() > 0;
}

} // namespace skybound
