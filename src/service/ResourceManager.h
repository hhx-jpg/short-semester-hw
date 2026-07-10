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

private:
    void registerDefaults();
    void registerImage(const QString& key, const QString& url);
    void registerAudio(const QString& key, const QString& url);
    void registerAnimation(const QString& key, const QStringList& imageKeys);

    std::unordered_map<std::string, QString> images_;
    std::unordered_map<std::string, QString> audio_;
    std::unordered_map<std::string, QStringList> animations_;
};

} // namespace skybound
