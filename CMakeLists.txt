cmake_minimum_required(VERSION 3.24)

project(SkyboundTactics VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)

add_executable(SkyboundTactics WIN32
    src/main.cpp
    src/service/ResourceManager.h
    src/service/ResourceManager.cpp
    src/model/GameWorldTypes.h
    src/domain/CharacterFactory.h
    src/domain/CharacterFactory.cpp
    src/domain/CharacterSystem.h
    src/domain/CharacterSystem.cpp
    src/domain/CollisionSystem.h
    src/domain/CollisionSystem.cpp
    src/domain/CombatSystem.h
    src/domain/CombatSystem.cpp
    src/domain/NpcSystem.h
    src/domain/NpcSystem.cpp
    src/domain/PhysicsSystem.h
    src/domain/PhysicsSystem.cpp
    src/domain/SceneBuilder.h
    src/domain/SceneBuilder.cpp
    src/viewmodel/GameWorldViewModel.h
    src/viewmodel/GameWorldViewModel.cpp
)

target_include_directories(SkyboundTactics PRIVATE
    src
)

target_link_libraries(SkyboundTactics PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::EntryPointPrivate
)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)

qt_add_resources(SkyboundTactics qml_resources
    PREFIX "/"
    FILES
        qml/Main.qml
)

qt_add_resources(SkyboundTactics sprite_resources
    PREFIX "/"
    FILES
        resources/background.png
        resources/background2.png
        resources/ui_heart.png
        "resources/Mob/Small Bee/Fly/Fly-Sheet.png"
        "resources/Mob/Small Bee/Attack/Attack-Sheet.png"
        "resources/Mob/Small Bee/Hit/Hit-Sheet.png"
        "resources/Mob/Small Bee/Hit/hit.png"
        resources/enemy/run/1.png
        resources/enemy/run/2.png
        resources/enemy/run/3.png
        resources/enemy/run/4.png
        resources/enemy/run/5.png
        resources/enemy/run/6.png
        resources/enemy/run/7.png
        resources/enemy/run/8.png
        resources/enemy/jump/1.png
        resources/enemy/jump/2.png
        resources/enemy/jump/3.png
        resources/enemy/jump/4.png
        resources/enemy/jump/5.png
        resources/enemy/jump/6.png
        resources/enemy/jump/7.png
        resources/enemy/jump/8.png
        resources/player/idle.png
        resources/player/run.png
        resources/player/jump.png
        resources/player/fall.png
        resources/player/attack.png
        resources/player/roll.png
        resources/player/dead.png
        resources/player/hit/1.png
        resources/player/hit/2.png
        resources/player/hit/3.png
        resources/player/hit/4.png
        resources/player/vfx_attack_down.png
        resources/player/vfx_attack_left.png
        resources/player/vfx_attack_right.png
        resources/player/vfx_attack_up.png
        resources/player/vfx_jump.png
        resources/player/vfx_land.png
        resources/audio/bgm.mp3
        resources/audio/player_attack_1.mp3
        resources/audio/player_attack_2.mp3
        resources/audio/player_attack_3.mp3
        resources/audio/player_jump.mp3
        resources/audio/player_land.mp3
        resources/audio/player_roll.mp3
        resources/audio/player_hurt.mp3
        resources/audio/player_dead.mp3
        resources/audio/player_run.mp3
)

if(WIN32)
    add_custom_command(TARGET SkyboundTactics POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE:Qt6::Qml>
            $<TARGET_FILE:Qt6::Quick>
            $<TARGET_FILE:Qt6::QuickControls2>
            $<TARGET_FILE_DIR:SkyboundTactics>
        COMMAND_EXPAND_LISTS
    )
endif()
