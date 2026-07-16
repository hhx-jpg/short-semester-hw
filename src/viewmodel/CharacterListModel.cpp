#include "viewmodel/CharacterListModel.h"

#include "model/GameWorldTypes.h"
#include "service/ResourceManager.h"

#include <algorithm>

namespace skybound {

CharacterListModel::CharacterListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int CharacterListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return items_.size();
}

QVariant CharacterListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size())
        return {};
    const auto& item = items_[index.row()];
    switch (role) {
    case IdRole:               return item.data.value(QStringLiteral("id"));
    case KindRole:             return item.data.value(QStringLiteral("kind"));
    case XRole:                return item.data.value(QStringLiteral("x"));
    case YRole:                return item.data.value(QStringLiteral("y"));
    case WidthRole:            return item.data.value(QStringLiteral("width"));
    case HeightRole:           return item.data.value(QStringLiteral("height"));
    case FacingLeftRole:       return item.data.value(QStringLiteral("facingLeft"));
    case AliveRole:            return item.data.value(QStringLiteral("alive"));
    case StateRole:            return item.data.value(QStringLiteral("state"));
    case SpriteSourceRole:     return item.data.value(QStringLiteral("spriteSource"));
    case SpriteIsSheetRole:    return item.data.value(QStringLiteral("spriteIsSheet"));
    case SpriteFrameCountRole: return item.data.value(QStringLiteral("spriteFrameCount"));
    case SpriteFrameIndexRole: return item.data.value(QStringLiteral("spriteFrameIndex"));
    case SpriteFrameWidthRole: return item.data.value(QStringLiteral("spriteFrameWidth"));
    case SpriteFrameHeightRole:return item.data.value(QStringLiteral("spriteFrameHeight"));
    case FrameIndexRole:       return item.data.value(QStringLiteral("frameIndex"));
    case FrameCountRole:       return item.data.value(QStringLiteral("frameCount"));
    case AttackDirectionRole:  return item.data.value(QStringLiteral("attackDirection"));
    case RollAttackRole:       return item.data.value(QStringLiteral("rollAttack"));
    case VfxSourceRole:        return item.data.value(QStringLiteral("vfxSource"));
    case VfxIsSheetRole:       return item.data.value(QStringLiteral("vfxIsSheet"));
    case VfxFrameCountRole:    return item.data.value(QStringLiteral("vfxFrameCount"));
    case VfxFrameIndexRole:    return item.data.value(QStringLiteral("vfxFrameIndex"));
    case VfxFrameWidthRole:    return item.data.value(QStringLiteral("vfxFrameWidth"));
    case VfxFrameHeightRole:   return item.data.value(QStringLiteral("vfxFrameHeight"));
    case AttackVfxSizeRole:    return item.data.value(QStringLiteral("attackVfxSize"));
    case AttackBoxXRole:       return item.data.value(QStringLiteral("attackBoxX"));
    case AttackBoxYRole:       return item.data.value(QStringLiteral("attackBoxY"));
    case AttackBoxWidthRole:   return item.data.value(QStringLiteral("attackBoxWidth"));
    case AttackBoxHeightRole:  return item.data.value(QStringLiteral("attackBoxHeight"));
    default:                   return {};
    }
}

QHash<int, QByteArray> CharacterListModel::roleNames() const {
    return {
        {IdRole,               "id"},
        {KindRole,             "kind"},
        {XRole,                "x"},
        {YRole,                "y"},
        {WidthRole,            "width"},
        {HeightRole,           "height"},
        {FacingLeftRole,       "facingLeft"},
        {AliveRole,            "alive"},
        {StateRole,            "state"},
        {SpriteSourceRole,     "spriteSource"},
        {SpriteIsSheetRole,    "spriteIsSheet"},
        {SpriteFrameCountRole, "spriteFrameCount"},
        {SpriteFrameIndexRole, "spriteFrameIndex"},
        {SpriteFrameWidthRole, "spriteFrameWidth"},
        {SpriteFrameHeightRole,"spriteFrameHeight"},
        {FrameIndexRole,       "frameIndex"},
        {FrameCountRole,       "frameCount"},
        {AttackDirectionRole,  "attackDirection"},
        {RollAttackRole,       "rollAttack"},
        {VfxSourceRole,        "vfxSource"},
        {VfxIsSheetRole,       "vfxIsSheet"},
        {VfxFrameCountRole,    "vfxFrameCount"},
        {VfxFrameIndexRole,    "vfxFrameIndex"},
        {VfxFrameWidthRole,    "vfxFrameWidth"},
        {VfxFrameHeightRole,   "vfxFrameHeight"},
        {AttackVfxSizeRole,    "attackVfxSize"},
        {AttackBoxXRole,       "attackBoxX"},
        {AttackBoxYRole,       "attackBoxY"},
        {AttackBoxWidthRole,   "attackBoxWidth"},
        {AttackBoxHeightRole,  "attackBoxHeight"},
    };
}

void CharacterListModel::syncFromCharacters(
    const QHash<QString, CharacterObject>& characters,
    const ResourceManager& resources) {

    // 1. 构建新 id → data 索引
    QHash<QString, QVariantMap> newData;
    for (auto it = characters.constBegin(); it != characters.constEnd(); ++it) {
        newData.insert(it.key(), buildFromCharacter(it.value(), resources));
    }

    // 2. 找出需要删除的角色（在新哈希中不存在）
    QList<int> toRemove;
    for (int i = 0; i < items_.size(); ++i) {
        if (!newData.contains(items_[i].id)) {
            toRemove.prepend(i);  // 从后往前删
        }
    }

    // 3. 执行删除
    for (int i : toRemove) {
        beginRemoveRows({}, i, i);
        items_.removeAt(i);
        endRemoveRows();
    }

    // 4. 找出需要新增或更新的角色
    for (auto it = newData.constBegin(); it != newData.constEnd(); ++it) {
        const QString& id = it.key();
        const QVariantMap& newMap = it.value();

        // 查找现有位置
        int existingIdx = -1;
        for (int i = 0; i < items_.size(); ++i) {
            if (items_[i].id == id) {
                existingIdx = i;
                break;
            }
        }

        if (existingIdx < 0) {
            // 新增
            beginInsertRows({}, items_.size(), items_.size());
            items_.push_back({id, newMap});
            endInsertRows();
        } else {
            // 已存在：通过 dataChanged 更新
            items_[existingIdx].data = newMap;
            emit dataChanged(index(existingIdx, 0), index(existingIdx, 0));
        }
    }
}

void CharacterListModel::clear() {
    if (!items_.isEmpty()) {
        beginResetModel();
        items_.clear();
        endResetModel();
    }
}

QVariantMap CharacterListModel::buildFromCharacter(const CharacterObject& character, const ResourceManager& resources) const {
    // 复用 ViewModel 的 animationPresentation 逻辑 —— 直接在这里构建 QVariantMap
    // 为了避免代码重复，我们在这里用 resources 直接生成数据

    QVariantMap item;
    item[QStringLiteral("id")] = character.id;
    item[QStringLiteral("kind")] = character.kind;
    item[QStringLiteral("x")] = character.position.x();
    item[QStringLiteral("y")] = character.position.y();
    item[QStringLiteral("width")] = character.charWidth;
    item[QStringLiteral("height")] = character.charHeight;
    item[QStringLiteral("facingLeft")] = character.facingLeft;
    item[QStringLiteral("alive")] = character.alive;
    item[QStringLiteral("state")] = character.state;
    item[QStringLiteral("frameIndex")] = character.frameIndex;
    item[QStringLiteral("frameCount")] = character.frameCount;
    item[QStringLiteral("attackDirection")] = character.attackDirection;
    item[QStringLiteral("rollAttack")] = character.rollAttack;

    // 动画展现（sprite）
    const bool isSheet = resources.isSpriteSheetAnimation(character.animationKey);
    const int frameCount = isSheet ? resources.spriteSheetFrameCount(character.animationKey) : resources.animationFrameCount(character.animationKey);
    const int clampedFrame = frameCount > 0 ? std::clamp(character.frameIndex, 0, frameCount - 1) : 0;
    item[QStringLiteral("spriteSource")] = isSheet
        ? resources.spriteSheetImage(character.animationKey)
        : resources.animationFrame(character.animationKey, clampedFrame);
    item[QStringLiteral("spriteIsSheet")] = isSheet;
    item[QStringLiteral("spriteFrameCount")] = frameCount;
    item[QStringLiteral("spriteFrameIndex")] = clampedFrame;
    item[QStringLiteral("spriteFrameWidth")] = isSheet ? resources.spriteSheetFrameWidth(character.animationKey) : 0;
    item[QStringLiteral("spriteFrameHeight")] = isSheet ? resources.spriteSheetFrameHeight(character.animationKey) : 0;

    // VFX
    const QString fallbackVfxKey = character.kind == QStringLiteral("enemy")
        ? QStringLiteral("mob.small_bee.vfx.attack_%1").arg(character.attackDirection)
        : QStringLiteral("player.vfx.attack.%1").arg(character.attackDirection);
    const QString vfxKey = character.attackVfxKey.isEmpty() ? fallbackVfxKey : character.attackVfxKey;
    const bool vfxIsSheet = resources.isSpriteSheetAnimation(vfxKey);
    const int vfxFrameCount = vfxIsSheet ? resources.spriteSheetFrameCount(vfxKey) : resources.animationFrameCount(vfxKey);
    const int vfxClampedFrame = vfxFrameCount > 0 ? std::clamp(character.frameIndex, 0, vfxFrameCount - 1) : 0;
    item[QStringLiteral("vfxSource")] = vfxIsSheet
        ? resources.spriteSheetImage(vfxKey)
        : resources.animationFrame(vfxKey, vfxClampedFrame);
    item[QStringLiteral("vfxIsSheet")] = vfxIsSheet;
    item[QStringLiteral("vfxFrameCount")] = vfxFrameCount;
    item[QStringLiteral("vfxFrameIndex")] = vfxClampedFrame;
    item[QStringLiteral("vfxFrameWidth")] = vfxIsSheet ? resources.spriteSheetFrameWidth(vfxKey) : 0;
    item[QStringLiteral("vfxFrameHeight")] = vfxIsSheet ? resources.spriteSheetFrameHeight(vfxKey) : 0;
    item[QStringLiteral("attackVfxSize")] = std::max(character.attackBox.rect.width(), character.attackBox.rect.height());
    item[QStringLiteral("attackBoxX")] = character.attackBox.rect.x();
    item[QStringLiteral("attackBoxY")] = character.attackBox.rect.y();
    item[QStringLiteral("attackBoxWidth")] = character.attackBox.rect.width();
    item[QStringLiteral("attackBoxHeight")] = character.attackBox.rect.height();

    return item;
}

} // namespace skybound
