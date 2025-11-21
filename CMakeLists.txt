cmake_minimum_required(VERSION 3.17)
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

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
if(WIN32)
    target_link_libraries(asio INTERFACE ws2_32 wsock32)
endif()

find_package(Threads REQUIRED)

add_subdirectory(network)
add_subdirectory(engine)

add_executable(r-type_server server/main.cpp)
target_link_libraries(r-type_server PRIVATE rtype_network)

add_executable(r-type_client client/main.cpp)
target_link_libraries(r-type_client PRIVATE rtype_network)

add_executable(test_lobby tests/test_lobby.cpp)
target_link_libraries(test_lobby PRIVATE rtype_network)
