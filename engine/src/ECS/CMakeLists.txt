# ECS CMakeLists.txt
#
# This builds two libraries:
# 1. rtype_ecs_core - Generic ECS components and systems (reusable by any game)
# 2. rtype_ecs - Full ECS including R-Type specific code (for backwards compatibility)

# =============================================================================
# Generic ECS Core Library (game-agnostic)
# =============================================================================
set(ECS_CORE_SOURCES
    Registry.cpp
    MovementSystem.cpp
    RenderingSystem.cpp
    TextRenderingSystem.cpp
    CollisionDetectionSystem.cpp
    ScrollingSystem.cpp
    AnimationSystem.cpp
    AudioSystem.cpp
    InputSystem.cpp
    MenuSystem.cpp
    HealthSystem.cpp
    ScoreSystem.cpp
)

set(ECS_CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ISystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/SparseArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Component.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/TextLabel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Clickable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MovementSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/RenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/TextRenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/CollisionDetectionSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ScrollingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/AnimationSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/AudioSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/InputSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MenuSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/HealthSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ScoreSystem.hpp
)

add_library(rtype_ecs_core STATIC
    ${ECS_CORE_SOURCES}
    ${ECS_CORE_HEADERS}
)

target_include_directories(rtype_ecs_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_ecs_core PUBLIC nlohmann_json::nlohmann_json rtype_animation)

set_target_properties(rtype_ecs_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# =============================================================================
# R-Type Specific ECS Extensions (for R-Type game only)
# =============================================================================
set(RTYPE_ECS_SOURCES
    PlayerSystem.cpp
    PlayerFactory.cpp
    BulletCollisionResponseSystem.cpp
    PlayerCollisionResponseSystem.cpp
    ObstacleCollisionResponseSystem.cpp
    EnemySystem.cpp
    EnemyFactory.cpp
    ShootingSystem.cpp
    LevelLoader.cpp
    PowerUpFactory.cpp
    PowerUpSpawnSystem.cpp
    PowerUpCollisionSystem.cpp
    ForcePodSystem.cpp
    ShieldSystem.cpp
    BossSystem.cpp
    BossAttackSystem.cpp
    MineSystem.cpp
    BlackOrbSystem.cpp
    ThirdBulletSystem.cpp
    EffectFactory.cpp
)

set(RTYPE_ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/BulletCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ObstacleCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemySystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemyFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ShootingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/LevelLoader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PowerUpFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PowerUpSpawnSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PowerUpCollisionSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ForcePodSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ShieldSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/BossSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/BossAttackSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MineSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/BlackOrbSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ThirdBulletSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EffectFactory.hpp
)

# Full rtype_ecs library (includes both core and R-Type specific)
# This maintains backwards compatibility with existing code
add_library(rtype_ecs STATIC
    ${ECS_CORE_SOURCES}
    ${RTYPE_ECS_SOURCES}
    ${ECS_CORE_HEADERS}
    ${RTYPE_ECS_HEADERS}
)

target_include_directories(rtype_ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_ecs PUBLIC nlohmann_json::nlohmann_json rtype_animation)

set_target_properties(rtype_ecs PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
