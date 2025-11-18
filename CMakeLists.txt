cmake_minimum_required(VERSION 3.17)
project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic --std=c++17)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(rtype_ecs
    src/ECS/Registry.cpp
)

target_include_directories(rtype_ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(rtype_test
    src/main.cpp
)
target_link_libraries(rtype_test PRIVATE rtype_ecs)

install(TARGETS rtype_ecs
    EXPORT RTypeTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
)
