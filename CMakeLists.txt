cmake_minimum_required(VERSION 3.24)

project(SkyboundTactics VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2 Test)

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
    src/domain/WorldProcessor.h
    src/domain/WorldProcessor.cpp
    src/viewmodel/GameWorldViewModel.h
    src/viewmodel/GameWorldViewModel.cpp
)

target_include_directories(SkyboundTactics PRIVATE
    src
)

target_link_libraries(SkyboundTactics PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
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
        resources/background3.png
        "resources/Mob/Small Bee/Fly/Fly-Sheet.png"
        "resources/Mob/Small Bee/Attack/Attack-Sheet.png"
        "resources/Mob/Small Bee/Hit/Hit-Sheet.png"
        "resources/Mob/Small Bee/Hit/hit.png"
        "resources/Mob/Small Bee/Hit/hit_left.png"
        resources/player/idle/1.png
        resources/player/idle/2.png
        resources/player/idle/3.png
        resources/player/idle/4.png
        resources/player/idle/5.png
        resources/player/idle/6.png
        resources/player/run/1.png
        resources/player/run/2.png
        resources/player/run/3.png
        resources/player/run/4.png
        resources/player/run/5.png
        resources/player/run/6.png
        resources/player/run/7.png
        resources/player/run/8.png
        resources/player/jump/1.png
        resources/player/jump/2.png
        resources/player/jump/3.png
        resources/player/jump/4.png
        resources/player/jump/5.png
        resources/player/jump/6.png
        resources/player/jump/7.png
        resources/player/jump/8.png
        resources/player/fall/1.png
        resources/player/fall/2.png
        resources/player/fall/3.png
        resources/player/fall/4.png
        resources/player/hit/1.png
        resources/player/hit/2.png
        resources/player/hit/3.png
        resources/player/hit/4.png
        resources/player/dash_on_floor/1.png
        resources/player/dash_on_floor/2.png
        resources/player/squat/1.png
        resources/player/squat/2.png
        resources/player/squat/3.png
        resources/player/squat/4.png
        resources/player/squat/5.png
        resources/player/squat/6.png
        resources/player/squat/7.png
        resources/player/squat/8.png
        resources/player/squat/9.png
        resources/player/squat/10.png
        resources/player/silk/1.png
        resources/player/silk/2.png
        resources/player/silk/3.png
        resources/player/silk/4.png
        resources/player/silk/5.png
        resources/player/silk/6.png
        resources/player/silk/7.png
        resources/player/silk/8.png
        resources/player/silk/9.png
        resources/player/throw_barb/1.png
        resources/player/throw_barb/2.png
        resources/player/throw_barb/3.png
        resources/player/throw_barb/4.png
        resources/player/throw_barb/5.png
        resources/player/throw_barb/6.png
        resources/player/throw_barb/7.png
        resources/player/throw_barb/8.png
        resources/player/sprite/sprite_sprite.png
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
        resources/audio/enemy_run.mp3
        resources/audio/enemy_dash.mp3
        resources/audio/enemy_hurt_1.mp3
        resources/audio/enemy_hurt_2.mp3
        resources/audio/enemy_hurt_3.mp3
        resources/audio/enemy_throw_barbs.mp3
        resources/audio/enemy_throw_silk.mp3
        resources/audio/enemy_throw_sword.mp3
        resources/audio/barb_break.mp3
        resources/audio/bullet_time.mp3
)

enable_testing()

add_executable(DomainSystemsTest
    tests/DomainSystemsTest.cpp
    src/model/GameWorldTypes.h
    src/domain/CharacterSystem.h
    src/domain/CharacterSystem.cpp
    src/domain/CollisionSystem.h
    src/domain/CollisionSystem.cpp
    src/domain/CombatSystem.h
    src/domain/CombatSystem.cpp
)
target_include_directories(DomainSystemsTest PRIVATE src)
target_link_libraries(DomainSystemsTest PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
if(WIN32)
    add_custom_command(TARGET DomainSystemsTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE:Qt6::Test>
            $<TARGET_FILE_DIR:DomainSystemsTest>
        COMMAND_EXPAND_LISTS
    )
endif()
add_test(NAME DomainSystemsTest COMMAND DomainSystemsTest)

if(WIN32)
    add_custom_command(TARGET SkyboundTactics POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE:Qt6::Multimedia>
            $<TARGET_FILE:Qt6::Qml>
            $<TARGET_FILE:Qt6::Quick>
            $<TARGET_FILE:Qt6::QuickControls2>
            $<TARGET_FILE_DIR:SkyboundTactics>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:Qt6::Core>/../plugins/multimedia"
            "$<TARGET_FILE_DIR:SkyboundTactics>/multimedia"
        COMMAND_EXPAND_LISTS
    )
endif()
