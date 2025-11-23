add_subdirectory(src/ECS)
add_subdirectory(src/Core)
add_subdirectory(src/Renderer)

add_library(rtype_engine_core INTERFACE)

target_include_directories(rtype_engine_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link with Core module which includes ECS
target_link_libraries(rtype_engine_core INTERFACE
    rtype_core
)
