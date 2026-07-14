### 问题 1：View 直接依赖 Service 层

**违反规则**：View 应该只知道 ViewModel，但重构前 QML 中直接调用了 `resourceManager`。

```qml
<!-- 重构前的代码 —— View 直接调用了 Service 层 -->
source: resourceManager.animationFrame(modelData.animationKey, modelData.frameIndex)
```
 
不要qml去调用resource manager，改用worldviewmodel去透传resource manager管理和加载的图片素材

### 问题 2：ViewModel 承担过多职责

**违反规则**：MVVM 不强制 SRP（单一职责原则），但 ViewModel 过于臃肿会降低可维护性。

**修复方案（Phase 4）**：将 `tick()` 中的角色循环逻辑提取到 Domain 层的 `WorldProcessor`，ViewModel 的 `tick()` 只做协调。

### 问题 3：全量刷新策略在大型场景下效率低

**违反规则**：不是严格的 MVVM 违反，但影响架构的质量。

**修复方案（Phase 3）**：限制 `worldChanged()` 的发射范围，保留细粒度信号。

重构前：`worldChanged()` 在 10+ 处不同函数中被发射（`playerRun`、`playerJump`、`playerAttack` 等每处动作处理器）。

重构后：`worldChanged()` 仅在三处发射：
1. `tick()` — 主循环每帧刷新
2. `reset()` — 完整状态重置
3. `setViewport()` — 视口变化

## 问题 4：Domain 层函数直接修改 ViewModel 的成员变量

**违反规则**：Domain 层的系统函数不应该直接知道 ViewModel 的内部状态。

**修复方案（Phase 2）**：引入 `CombatResult` 结构体，让 Domain 函数通过返回值与 ViewModel 通信。

重构前——Domain 函数通过 mutable 引用修改 ViewModel 成员：

### 问题 5：缺少单元测试

**违反规则**：不是 MVVM 的结构违反，但失去了 MVVM 的一个核心优势——可测试性。

MVVM 的重要优势之一是 Domain 逻辑可以独立于 UI 进行单元测试。当前工程中：

- Domain 层函数（`resolveTerrainCollision`、`checkAttackHits` 等）是纯函数，**天然适合测试**
- 但没有对应的单元测试文件
- ViewModel 的 tick() 循环逻辑也没有测试

**建议**：使用 Qt Test 框架为 Domain 层的关键函数添加单元测试，特别是碰撞解析和攻击命中检测这类容易出错的逻辑。