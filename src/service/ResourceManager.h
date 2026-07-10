#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <unordered_map>

namespace skybound {

class ResourceManager : public QObject {
    Q_OBJECT

public:
    explicit ResourceManager(QObject* parent = nullptr);

    Q_INVOKABLE QString image(const QString& key) const;
    Q_INVOKABLE QString audio(const QString& key) const;
    Q_INVOKABLE QString animationFrame(const QString& animationKey, int frameIndex) const;
    Q_INVOKABLE int animationFrameCount(const QString& animationKey) const;
    Q_INVOKABLE QVariantList animationFrames(const QString& animationKey) const;
    Q_INVOKABLE bool isSpriteSheetAnimation(const QString& animationKey) const;
    Q_INVOKABLE QString spriteSheetImage(const QString& animationKey) const;
    Q_INVOKABLE int spriteSheetFrameWidth(const QString& animationKey) const;
    Q_INVOKABLE int spriteSheetFrameHeight(const QString& animationKey) const;
    Q_INVOKABLE int spriteSheetFrameCount(const QString& animationKey) const;

private:
    struct SpriteSheet {
        QString imageKey;
        int frameWidth = 0;
        int frameHeight = 0;
        int frameCount = 1;
    };

    void registerDefaults();
    void registerImage(const QString& key, const QString& url);
    void registerAudio(const QString& key, const QString& url);
    void registerAnimation(const QString& key, const QStringList& imageKeys);
    void registerSpriteSheet(const QString& key, const QString& imageKey, int frameWidth, int frameHeight, int frameCount);

    std::unordered_map<std::string, QString> images_;
    std::unordered_map<std::string, QString> audio_;
    std::unordered_map<std::string, QStringList> animations_;
    std::unordered_map<std::string, SpriteSheet> spriteSheets_;
};

} // namespace skybound
