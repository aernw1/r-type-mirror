set(ECS_SOURCES
    Registry.cpp
    AudioSystem.cpp
    MovementSystem.cpp
    InputSystem.cpp
    RenderingSystem.cpp
    TextRenderingSystem.cpp
    MenuSystem.cpp
    PlayerSystem.cpp
    PlayerFactory.cpp
    CollisionDetectionSystem.cpp
    BulletCollisionResponseSystem.cpp
    PlayerCollisionResponseSystem.cpp
    ObstacleCollisionResponseSystem.cpp
    EnemySystem.cpp
    EnemyFactory.cpp
    HealthSystem.cpp
    ScoreSystem.cpp
    ShootingSystem.cpp
    ScrollingSystem.cpp
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
    AnimationSystem.cpp
    EffectFactory.cpp
)

set(ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Component.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/AudioSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/TextLabel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Clickable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ISystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/SparseArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MovementSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/InputSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/RenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/TextRenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MenuSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/CollisionDetectionSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/BulletCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ObstacleCollisionResponseSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemySystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemyFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/HealthSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ScoreSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ShootingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ScrollingSystem.hpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/AnimationSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EffectFactory.hpp
)

add_library(rtype_ecs STATIC
    ${ECS_SOURCES}
    ${ECS_HEADERS}
)

target_include_directories(rtype_ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_ecs PUBLIC nlohmann_json::nlohmann_json rtype_animation)

set_target_properties(rtype_ecs PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
