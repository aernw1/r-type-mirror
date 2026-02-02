set(ANIMATION_SOURCES
    AnimationModule.cpp
)

set(ANIMATION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Animation/AnimationTypes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Animation/IAnimation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Animation/AnimationModule.hpp
)

add_library(rtype_animation STATIC
    ${ANIMATION_SOURCES}
    ${ANIMATION_HEADERS}
)

target_include_directories(rtype_animation PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_animation
    PUBLIC
        rtype_core
        nlohmann_json::nlohmann_json
)

target_compile_features(rtype_animation PUBLIC cxx_std_17)

set_target_properties(rtype_animation PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
