#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QVariantMap>

namespace skybound {

class ResourceManager;
struct CharacterObject;

class CharacterListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        KindRole,
        XRole,
        YRole,
        WidthRole,
        HeightRole,
        FacingLeftRole,
        AliveRole,
        StateRole,
        SpriteSourceRole,
        SpriteIsSheetRole,
        SpriteFrameCountRole,
        SpriteFrameIndexRole,
        SpriteFrameWidthRole,
        SpriteFrameHeightRole,
        FrameIndexRole,
        FrameCountRole,
        AttackDirectionRole,
        RollAttackRole,
        VfxSourceRole,
        VfxIsSheetRole,
        VfxFrameCountRole,
        VfxFrameIndexRole,
        VfxFrameWidthRole,
        VfxFrameHeightRole,
        AttackVfxSizeRole,
        AttackBoxXRole,
        AttackBoxYRole,
        AttackBoxWidthRole,
        AttackBoxHeightRole,
    };

    explicit CharacterListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// 与角色哈希表同步，只对实际变化发射 dataChanged / beginRemoveRows / beginInsertRows
    void syncFromCharacters(
        const QHash<QString, CharacterObject>& characters,
        const ResourceManager& resources);

    void clear();

private:
    struct Item {
        QString id;
        QVariantMap data;
    };
    QList<Item> items_;

    QVariantMap buildFromCharacter(const CharacterObject& character, const ResourceManager& resources) const;
};

} // namespace skybound
