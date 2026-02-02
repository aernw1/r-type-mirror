# Three-tier SFML 2.x dependency resolution:
# 1. vcpkg (if CMAKE_TOOLCHAIN_FILE is set)
# 2. System-installed SFML 2.x (via find_package)
# 3. FetchContent (download and build SFML 2.6.2 from source)

set(SFML_FOUND FALSE)

# Tier 1: Try vcpkg first (already configured by root CMakeLists.txt)
if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    message(STATUS "Searching for SFML 2.6+ via vcpkg...")
    find_package(SFML 2.6 COMPONENTS graphics window system QUIET)
    if(SFML_FOUND)
        message(STATUS "Found SFML ${SFML_VERSION} via vcpkg")
    endif()
endif()

# Tier 2: Try system-installed SFML 2.6+
if(NOT SFML_FOUND)
    message(STATUS "Searching for system-installed SFML 2.6+...")
    find_package(SFML 2.6 COMPONENTS graphics window system QUIET)
    if(SFML_FOUND)
        message(STATUS "Found system-installed SFML ${SFML_VERSION}")
    endif()
endif()

# Tier 3: Fetch SFML 2.6.2 from source if not found
if(NOT SFML_FOUND)
    message(STATUS "SFML 2.x not found, fetching SFML 2.6.2 from source...")

    include(FetchContent)

    # Save current BUILD_SHARED_LIBS to avoid polluting other dependencies
    set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
        GIT_SHALLOW TRUE
    )

    # Don't build audio and network modules (we only need graphics/window/system)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SFML)

    # Restore original BUILD_SHARED_LIBS setting
    set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

    set(SFML_LIBRARIES sfml-graphics sfml-window sfml-system)
    set(SFML_FOUND TRUE)
    message(STATUS "SFML 2.6.2 fetched and configured")
endif()

# Determine library names based on target availability
if(SFML_FOUND)
    if(TARGET SFML::Graphics)
        # vcpkg uses SFML:: namespace
        set(SFML_LIBRARIES SFML::Graphics SFML::Window SFML::System)
        message(STATUS "Using SFML 2.x with SFML:: namespace")
    elseif(TARGET sfml-graphics)
        # FetchContent or manual install uses sfml- prefix
        set(SFML_LIBRARIES sfml-graphics sfml-window sfml-system)
        message(STATUS "Using SFML 2.x with sfml- prefix")
    else()
        message(FATAL_ERROR "SFML found but could not determine target names. "
                            "Expected SFML::Graphics or sfml-graphics targets. "
                            "Please ensure SFML 2.6+ is properly installed.")
    endif()
else()
    message(FATAL_ERROR "SFML 2.6+ could not be found via vcpkg, system packages, or FetchContent. "
                        "Please check your CMake configuration.")
endif()

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
        ${SFML_LIBRARIES}
)

target_compile_features(rtype_sfml_renderer PUBLIC cxx_std_17)

if(WIN32)
    target_compile_definitions(rtype_sfml_renderer PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set_target_properties(rtype_sfml_renderer PROPERTIES
    PREFIX ""
    OUTPUT_NAME "SFMLRenderer"
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS rtype_sfml_renderer
    LIBRARY DESTINATION lib/rtype/modules
    RUNTIME DESTINATION bin/rtype/modules
)
