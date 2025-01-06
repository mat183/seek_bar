# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project Name
project(seekBarApp)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# additional settings could be defined here: clang tidy etc.

# Skia paths
set(SKIA_BUILD_DIR ${SKIA_DIR}${SKIA_BUILD})

message(STATUS "SKIA_DIR: ${SKIA_DIR}")
message(STATUS "SKIA_BUILD_DIR: ${SKIA_BUILD_DIR}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall        # Enable common warnings
        -Wextra      # Enable extra warnings
        -Wpedantic   # Strict standard compliance
        # I commented out other options below because of huge amount of warnings that comes from skia library
        # -Wshadow     # Warn about shadowed variables
        # -Wnon-virtual-dtor  # Warn about non-virtual destructors
        # -Wold-style-cast    # Warn about C-style casts
        # -Wcast-align        # Warn about potential alignment issues
        # -Wconversion        # Warn about implicit conversions
        # -Wunused            # Warn about unused variables
        # -Wsign-conversion   # Warn about signed/unsigned conversions
        # -Wdouble-promotion  # Warn about implicit float-to-double promotions
    )
    add_link_options(
        -Wl,--no-undefined  # Fail if there are undefined symbols
    )

    # Enable sanitizers in debug builds
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fno-rtti -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()

    # Enable LTO in release builds
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endif()

find_package(OpenGL REQUIRED)

# Build static libraries
add_library(imageprovider STATIC
    src/image_provider.cpp
)
add_library(seekbar STATIC
    src/seek_bar.cpp
)
add_library(utils STATIC
    src/utils.cpp
)

# Add include & link directories for imageprovider lib
target_include_directories(imageprovider PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SKIA_DIR}
)
target_link_directories(imageprovider PUBLIC
    ${SKIA_BUILD_DIR}
)
# Link necessary dependencies to imageprovider lib
target_link_libraries(imageprovider PUBLIC
    skia
    png
    z
    freetype
    jpeg
    webp
    webpmux
    webpdemux
    fontconfig
)

# Add include & link directories for seekbar lib
target_include_directories(seekbar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SKIA_DIR}
)
target_link_directories(seekbar PUBLIC
    ${SKIA_BUILD_DIR}
)
# Link necessary dependencies to seekbar library
target_link_libraries(seekbar PRIVATE
    imageprovider
    utils
)

# Build the executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link created static libraries and other deps to executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    seekbar
    pthread
    glfw
    GL
)
