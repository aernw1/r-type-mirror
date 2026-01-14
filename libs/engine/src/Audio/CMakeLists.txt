# SFML Audio backend

set(SFML_AUDIO_FOUND FALSE)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    message(STATUS "Searching for SFML Audio via vcpkg...")
    find_package(SFML 2.6 COMPONENTS audio system QUIET)
    if(SFML_FOUND)
        set(SFML_AUDIO_FOUND TRUE)
        message(STATUS "Found SFML ${SFML_VERSION} (audio) via vcpkg")
    endif()
endif()

if(NOT SFML_AUDIO_FOUND)
    message(STATUS "Searching for system-installed SFML Audio 2.6+...")
    find_package(SFML 2.6 COMPONENTS audio system QUIET)
    if(SFML_FOUND)
        set(SFML_AUDIO_FOUND TRUE)
        message(STATUS "Found system-installed SFML ${SFML_VERSION} (audio)")
    endif()
endif()

if(NOT SFML_AUDIO_FOUND)
    message(STATUS "SFML Audio not found, fetching SFML 2.6.2 from source...")
    include(FetchContent)

    set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
        GIT_SHALLOW TRUE
    )

    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SFML)

    set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
    set(SFML_AUDIO_FOUND TRUE)
    message(STATUS "SFML 2.6.2 fetched and configured (audio)")
endif()

if(SFML_AUDIO_FOUND)
    if(TARGET SFML::Audio)
        set(SFML_AUDIO_LIBS SFML::Audio SFML::System)
        message(STATUS "Using SFML Audio targets with SFML:: namespace")
    elseif(TARGET sfml-audio)
        set(SFML_AUDIO_LIBS sfml-audio sfml-system)
        message(STATUS "Using SFML Audio targets with sfml- prefix")
    else()
        message(FATAL_ERROR "SFML Audio found but targets not available (expected SFML::Audio or sfml-audio).")
    endif()
endif()

add_library(rtype_sfml_audio STATIC
    SFMLAudio.cpp
)

target_include_directories(rtype_sfml_audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

find_package(fmt CONFIG REQUIRED)

target_link_libraries(rtype_sfml_audio
    PUBLIC
        rtype_core
        ${SFML_AUDIO_LIBS}
        fmt::fmt
)

target_compile_features(rtype_sfml_audio PUBLIC cxx_std_17)

