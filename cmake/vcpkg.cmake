# Automatic vcpkg bootstrap and integration
# This script automatically sets up vcpkg if not already configured

if(NOT DEFINED VCPKG_ROOT)
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    else()
        set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
    endif()
endif()

set(VCPKG_EXECUTABLE "${VCPKG_ROOT}/vcpkg")
if(WIN32)
    set(VCPKG_EXECUTABLE "${VCPKG_EXECUTABLE}.exe")
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
else()
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.sh")
endif()

if(NOT EXISTS "${VCPKG_EXECUTABLE}")
    message(STATUS "vcpkg not found, setting up automatically...")
    if(NOT EXISTS "${VCPKG_ROOT}")
        message(STATUS "Cloning vcpkg from GitHub...")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
            RESULT_VARIABLE GIT_RESULT
            OUTPUT_QUIET
            ERROR_VARIABLE GIT_ERROR
        )
        if(NOT GIT_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone vcpkg: ${GIT_ERROR}")
        endif()
        message(STATUS "vcpkg cloned to ${VCPKG_ROOT}")
    endif()

    if(EXISTS "${VCPKG_BOOTSTRAP}")
        message(STATUS "Bootstrapping vcpkg...")
        execute_process(
            COMMAND ${VCPKG_BOOTSTRAP}
            WORKING_DIRECTORY "${VCPKG_ROOT}"
            RESULT_VARIABLE BOOTSTRAP_RESULT
            OUTPUT_VARIABLE BOOTSTRAP_OUTPUT
            ERROR_VARIABLE BOOTSTRAP_ERROR
        )
        if(NOT BOOTSTRAP_RESULT EQUAL 0)
            message(WARNING "vcpkg bootstrap failed: ${BOOTSTRAP_ERROR}")
            message(STATUS "Falling back to manual dependency management")
            return()
        endif()
        message(STATUS "vcpkg bootstrapped successfully")
    else()
        message(WARNING "vcpkg bootstrap script not found, falling back to manual dependency management")
        return()
    endif()
endif()

if(EXISTS "${VCPKG_EXECUTABLE}")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    message(STATUS "vcpkg toolchain enabled: ${CMAKE_TOOLCHAIN_FILE}")

    set(ENV{VCPKG_ROOT} "${VCPKG_ROOT}")
else()
    message(STATUS "vcpkg not available, will use system packages or FetchContent")
endif()
