cmake_minimum_required(VERSION 3.17)
project(SDL2_PoC)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(poc_sdl2 poc_sdl2.cpp)
target_link_libraries(poc_sdl2 PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
)
