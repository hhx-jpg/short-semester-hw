#include "service/ResourceManager.h"

namespace skybound {

namespace {
std::string keyOf(const QString& key) {
    return key.toStdString();
}

QStringList numberedKeys(const QString& prefix, int count) {
    QStringList keys;
    for (int index = 1; index <= count; ++index) {
        keys.push_back(QStringLiteral("%1.%2").arg(prefix).arg(index));
    }
    return keys;
}
} // namespace

ResourceManager::ResourceManager(QObject* parent)
    : QObject(parent) {
    registerDefaults();
}

QString ResourceManager::image(const QString& key) const {
    const auto it = images_.find(keyOf(key));
    return it == images_.end() ? QString{} : it->second;
}

QString ResourceManager::audio(const QString& key) const {
    const auto it = audio_.find(keyOf(key));
    return it == audio_.end() ? QString{} : it->second;
}

QString ResourceManager::animationFrame(const QString& animationKey, int frameIndex) const {
    const auto it = animations_.find(keyOf(animationKey));
    if (it == animations_.end() || it->second.isEmpty()) {
        return image(animationKey);
    }

    const int clamped = std::max(0, std::min(frameIndex, static_cast<int>(it->second.size()) - 1));
    return image(it->second.at(clamped));
}

int ResourceManager::animationFrameCount(const QString& animationKey) const {
    const auto it = animations_.find(keyOf(animationKey));
    return it == animations_.end() ? 0 : static_cast<int>(it->second.size());
}

QVariantList ResourceManager::animationFrames(const QString& animationKey) const {
    QVariantList result;
    const auto it = animations_.find(keyOf(animationKey));
    if (it == animations_.end()) {
        return result;
    }

    for (const auto& imageKey : it->second) {
        result.push_back(image(imageKey));
    }
    return result;
}

bool ResourceManager::isSpriteSheetAnimation(const QString& animationKey) const {
    return spriteSheets_.find(keyOf(animationKey)) != spriteSheets_.end();
}

QString ResourceManager::spriteSheetImage(const QString& animationKey) const {
    const auto it = spriteSheets_.find(keyOf(animationKey));
    return it == spriteSheets_.end() ? QString{} : image(it->second.imageKey);
}

int ResourceManager::spriteSheetFrameWidth(const QString& animationKey) const {
    const auto it = spriteSheets_.find(keyOf(animationKey));
    return it == spriteSheets_.end() ? 0 : it->second.frameWidth;
}

int ResourceManager::spriteSheetFrameHeight(const QString& animationKey) const {
    const auto it = spriteSheets_.find(keyOf(animationKey));
    return it == spriteSheets_.end() ? 0 : it->second.frameHeight;
}

int ResourceManager::spriteSheetFrameCount(const QString& animationKey) const {
    const auto it = spriteSheets_.find(keyOf(animationKey));
    return it == spriteSheets_.end() ? 0 : it->second.frameCount;
}

void ResourceManager::registerDefaults() {
    registerImage(QStringLiteral("scene.factory.background"), QStringLiteral("qrc:/resources/background.png"));
    registerImage(QStringLiteral("scene.factory.background2"), QStringLiteral("qrc:/resources/background2.png"));
    registerImage(QStringLiteral("scene.custom.background"), QStringLiteral("qrc:/resources/background3.png"));
    registerImage(QStringLiteral("scene.new_forest.background"), QStringLiteral("qrc:/resources/background_forest2.png"));
    registerImage(QStringLiteral("scene.forest3.background"), QStringLiteral("qrc:/resources/background_forest3.png"));
    registerImage(QStringLiteral("mob.small_bee.fly.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Fly/Fly-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.attack.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Attack/Attack-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.hit.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Hit/Hit-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.vfx.attack.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Hit/hit.png"));
    registerImage(QStringLiteral("mob.small_bee.vfx.attack_left.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Hit/hit_left.png"));
    registerSpriteSheet(QStringLiteral("mob.small_bee.fly"), QStringLiteral("mob.small_bee.fly.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.attack"), QStringLiteral("mob.small_bee.attack.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.hit"), QStringLiteral("mob.small_bee.hit.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.vfx.attack_right"), QStringLiteral("mob.small_bee.vfx.attack.sheet"), 1024, 1024, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.vfx.attack_left"), QStringLiteral("mob.small_bee.vfx.attack_left.sheet"), 1024, 1024, 4);

    // ──────────────────────────────────────────────
    // 蜗牛（Snail）素材注册
    // 三个 sprite sheet：行走(walk)、缩壳(hide)、死亡(dead)
    // 每张 384×32 像素，按 32×32 切分，各 12 帧
    // ──────────────────────────────────────────────
    registerImage(QStringLiteral("mob.snail.walk.sheet"), QStringLiteral("qrc:/resources/Mob/Snail/walk-Sheet.png"));
    registerImage(QStringLiteral("mob.snail.hide.sheet"), QStringLiteral("qrc:/resources/Mob/Snail/Hide-Sheet.png"));
    registerImage(QStringLiteral("mob.snail.dead.sheet"), QStringLiteral("qrc:/resources/Mob/Snail/Dead-Sheet.png"));
    registerSpriteSheet(QStringLiteral("mob.snail.walk"), QStringLiteral("mob.snail.walk.sheet"), 40, 32, 8);
    registerSpriteSheet(QStringLiteral("mob.snail.hide"), QStringLiteral("mob.snail.hide.sheet"), 32, 32, 12);
    registerSpriteSheet(QStringLiteral("mob.snail.dead"), QStringLiteral("mob.snail.dead.sheet"), 32, 32, 12);

    const auto registerPlayerFrames = [this](const QString& animationKey, const QString& folder, int count) {
        for (int index = 1; index <= count; ++index) {
            const QString key = QStringLiteral("%1.%2").arg(animationKey).arg(index);
            registerImage(key, QStringLiteral("qrc:/resources/player/%1/%2.png").arg(folder).arg(index));
        }
        registerAnimation(animationKey, numberedKeys(animationKey, count));
    };

    registerPlayerFrames(QStringLiteral("player.idle"), QStringLiteral("idle"), 6);
    registerPlayerFrames(QStringLiteral("player.run"), QStringLiteral("run"), 8);
    registerPlayerFrames(QStringLiteral("player.jump"), QStringLiteral("jump"), 8);
    registerPlayerFrames(QStringLiteral("player.fall"), QStringLiteral("fall"), 4);
    registerPlayerFrames(QStringLiteral("player.attack"), QStringLiteral("throw_barb"), 8);
    registerPlayerFrames(QStringLiteral("player.hit"), QStringLiteral("hit"), 4);
    registerPlayerFrames(QStringLiteral("player.charge"), QStringLiteral("squat"), 10);
    registerPlayerFrames(QStringLiteral("player.vfx.burst"), QStringLiteral("silk"), 9);
    registerPlayerFrames(QStringLiteral("player.dash"), QStringLiteral("dash_on_floor"), 2);

    registerImage(QStringLiteral("player.dead"), QStringLiteral("qrc:/resources/player/sprite/sprite_sprite.png"));
    registerAnimation(QStringLiteral("player.dead"), QStringList{QStringLiteral("player.dead")});

    registerImage(QStringLiteral("player.vfx.attack.down"), QStringLiteral("qrc:/resources/player/vfx_attack_down.png"));
    registerImage(QStringLiteral("player.vfx.attack.left"), QStringLiteral("qrc:/resources/player/vfx_attack_left.png"));
    registerImage(QStringLiteral("player.vfx.attack.right"), QStringLiteral("qrc:/resources/player/vfx_attack_right.png"));
    registerImage(QStringLiteral("player.vfx.attack.up"), QStringLiteral("qrc:/resources/player/vfx_attack_up.png"));
    registerImage(QStringLiteral("player.vfx.jump"), QStringLiteral("qrc:/resources/player/vfx_jump.png"));
    registerImage(QStringLiteral("player.vfx.land"), QStringLiteral("qrc:/resources/player/vfx_land.png"));
    registerSpriteSheet(QStringLiteral("player.vfx.attack.down"), QStringLiteral("player.vfx.attack.down"), 324, 324, 5);
    registerSpriteSheet(QStringLiteral("player.vfx.attack.left"), QStringLiteral("player.vfx.attack.left"), 324, 324, 5);
    registerSpriteSheet(QStringLiteral("player.vfx.attack.right"), QStringLiteral("player.vfx.attack.right"), 324, 324, 5);
    registerSpriteSheet(QStringLiteral("player.vfx.attack.up"), QStringLiteral("player.vfx.attack.up"), 324, 324, 5);

    registerAudio(QStringLiteral("bgm.factory"), QStringLiteral("qrc:/resources/audio/bgm.mp3"));
    registerAudio(QStringLiteral("player.attack.1"), QStringLiteral("qrc:/resources/audio/player_attack_1.mp3"));
    registerAudio(QStringLiteral("player.attack.2"), QStringLiteral("qrc:/resources/audio/player_attack_2.mp3"));
    registerAudio(QStringLiteral("player.attack.3"), QStringLiteral("qrc:/resources/audio/player_attack_3.mp3"));
    registerAudio(QStringLiteral("player.jump"), QStringLiteral("qrc:/resources/audio/player_jump.mp3"));
    registerAudio(QStringLiteral("player.land"), QStringLiteral("qrc:/resources/audio/player_land.mp3"));
    registerAudio(QStringLiteral("player.roll"), QStringLiteral("qrc:/resources/audio/player_roll.mp3"));
    registerAudio(QStringLiteral("player.hurt"), QStringLiteral("qrc:/resources/audio/player_hurt.mp3"));
    registerAudio(QStringLiteral("player.dead"), QStringLiteral("qrc:/resources/audio/player_dead.mp3"));
    registerAudio(QStringLiteral("player.run"), QStringLiteral("qrc:/resources/audio/player_run.mp3"));
    registerAudio(QStringLiteral("enemy.run"), QStringLiteral("qrc:/resources/audio/enemy_run.mp3"));
    registerAudio(QStringLiteral("enemy.attack"), QStringLiteral("qrc:/resources/audio/enemy_dash.mp3"));
    registerAudio(QStringLiteral("enemy.hurt.1"), QStringLiteral("qrc:/resources/audio/enemy_hurt_1.mp3"));
    registerAudio(QStringLiteral("enemy.hurt.2"), QStringLiteral("qrc:/resources/audio/enemy_hurt_2.mp3"));
    registerAudio(QStringLiteral("enemy.hurt.3"), QStringLiteral("qrc:/resources/audio/enemy_hurt_3.mp3"));
    registerAudio(QStringLiteral("enemy.throw.barbs"), QStringLiteral("qrc:/resources/audio/enemy_throw_barbs.mp3"));
    registerAudio(QStringLiteral("enemy.throw.silk"), QStringLiteral("qrc:/resources/audio/enemy_throw_silk.mp3"));
    registerAudio(QStringLiteral("enemy.throw.sword"), QStringLiteral("qrc:/resources/audio/enemy_throw_sword.mp3"));
    registerAudio(QStringLiteral("barb.break"), QStringLiteral("qrc:/resources/audio/barb_break.mp3"));
    registerAudio(QStringLiteral("bullet.time"), QStringLiteral("qrc:/resources/audio/bullet_time.mp3"));
}

void ResourceManager::registerImage(const QString& key, const QString& url) {
    images_[keyOf(key)] = url;
}

void ResourceManager::registerAudio(const QString& key, const QString& url) {
    audio_[keyOf(key)] = url;
}

void ResourceManager::registerAnimation(const QString& key, const QStringList& imageKeys) {
    animations_[keyOf(key)] = imageKeys;
}

void ResourceManager::registerSpriteSheet(const QString& key, const QString& imageKey, int frameWidth, int frameHeight, int frameCount) {
    spriteSheets_[keyOf(key)] = SpriteSheet{imageKey, frameWidth, frameHeight, frameCount};
}

} // namespace skybound
