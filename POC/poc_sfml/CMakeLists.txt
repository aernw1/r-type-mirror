cmake_minimum_required(VERSION 3.17)
project(SFML_PoC)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

add_executable(poc_sfml poc_sfml.cpp)
target_link_libraries(poc_sfml PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
