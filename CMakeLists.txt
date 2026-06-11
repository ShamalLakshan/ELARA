cmake_minimum_required(VERSION 3.16)
project(ELARA VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# Release build strips debug info; Debug keeps it
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Shared submodule headers
add_library(elara_shared INTERFACE)
target_include_directories(elara_shared INTERFACE
    ${CMAKE_SOURCE_DIR}/lib/shared/include
)

# Find TagLib for ID3/FLAC tag reading
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

# stb_image - header-only, vendored under lib/stb
# create an interface target for it
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/lib/stb)

add_subdirectory(builder)
add_subdirectory(validator)