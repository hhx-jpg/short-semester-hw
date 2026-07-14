#pragma once

#include "model/GameWorldTypes.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace skybound {

class ResourceManager;

class GameWorldViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList characters READ characters NOTIFY charactersChanged)
    Q_PROPERTY(QVariantList mapLayers READ mapLayers NOTIFY mapLayersChanged)
    Q_PROPERTY(QVariantList terrain READ terrain NOTIFY terrainChanged)
    Q_PROPERTY(QVariantList debugBoxes READ debugBoxes NOTIFY debugBoxesChanged)
    Q_PROPERTY(int damageCount READ damageCount NOTIFY damageCountChanged)
    Q_PROPERTY(QString currentScene READ currentScene NOTIFY currentSceneChanged)
    Q_PROPERTY(qreal mapAspect READ mapAspect NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapWidth READ mapWidth NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapHeight READ mapHeight NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapX READ mapX NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapY READ mapY NOTIFY viewportChanged)
    Q_PROPERTY(int playerHp READ playerHp NOTIFY playerStatsChanged)
    Q_PROPERTY(int playerMaxHp READ playerMaxHp NOTIFY playerStatsChanged)
    Q_PROPERTY(int playerHeartCount READ playerHeartCount NOTIFY playerStatsChanged)
    Q_PROPERTY(qreal chargeProgress READ chargeProgress NOTIFY chargeProgressChanged)

public:
    explicit GameWorldViewModel(const ResourceManager& resources, QObject* parent = nullptr);

    QVariantList characters() const;
    QVariantList mapLayers() const;
    QVariantList terrain() const;
    QVariantList debugBoxes() const;
    int damageCount() const;
    QString currentScene() const;
    qreal mapAspect() const;
    qreal mapWidth() const;
    qreal mapHeight() const;
    qreal mapX() const;
    qreal mapY() const;
    int playerHp() const;
    int playerMaxHp() const;
    int playerHeartCount() const;
    qreal chargeProgress() const;

    Q_INVOKABLE void reset();
    Q_INVOKABLE void setViewport(qreal width, qreal height);
    Q_INVOKABLE void tick(int deltaMs);
    Q_INVOKABLE void playerRun(int direction);
    Q_INVOKABLE void playerStopRun(int direction);
    Q_INVOKABLE void playerJump();
    Q_INVOKABLE void playerRoll();
    Q_INVOKABLE void playerAttack(const QString& direction);
    Q_INVOKABLE void playerTakeHit(int damage);
    Q_INVOKABLE void playerCastSkill(const QString& skillId);
    Q_INVOKABLE void setAimUpPressed(bool pressed);
    Q_INVOKABLE void setAimDownPressed(bool pressed);
    Q_INVOKABLE void setChargePressed(bool pressed);
    Q_INVOKABLE void releaseAttack();
    Q_INVOKABLE void playerBurstAttack();

signals:
    void worldChanged();
    void charactersChanged();
    void mapLayersChanged();
    void terrainChanged();
    void debugBoxesChanged();
    void currentSceneChanged();
    void playerStatsChanged();
    void viewportChanged();
    void damageCountChanged();
    void soundRequested(const QString& key);
    void chargeProgressChanged();

private:
    void initializeWorld();
    void updateMapGeometry();
    void switchToScene(SceneId scene, EntrySide entrySide);
    void applySceneSwitch(const SceneSwitchRequest& sceneSwitch);
    void updateCharge(int deltaMs, WorldEvents& events);
    void applyCombatResult(const CombatResult& result, WorldEvents& events);
    void emitEvents(const WorldEvents& events);
    void notifyWorldDataChanged(bool sceneChanged = false);
    QVariantMap animationPresentation(const QString& key, int frameIndex) const;
    CharacterObject* player();
    const CharacterObject* player() const;
    QVariantMap rectToVariant(const QRectF& rect) const;
    QVariantMap pointToVariant(const QPointF& point) const;
    QVariantMap boxToVariant(const CollisionBox& box) const;
    QVariantMap characterToVariant(const CharacterObject& character) const;

    const ResourceManager& resources_;
    SceneId currentScene_ = SceneId::OriginalFactory;
    WorldTuning tuning_;
    qreal viewportWidth_ = 1200;
    qreal viewportHeight_ = 760;
    qreal mapAspect_ = 1695.0 / 725.0;
    qreal mapWidth_ = 1200;
    qreal mapHeight_ = 369;
    qreal mapX_ = 0;
    qreal mapY_ = 391;
    qreal playableLeft_ = 0;
    qreal playableRight_ = 1200;

    QHash<QString, CharacterObject> characters_;
    QList<TerrainPiece> terrain_;
    QList<MapLayer> mapLayers_;
    QSet<QString> resolvedAttackTokens_;
    int damageCount_ = 0;
    qreal chargeProgress_ = 0.0;
    bool chargePressed_ = false;
    bool aimingUp_ = false;
    bool aimingDown_ = false;
};

} // namespace skybound
