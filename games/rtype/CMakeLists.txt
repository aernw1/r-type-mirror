# R-Type Game - Systems and Factories
#
# This builds R-Type specific systems as a library that extends rtype_ecs_core.

set(RTYPE_SYSTEMS_SOURCES
    systems/PlayerSystem.cpp
    systems/PlayerFactory.cpp
    systems/BulletCollisionResponseSystem.cpp
    systems/PlayerCollisionResponseSystem.cpp
    systems/ObstacleCollisionResponseSystem.cpp
    systems/EnemySystem.cpp
    systems/EnemyFactory.cpp
    systems/ShootingSystem.cpp
    systems/LevelLoader.cpp
    systems/PowerUpFactory.cpp
    systems/PowerUpSpawnSystem.cpp
    systems/PowerUpCollisionSystem.cpp
    systems/ForcePodSystem.cpp
    systems/ShieldSystem.cpp
    systems/BossSystem.cpp
    systems/BossAttackSystem.cpp
    systems/MineSystem.cpp
    systems/BlackOrbSystem.cpp
    systems/ThirdBulletSystem.cpp
    systems/EffectFactory.cpp
)

set(RTYPE_SYSTEMS_HEADERS
    include/PlayerSystem.hpp
    include/PlayerFactory.hpp
    include/BulletCollisionResponseSystem.hpp
    include/PlayerCollisionResponseSystem.hpp
    include/ObstacleCollisionResponseSystem.hpp
    include/EnemySystem.hpp
    include/EnemyFactory.hpp
    include/ShootingSystem.hpp
    include/LevelLoader.hpp
    include/PowerUpFactory.hpp
    include/PowerUpSpawnSystem.hpp
    include/PowerUpCollisionSystem.hpp
    include/ForcePodSystem.hpp
    include/ShieldSystem.hpp
    include/BossSystem.hpp
    include/BossAttackSystem.hpp
    include/MineSystem.hpp
    include/BlackOrbSystem.hpp
    include/ThirdBulletSystem.hpp
    include/EffectFactory.hpp
)

add_library(rtype_game STATIC
    ${RTYPE_SYSTEMS_SOURCES}
    ${RTYPE_SYSTEMS_HEADERS}
)

target_include_directories(rtype_game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/include
)

target_link_libraries(rtype_game PUBLIC rtype_ecs_core rtype_animation)

set_target_properties(rtype_game PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
