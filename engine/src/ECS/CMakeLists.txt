# ECS CMakeLists.txt
#
# This builds the core ECS library (game-agnostic).
# R-Type specific systems have been moved to games/rtype/

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
    Camera2DSystem.cpp
    PhysicsSystem.cpp
    PhysicsCollisionSystem.cpp
)

set(ECS_CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ISystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/SparseArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Component.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/IComponent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Transform.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Rendering.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Physics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Audio.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Animation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Gameplay.hpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Camera2DSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Components/Camera2D.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PhysicsSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PhysicsCollisionSystem.hpp
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
# Legacy rtype_ecs library (alias to rtype_ecs_core for backwards compatibility)
# =============================================================================
# Note: R-Type specific systems are now in games/rtype/systems/ and built as rtype_game
add_library(rtype_ecs ALIAS rtype_ecs_core)
