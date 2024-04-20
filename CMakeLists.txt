cmake_minimum_required(VERSION 3.15)

if(PROJECT_BINARY_DIR MATCHES PROJECT_SOURCE_DIR)
    message(WARNING "Binary Directory of CMake should be different from Source Directory!")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (WIN32)                #Windows
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

if(NOT MSVC)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Found CCache: ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
    endif()
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(AsioAsyncServer LANGUAGES CXX VERSION 1.0.0)

add_subdirectory(AsyncServer)
add_subdirectory(AsyncClient)
