cmake_minimum_required(VERSION 3.20)

project(blockyblocks LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(SDL3 CONFIG REQUIRED)

add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

add_executable(blockyblocks src/main.c)
target_link_libraries(blockyblocks PRIVATE glad SDL3::SDL3)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(blockyblocks PRIVATE -Wall -Wextra -Wpedantic)
endif()
