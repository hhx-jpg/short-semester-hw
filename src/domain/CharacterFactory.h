#pragma once

#include "model/GameWorldTypes.h"

namespace skybound {

class CharacterFactory {
public:
    static CharacterObject createPlayer(qreal x, qreal y, const WorldTuning& tuning);
    static CharacterObject createSmallBee(qreal x, qreal y, const WorldTuning& tuning);
    static CharacterObject createSnail(qreal x, qreal y, const WorldTuning& tuning);

    // ──────────────────────────────────────────────
    // 按类型字符串创建怪物
    //
    // 根据 mobType 参数分发到对应的 createXxx 方法，
    // 方便从 SceneBuildResult.mobSpawns 配置文件批量生成怪物。
    //
    // 参数：
    //   type   — 怪物类型字符串，目前支持 "small_bee" / "snail"
    //   x, y   — 出生位置
    //   tuning — 世界调参
    //
    // 返回创建好的 CharacterObject；未知类型返回默认构造的空对象
    // （调用方应检查 id 是否为空来判断是否创建成功）。
    // ──────────────────────────────────────────────
    static CharacterObject createByType(const QString& type, qreal x, qreal y, const WorldTuning& tuning);
};

} // namespace skybound
