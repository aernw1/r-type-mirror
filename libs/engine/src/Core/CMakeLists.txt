set(CORE_SOURCES
    Engine.cpp
    ModuleLoader.cpp
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/Engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/Module.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/ModuleLoader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/Logger.hpp
)

add_library(rtype_core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(rtype_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_core PUBLIC
    rtype_ecs
    ${CMAKE_DL_LIBS}
)

target_compile_features(rtype_core PUBLIC cxx_std_17)

set_target_properties(rtype_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if(WIN32)
    target_compile_definitions(rtype_core PUBLIC
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()
