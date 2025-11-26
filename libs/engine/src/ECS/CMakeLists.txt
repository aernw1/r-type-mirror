set(ECS_SOURCES
    Registry.cpp
    MovementSystem.cpp
    RenderingSystem.cpp
)

set(ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Component.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/ISystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/MovementSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ECS/RenderingSystem.hpp
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
