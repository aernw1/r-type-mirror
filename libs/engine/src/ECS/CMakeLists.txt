set(ECS_SOURCES
    Registry.cpp
    MovementSystem.cpp
    InputSystem.cpp
    RenderingSystem.cpp
    TextRenderingSystem.cpp
    PlayerSystem.cpp
    EnemySystem.cpp
    EnemyFactory.cpp
)

set(ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Component.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ISystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MovementSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/InputSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/RenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/TextRenderingSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/PlayerSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemySystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/EnemyFactory.hpp
)

add_library(rtype_ecs STATIC
    ${ECS_SOURCES}
    ${ECS_HEADERS}
)

target_include_directories(rtype_ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(rtype_ecs PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
