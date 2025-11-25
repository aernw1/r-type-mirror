# Find SFML via vcpkg (managed by root CMakeLists.txt)
# SFML 3.x uses capitalized component names
find_package(SFML COMPONENTS Graphics Window System REQUIRED CONFIG)

# Create SFML Renderer library
add_library(rtype_sfml_renderer SHARED
    SFMLRenderer.cpp
)

target_include_directories(rtype_sfml_renderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtype_sfml_renderer
    PUBLIC
        rtype_core
        rtype_ecs
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

target_compile_features(rtype_sfml_renderer PUBLIC cxx_std_17)

# Set module properties
set_target_properties(rtype_sfml_renderer PROPERTIES
    PREFIX ""
    OUTPUT_NAME "SFMLRenderer"
    POSITION_INDEPENDENT_CODE ON
)

# Install rules (optional)
install(TARGETS rtype_sfml_renderer
    LIBRARY DESTINATION lib/rtype/modules
    RUNTIME DESTINATION bin/rtype/modules
)
