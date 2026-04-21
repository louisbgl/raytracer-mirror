cmake_minimum_required(VERSION 3.20)
project(Raytracer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Common compiler flags
add_library(raytracer_flags INTERFACE)
target_compile_options(raytracer_flags INTERFACE
    -Wall -Wextra -Werror -g3
)

include(FetchContent)

# FetchContent-built libraries land in lib/deps/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/deps)

# ── libconfig++ ────────────────────────────────────────────────────────────────
# Try system package first, fallback to manual find
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBCONFIG++ libconfig++)
endif()

if(NOT LIBCONFIG++_FOUND)
    # Try to find it manually
    find_path(LIBCONFIG++_INCLUDE_DIR libconfig.h++ PATHS /usr/include /usr/local/include)
    find_library(LIBCONFIG++_LIBRARY NAMES config++ PATHS /usr/lib /usr/local/lib)

    if(LIBCONFIG++_INCLUDE_DIR AND LIBCONFIG++_LIBRARY)
        set(LIBCONFIG++_FOUND TRUE)
        set(LIBCONFIG++_INCLUDE_DIRS ${LIBCONFIG++_INCLUDE_DIR})
        set(LIBCONFIG++_LIBRARIES ${LIBCONFIG++_LIBRARY})
    else()
        message(FATAL_ERROR "libconfig++ not found. Please install it (e.g., sudo apt install libconfig++-dev)")
    endif()
endif()

# ── Criterion ──────────────────────────────────────────────────────────────────
# Try system package first, fallback to FetchContent
find_library(CRITERION_LIB NAMES criterion)
if(CRITERION_LIB)
    set(_criterion_target ${CRITERION_LIB})
else()
    FetchContent_Declare(Criterion
        GIT_REPOSITORY  https://github.com/Snaipe/Criterion.git
        GIT_TAG         v2.4.2)
    FetchContent_MakeAvailable(Criterion)
    set(_criterion_target criterion)
    set(_criterion_include ${criterion_SOURCE_DIR}/include)
endif()

# Reset output directory for main executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# ── Main raytracer executable ──────────────────────────────────────────────────
add_executable(raytracer
    src/main.cpp
    src/core/Image.cpp
    src/core/Core.cpp
    src/core/PluginLoader.cpp
    src/parsers/SceneParser.cpp
    src/factories/ShapeFactory.cpp
    src/factories/MaterialFactory.cpp
    src/factories/LightFactory.cpp
)

target_include_directories(raytracer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${LIBCONFIG++_INCLUDE_DIRS}
)

target_link_libraries(raytracer PRIVATE
    raytracer_flags
    ${LIBCONFIG++_LIBRARIES}
    dl  # For plugin loading (libdl)
)

# ── Tests ──────────────────────────────────────────────────────────────────────
# Placeholder test executable (update with actual test files when available)
if(EXISTS ${PROJECT_SOURCE_DIR}/tests)
    file(GLOB TEST_SOURCES tests/*.cpp)
    if(TEST_SOURCES)
        add_executable(run_tests EXCLUDE_FROM_ALL ${TEST_SOURCES})

        target_include_directories(run_tests PRIVATE
            ${PROJECT_SOURCE_DIR}
            ${_criterion_include}
        )

        target_link_libraries(run_tests PRIVATE
            ${_criterion_target}
            raytracer_flags
        )

        add_custom_target(tests_run
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/tests/results
            COMMAND $<TARGET_FILE:run_tests> --xml=tests/results/test-results.xml || true
            DEPENDS run_tests
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Running Criterion tests..."
        )
    endif()
endif()

# ── Plugins ────────────────────────────────────────────────────────────────────
# Helper function to create plugin targets
function(add_raytracer_plugin PLUGIN_NAME PLUGIN_TYPE SOURCE_FILE)
    add_library(${PLUGIN_NAME} SHARED ${SOURCE_FILE})

    target_include_directories(${PLUGIN_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    target_link_libraries(${PLUGIN_NAME} PRIVATE
        raytracer_flags
    )

    # Set output directory based on plugin type
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/${PLUGIN_TYPE}
        PREFIX ""  # Remove 'lib' prefix from .so files
    )
endfunction()

# Shape plugins
add_raytracer_plugin(sphere shapes src/plugins/Shapes/Sphere.cpp)

# Material plugins
add_raytracer_plugin(lambertian materials src/plugins/Materials/Lambertian.cpp)
add_raytracer_plugin(transparent materials src/plugins/Materials/Transparent.cpp)

# Light plugins
add_raytracer_plugin(pointlight lights src/plugins/Lights/PointLight.cpp)

# Create plugin directories
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/shapes)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/materials)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/lights)

# Convenience targets for building groups of plugins
add_custom_target(shapes
    DEPENDS sphere
    COMMENT "Building all shape plugins..."
)

add_custom_target(materials
    DEPENDS lambertian
    COMMENT "Building all material plugins..."
)

add_custom_target(lights
    DEPENDS pointlight
    COMMENT "Building all light plugins..."
)

add_custom_target(plugins
    DEPENDS shapes materials lights
    COMMENT "Building all plugins..."
)
