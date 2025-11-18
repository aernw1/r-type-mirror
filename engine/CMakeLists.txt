add_subdirectory(src/ECS)

add_library(rtype_engine_core INTERFACE)

target_include_directories(rtype_engine_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
