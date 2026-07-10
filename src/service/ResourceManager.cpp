#include "service/ResourceManager.h"

namespace skybound {

namespace {
std::string keyOf(const QString& key) {
    return key.toStdString();
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
    registerImage(QStringLiteral("mob.small_bee.fly.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Fly/Fly-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.attack.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Attack/Attack-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.hit.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Hit/Hit-Sheet.png"));
    registerImage(QStringLiteral("mob.small_bee.vfx.attack.sheet"), QStringLiteral("qrc:/resources/Mob/Small Bee/Hit/hit.png"));
    registerSpriteSheet(QStringLiteral("mob.small_bee.fly"), QStringLiteral("mob.small_bee.fly.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.attack"), QStringLiteral("mob.small_bee.attack.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.hit"), QStringLiteral("mob.small_bee.hit.sheet"), 64, 64, 4);
    registerSpriteSheet(QStringLiteral("mob.small_bee.vfx.attack"), QStringLiteral("mob.small_bee.vfx.attack.sheet"), 1024, 1024, 4);

    for (int i = 1; i <= 8; ++i) {
        registerImage(QStringLiteral("enemy.run.%1").arg(i), QStringLiteral("qrc:/resources/enemy/run/%1.png").arg(i));
        registerImage(QStringLiteral("enemy.jump.%1").arg(i), QStringLiteral("qrc:/resources/enemy/jump/%1.png").arg(i));
    }

    registerImage(QStringLiteral("player.idle"), QStringLiteral("qrc:/resources/player/idle.png"));
    registerImage(QStringLiteral("player.run"), QStringLiteral("qrc:/resources/player/run.png"));
    registerImage(QStringLiteral("player.jump"), QStringLiteral("qrc:/resources/player/jump.png"));
    registerImage(QStringLiteral("player.fall"), QStringLiteral("qrc:/resources/player/fall.png"));
    registerImage(QStringLiteral("player.attack"), QStringLiteral("qrc:/resources/player/attack.png"));
    registerImage(QStringLiteral("player.roll"), QStringLiteral("qrc:/resources/player/roll.png"));
    registerImage(QStringLiteral("player.dead"), QStringLiteral("qrc:/resources/player/dead.png"));
    for (int i = 1; i <= 4; ++i) {
        registerImage(QStringLiteral("player.hit.%1").arg(i), QStringLiteral("qrc:/resources/player/hit/%1.png").arg(i));
    }

    registerImage(QStringLiteral("player.vfx.attack.down"), QStringLiteral("qrc:/resources/player/vfx_attack_down.png"));
    registerImage(QStringLiteral("player.vfx.attack.left"), QStringLiteral("qrc:/resources/player/vfx_attack_left.png"));
    registerImage(QStringLiteral("player.vfx.attack.right"), QStringLiteral("qrc:/resources/player/vfx_attack_right.png"));
    registerImage(QStringLiteral("player.vfx.attack.up"), QStringLiteral("qrc:/resources/player/vfx_attack_up.png"));
    registerImage(QStringLiteral("player.vfx.jump"), QStringLiteral("qrc:/resources/player/vfx_jump.png"));
    registerImage(QStringLiteral("player.vfx.land"), QStringLiteral("qrc:/resources/player/vfx_land.png"));

    QStringList runFrames;
    QStringList jumpFrames;
    for (int i = 1; i <= 8; ++i) {
        runFrames.push_back(QStringLiteral("enemy.run.%1").arg(i));
        jumpFrames.push_back(QStringLiteral("enemy.jump.%1").arg(i));
    }
    registerAnimation(QStringLiteral("enemy.run"), runFrames);
    registerAnimation(QStringLiteral("enemy.jump"), jumpFrames);
    registerAnimation(QStringLiteral("enemy.idle"), QStringList{QStringLiteral("enemy.run.1")});
    registerAnimation(QStringLiteral("enemy.fall"), jumpFrames);
    registerAnimation(QStringLiteral("enemy.roll"), runFrames);
    registerAnimation(QStringLiteral("enemy.attack"), runFrames);
    registerAnimation(QStringLiteral("enemy.hit"), QStringList{QStringLiteral("enemy.run.1")});
    registerAnimation(QStringLiteral("enemy.skill"), runFrames);
    QStringList hitFrames;
    for (int i = 1; i <= 4; ++i) {
        hitFrames.push_back(QStringLiteral("player.hit.%1").arg(i));
    }
    registerAnimation(QStringLiteral("player.hit"), hitFrames);

    registerAnimation(QStringLiteral("enemy.dead"), QStringList{QStringLiteral("enemy.run.1")});

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
