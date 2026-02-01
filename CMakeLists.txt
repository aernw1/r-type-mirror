cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    include(cmake/vcpkg.cmake)
endif()

project(RTypeEngine VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Copy assets to build directory
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

# =============================================================================
# Dependencies
# =============================================================================
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)

# =============================================================================
# Engine Libraries (generic, reusable by any game)
# =============================================================================
add_subdirectory(engine)

# =============================================================================
# Network Libraries (also generic)
# =============================================================================
add_subdirectory(libs/network)

# =============================================================================
# Games
# =============================================================================

# R-Type Game
add_executable(r-type_server games/rtype/server/main.cpp)
target_link_libraries(r-type_server PRIVATE rtype_network rtype_asio_network)

add_executable(r-type_client
    games/rtype/client/main.cpp
    games/rtype/client/src/MenuState.cpp
    games/rtype/client/src/SettingsState.cpp
    games/rtype/client/src/room/RoomListState.cpp
    games/rtype/client/src/LobbyState.cpp
    games/rtype/client/src/GameState.cpp
    games/rtype/client/src/ResultsState.cpp
    games/rtype/client/src/EditorState.cpp
    games/rtype/client/src/editor/EditorCanvasManager.cpp
    games/rtype/client/src/editor/EditorAssetLibrary.cpp
    games/rtype/client/src/editor/EditorUIManager.cpp
    games/rtype/client/src/editor/EditorEntityManager.cpp
    games/rtype/client/src/editor/EditorFileManager.cpp
    games/rtype/client/src/editor/EditorInputHandler.cpp
    games/rtype/client/src/editor/EditorDrawing.cpp
    games/rtype/client/src/editor/EditorPropertyManager.cpp
    games/rtype/client/src/editor/EditorColliderManager.cpp
)
target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/games/rtype/client/include
)
target_link_libraries(r-type_client PRIVATE
    rtype_network
    rtype_asio_network
    rtype_ecs
    rtype_core
    rtype_sfml_renderer
    rtype_sfml_audio
)

# Breakout Game (demo of engine reusability)
if(TARGET rtype_sfml_renderer)
    add_executable(breakout
        games/breakout/src/main.cpp
        games/breakout/src/BreakoutPhysicsSystem.cpp
        games/breakout/src/BreakoutRenderSystem.cpp
        games/breakout/src/BallAccelerationSystem.cpp
    )
    target_include_directories(breakout PRIVATE
        ${CMAKE_SOURCE_DIR}/games/breakout/include
    )
    target_link_libraries(breakout PRIVATE
        rtype_core
        rtype_ecs_core
        rtype_sfml_renderer
    )
    message(STATUS "breakout will be built (engine reusability demo)")
endif()

# Metroidvania Game (NEW - to be developed)
# add_subdirectory(games/metroidvania)

# =============================================================================
# Tests
# =============================================================================
add_executable(test_lobby tests/test_lobby.cpp)
target_link_libraries(test_lobby PRIVATE rtype_network rtype_asio_network)

add_executable(test_sparse_array tests/test_sparse_array.cpp)
target_link_libraries(test_sparse_array PRIVATE rtype_ecs_core)

add_executable(test_udp_protocol tests/test_udp_protocol.cpp)
target_link_libraries(test_udp_protocol PRIVATE rtype_network rtype_asio_network)
message(STATUS "test_udp_protocol will be built (UDP game protocol test)")

add_executable(test_tcp_framing tests/test_tcp_framing.cpp)
target_link_libraries(test_tcp_framing PRIVATE rtype_asio_network)

if(TARGET rtype_sfml_renderer)
    add_executable(test_renderer tests/test_renderer.cpp)
    target_link_libraries(test_renderer PRIVATE
        rtype_core
        rtype_ecs
    )
    message(STATUS "test_renderer will be built (loads renderer plugin dynamically)")

    add_executable(test_player_system tests/test_player_system.cpp)
    target_link_libraries(test_player_system PRIVATE
        rtype_core
        rtype_ecs
    )
    message(STATUS "test_player_system will be built (loads renderer plugin dynamically)")

    add_executable(test_health_system tests/test_health_system.cpp)
    target_link_libraries(test_health_system PRIVATE
        rtype_core
        rtype_ecs
    )
    message(STATUS "test_health_system will be built (loads renderer plugin dynamically)")

    add_executable(test_background tests/test_background.cpp)
    target_link_libraries(test_background PRIVATE
        rtype_core
        rtype_ecs
        rtype_sfml_renderer
    )
    message(STATUS "test_background will be built")
endif()
