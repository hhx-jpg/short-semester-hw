# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and run

This is a CMake-based Qt 6.8+ desktop application (C++20, Qt Quick/QML). The out-of-source build directory is `D:\SkyboundTactics-qt-build`, not the repository's ignored `build/` directory.

```powershell
# Configure (from the repository root)
cmake -S . -B D:\SkyboundTactics-qt-build

# Build the application
cmake --build D:\SkyboundTactics-qt-build --target SkyboundTactics

# Run the built executable
D:\SkyboundTactics-qt-build\SkyboundTactics.exe
```

There is currently no automated test target or lint configuration. Verify gameplay changes by building and running the Qt application. The QML window documents the controls: A/D move, Space jumps, Shift rolls, right mouse attacks/charges, W/S choose attack direction, and K applies player damage.

## Architecture

- `src/main.cpp` is the composition root: it creates `ResourceManager` and `GameWorldViewModel`, exposes them to QML as `resourceManager` and `gameWorld`, then loads `qrc:/Main.qml`.
- `qml/Main.qml` is the Qt Quick view. It owns input handling, the 16 ms game-loop timer, visual repeaters for map layers/actors/hitboxes, VFX, and HUD. It should invoke exposed `GameWorldViewModel` methods rather than own simulation rules.
- `src/viewmodel/GameWorldViewModel.*` is the QML-facing coordinator. Its `Q_PROPERTY` values are converted to `QVariantList`/`QVariantMap` render data, while its `Q_INVOKABLE` methods receive player input. Each `tick()` orchestrates AI, animation, physics, collision-box refresh, scene switches, combat, and Qt signals.
- `src/model/GameWorldTypes.h` defines the shared world data model: character state, terrain/map geometry, physics tuning, transient events, and scene-switch/build result structures.
- `src/domain/` contains stateless gameplay subsystems operating on those model structs:
  - `CharacterFactory` creates the player and small-bee enemy defaults.
  - `CharacterSystem` owns state transitions, animation selection, frame advancement, and attacks.
  - `PhysicsSystem` applies movement/gravity, terrain boundaries, and scene-transition requests.
  - `CollisionSystem` calculates hurt/attack boxes and resolves terrain collisions.
  - `CombatSystem` resolves attack-box/hurtbox overlaps and deduplicates each attack serial.
  - `NpcSystem` supplies the small-bee chase/attack AI.
  - `SceneBuilder` produces map layers, terrain, and playable bounds for each `SceneId`.
- `src/service/ResourceManager.*` maps stable logical resource/animation keys to `qrc:/` URLs and sprite-sheet metadata for QML. When adding an asset, add it to `CMakeLists.txt` resources and register its key/animation here.
- `CMakeLists.txt` is the canonical source/resource manifest. New C++ sources, QML files, images, and audio must be added to the relevant target/resource list or they will not be compiled/embedded.

## Project conventions

- Keep QML focused on input and rendering; keep mutable gameplay state and simulation logic in `GameWorldViewModel` plus `src/domain/`.
- Use the existing `skybound` namespace and `QStringLiteral(...)` style in C++.
- `GameWorldViewModel::tick()` is the central update ordering. Preserve the sequence of AI → animation → physics → collision boxes → scene switch → combat when extending systems, unless a deliberate gameplay reason requires changing it.
- Resource keys are the coupling point between gameplay state, `ResourceManager`, and QML. Update all three sides when introducing a new animation/VFX key.
- The working tree currently contains user changes, including `MVVM_todo.md`; do not overwrite or discard unrelated modifications.
