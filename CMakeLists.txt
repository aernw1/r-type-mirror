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
find_package(Threads REQUIRED)

add_subdirectory(libs/engine)
add_subdirectory(libs/network)

add_executable(r-type_server server/main.cpp)
target_link_libraries(r-type_server PRIVATE rtype_network)

add_executable(r-type_client client/main.cpp)
target_link_libraries(r-type_client PRIVATE
    rtype_network
    rtype_sfml_renderer
    rtype_core
    rtype_ecs
)

add_executable(test_lobby tests/test_lobby.cpp)
target_link_libraries(test_lobby PRIVATE rtype_network)

add_executable(test_sparse_array tests/test_sparse_array.cpp)
target_link_libraries(test_sparse_array PRIVATE rtype_ecs)

if(TARGET rtype_sfml_renderer)
    add_executable(test_renderer tests/test_renderer.cpp)
    target_link_libraries(test_renderer PRIVATE
        rtype_core
        rtype_ecs
    )
    message(STATUS "test_renderer will be built (loads renderer plugin dynamically)")
endif()
