cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    include(cmake/vcpkg.cmake)
endif()

project(RType VERSION 1.0.0 LANGUAGES CXX)

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

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(libs/engine)
add_subdirectory(libs/network)

add_executable(r-type_server server/main.cpp)
target_link_libraries(r-type_server PRIVATE rtype_network rtype_asio_network)

add_executable(r-type_client
    client/main.cpp
    client/src/MenuState.cpp
    client/src/room/RoomListState.cpp
    client/src/LobbyState.cpp
    client/src/GameState.cpp
    client/src/ResultsState.cpp
    client/src/EditorState.cpp
    client/src/level_transition/LevelTransitionState.cpp
    client/src/editor/EditorCanvasManager.cpp
    client/src/editor/EditorAssetLibrary.cpp
    client/src/editor/EditorUIManager.cpp
    client/src/editor/EditorEntityManager.cpp
    client/src/editor/EditorFileManager.cpp
    # Editor utilities
    client/src/editor/EditorInputHandler.cpp
    client/src/editor/EditorDrawing.cpp
    client/src/editor/EditorPropertyManager.cpp
    client/src/editor/EditorColliderManager.cpp
)
target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
)
target_link_libraries(r-type_client PRIVATE
    rtype_network
    rtype_asio_network
    rtype_ecs
    rtype_core
    rtype_sfml_renderer
    rtype_sfml_audio
)

add_executable(test_lobby tests/test_lobby.cpp)
target_link_libraries(test_lobby PRIVATE rtype_network rtype_asio_network)

add_executable(test_sparse_array tests/test_sparse_array.cpp)
target_link_libraries(test_sparse_array PRIVATE rtype_ecs)

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

    add_executable(test_animation tests/test_animation.cpp)
    target_link_libraries(test_animation PRIVATE
        rtype_core
        rtype_ecs
        rtype_animation
    )
    message(STATUS "test_animation will be built (tests explosion animation)")

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
