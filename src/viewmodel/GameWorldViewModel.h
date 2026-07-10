#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace skybound {

class GameWorldViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList characters READ characters NOTIFY worldChanged)
    Q_PROPERTY(QVariantList mapLayers READ mapLayers NOTIFY worldChanged)
    Q_PROPERTY(QVariantList terrain READ terrain NOTIFY worldChanged)
    Q_PROPERTY(QVariantList debugBoxes READ debugBoxes NOTIFY worldChanged)
    Q_PROPERTY(int damageCount READ damageCount NOTIFY damageCountChanged)
    Q_PROPERTY(QString currentScene READ currentScene NOTIFY worldChanged)
    Q_PROPERTY(qreal mapAspect READ mapAspect NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapWidth READ mapWidth NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapHeight READ mapHeight NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapX READ mapX NOTIFY viewportChanged)
    Q_PROPERTY(qreal mapY READ mapY NOTIFY viewportChanged)

public:
    explicit GameWorldViewModel(QObject* parent = nullptr);

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

signals:
    void worldChanged();
    void viewportChanged();
    void damageCountChanged();
    void soundRequested(const QString& key);

private:
    enum class SceneId {
        OriginalFactory,
        Background2Factory,
    };

    enum class EntrySide {
        Left,
        Right,
    };

    struct CollisionBox {
        QRectF rect;
        bool active = true;
    };

    struct CharacterObject {
        QString id;
        QString kind;
        int hp = 100;
        int maxHp = 100;
        int lives = 1;
        QPointF position;
        QPointF velocity;
        bool facingLeft = true;
        bool alive = true;
        QString state = QStringLiteral("idle");
        QString animationKey = QStringLiteral("enemy.idle");
        int frameIndex = 0;
        int frameCount = 1;
        int frameIntervalMs = 90;
        int frameElapsedMs = 0;
        int actionElapsedMs = 0;
        int actionDurationMs = 0;
        QString attackDirection = QStringLiteral("left");
        int attackSerial = 0;
        int moveDirection = 0;
        CollisionBox hurtbox;
        CollisionBox attackBox;
    };

    struct TerrainPiece {
        QString id;
        QString kind;
        QRectF rect;
        bool solid = true;
    };

    struct MapLayer {
        QString id;
        QString imageKey;
        QRectF rect;
        qreal opacity = 1.0;
    };

    void initializeWorld();
    void updateMapGeometry();
    void buildOriginalScene();
    void buildBackground2Scene();
    void switchToScene(SceneId scene, EntrySide entrySide);
    QRectF imageRectToWorld(qreal x1, qreal y1, qreal x2, qreal y2, qreal imageWidth, qreal imageHeight) const;
    void updateCharacterAnimation(CharacterObject& character, int deltaMs);
    void updatePhysics(CharacterObject& character, int deltaMs);
    void updateCollisionBoxes(CharacterObject& character);
    void resolveTerrainCollision(CharacterObject& character);
    void setCharacterState(CharacterObject& character, const QString& state, int frameCount, int frameIntervalMs, int durationMs = 0);
    void checkPlayerAttackHits();
    CharacterObject* player();
    const CharacterObject* player() const;
    QVariantMap rectToVariant(const QRectF& rect) const;
    QVariantMap pointToVariant(const QPointF& point) const;
    QVariantMap boxToVariant(const CollisionBox& box) const;
    QVariantMap characterToVariant(const CharacterObject& character) const;
    bool canUseBox(const CollisionBox& box) const;

    SceneId currentScene_ = SceneId::OriginalFactory;
    qreal viewportWidth_ = 1200;
    qreal viewportHeight_ = 760;
    qreal mapAspect_ = 1695.0 / 725.0;
    qreal mapWidth_ = 1200;
    qreal mapHeight_ = 369;
    qreal mapX_ = 0;
    qreal mapY_ = 391;
    qreal playableLeft_ = 0;
    qreal playableRight_ = 1200;
    qreal gravity_ = 0.62;
    qreal jumpVelocity_ = -13.0;
    qreal maxFallVelocity_ = 16.0;
    qreal moveSpeed_ = 4.6;
    qreal actorWidth_ = 90;
    qreal actorHeight_ = 90;
    qreal playerHurtboxOffsetX_ = 23;
    qreal playerHurtboxOffsetY_ = 12;
    qreal playerHurtboxWidth_ = 44;
    qreal playerHurtboxHeight_ = 72;
    qreal enemyHurtboxOffsetX_ = 23;
    qreal enemyHurtboxOffsetY_ = 12;
    qreal enemyHurtboxWidth_ = 44;
    qreal enemyHurtboxHeight_ = 72;
    qreal attackBoxWidth_ = 138;
    qreal attackBoxHeight_ = 60;
    qreal attackOffsetLeftX_ = -94;
    qreal attackOffsetLeftY_ = 18;
    qreal attackOffsetRightX_ = 46;
    qreal attackOffsetRightY_ = 18;
    qreal attackOffsetUpX_ = 15;
    qreal attackOffsetUpY_ = -96;
    qreal attackOffsetDownX_ = 15;
    qreal attackOffsetDownY_ = 54;

    QHash<QString, CharacterObject> characters_;
    QList<TerrainPiece> terrain_;
    QList<MapLayer> mapLayers_;
    QSet<QString> resolvedAttackTokens_;
    int damageCount_ = 0;
};

} // namespace skybound
