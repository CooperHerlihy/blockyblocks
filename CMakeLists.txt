cmake_minimum_required(VERSION 3.20)

project(blockyblocks LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(SDL3 CONFIG REQUIRED)

add_executable(blockyblocks
    src/bb_input.c
    src/bb_world.c
    src/bb_shader.c
    src/bb_main.c)

target_include_directories(blockyblocks
    PUBLIC
    vendor/glad/include)

target_link_libraries(blockyblocks
    PRIVATE
    SDL3::SDL3)

# libm is a separate library on Unix; on Windows the math functions are in
# the CRT and there is no libm to link.
if(UNIX)
    target_link_libraries(blockyblocks PRIVATE m)
endif()

# On Windows, copy SDL3.dll next to the executable so it can be run
# directly from the build directory.
if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET blockyblocks POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:blockyblocks>
        VERBATIM)
endif()

# MSVC fopen and friends raise deprecation warnings that are treated as
# errors unless suppressed.
if(MSVC)
    target_compile_definitions(blockyblocks PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(blockyblocks PRIVATE /WX /W4)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(blockyblocks PRIVATE -Werror -Wall -Wextra -Wpedantic -Wconversion)

    # Debug builds: symbols plus a light optimization level that keeps the code
    # debuggable (-Og preserves variable mappings and stepping order).
    target_compile_options(blockyblocks PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-Og>)
endif()

