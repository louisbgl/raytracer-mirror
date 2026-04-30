cmake_minimum_required(VERSION 3.20)
project(Raytracer C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Common compiler flags
add_library(raytracer_flags INTERFACE)

# Base flags for all builds
target_compile_options(raytracer_flags INTERFACE
    -Wall -Wextra -Werror
)

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    target_compile_options(raytracer_flags INTERFACE
        -g3                    # Full debug symbols
        -O0                    # No optimization (for debugging)
    )
    message(STATUS "Building in Debug mode")
endif()

# Release flags - aggressive optimizations for raytracing performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(raytracer_flags INTERFACE
        -O3                    # Maximum optimization
        -march=native          # Use CPU-specific instructions (SIMD, AVX, etc.)
        -ffast-math           # Aggressive floating-point optimizations
        -funroll-loops        # Unroll loops for performance
        -DNDEBUG              # Disable assertions
    )
    message(STATUS "Building in Release mode with aggressive optimizations")
endif()

include(FetchContent)

# FetchContent-built libraries land in lib/deps/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/deps)

# ── libconfig++ ────────────────────────────────────────────────────────────────
# Try system package first (fast path), fallback to FetchContent
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBCONFIG++ QUIET libconfig++)
endif()

if(NOT LIBCONFIG++_FOUND)
    message(STATUS "libconfig++ not found in system, downloading with FetchContent...")
    FetchContent_Declare(libconfig
        GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git
        GIT_TAG v1.8.2)

    # Disable examples and tests to avoid build issues
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(libconfig)

    set(LIBCONFIG++_FOUND TRUE)
    set(LIBCONFIG++_INCLUDE_DIRS ${libconfig_SOURCE_DIR}/lib ${libconfig_BINARY_DIR}/lib)
    set(LIBCONFIG++_LIBRARIES config++)
    set(LIBCONFIG++_LIBRARY_DIRS "")
else()
    message(STATUS "Found system libconfig++ (${LIBCONFIG++_LIBRARIES})")
    # Ensure library directories are included for system package
    if(NOT LIBCONFIG++_LIBRARY_DIRS)
        if(APPLE)
            set(LIBCONFIG++_LIBRARY_DIRS /opt/homebrew/lib /opt/homebrew/opt/libconfig/lib /usr/local/lib)
        else()
            set(LIBCONFIG++_LIBRARY_DIRS /usr/lib /usr/local/lib)
        endif()
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
    src/core/PluginManager.cpp
    src/core/ProgressBar.cpp
    src/core/Logger.cpp
    src/parsers/SceneParser.cpp
    src/factories/ShapeFactory.cpp
    src/factories/MaterialFactory.cpp
    src/factories/LightFactory.cpp
    src/utils/HelpDisplay.cpp
    src/core/AShape.cpp
    src/Math/Matrix4x4.cpp
)

target_include_directories(raytracer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${LIBCONFIG++_INCLUDE_DIRS}
)

target_link_directories(raytracer PRIVATE ${LIBCONFIG++_LIBRARY_DIRS})

target_link_libraries(raytracer PRIVATE
    raytracer_flags
    ${LIBCONFIG++_LIBRARIES}
)

# Link libdl on Linux (not needed on macOS)
if(NOT APPLE)
    target_link_libraries(raytracer PRIVATE dl)
    target_link_options(raytracer PRIVATE -rdynamic)
endif()

# Set platform-specific plugin extension
if(APPLE)
    target_compile_definitions(raytracer PRIVATE PLUGIN_EXTENSION=".dylib")
else()
    target_compile_definitions(raytracer PRIVATE PLUGIN_EXTENSION=".so")
endif()

# ── Tests ──────────────────────────────────────────────────────────────────────
# Placeholder test executable (update with actual test files when available)
if(EXISTS ${PROJECT_SOURCE_DIR}/tests)
    file(GLOB TEST_SOURCES tests/*.cpp)
    if(TEST_SOURCES)
        add_executable(run_tests EXCLUDE_FROM_ALL
            ${TEST_SOURCES}
            src/Math/Roots3And4.c
            src/parsers/ObjParser.cpp
            src/plugins/Shapes/MeshTriangle.cpp
        )

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
# Track all plugin targets for help generation
set(ALL_PLUGIN_TARGETS "" CACHE INTERNAL "List of all plugin targets")

# Helper function to create plugin targets (accepts multiple source files via ARGN)
function(add_raytracer_plugin PLUGIN_NAME PLUGIN_TYPE SOURCE_FILE)
    add_library(${PLUGIN_NAME} SHARED ${SOURCE_FILE} ${ARGN})

    target_include_directories(${PLUGIN_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    target_link_libraries(${PLUGIN_NAME} PRIVATE
        raytracer_flags
    )

    # Set platform-specific extension
    if(APPLE)
        target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_EXTENSION=".dylib")
    else()
        target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_EXTENSION=".so")
    endif()

    # Set output directory based on plugin type
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/${PLUGIN_TYPE}
        PREFIX ""  # Remove 'lib' prefix from .so/.dylib files
    )

    # Track this plugin for help generation
    set(ALL_PLUGIN_TARGETS ${ALL_PLUGIN_TARGETS} ${PLUGIN_NAME} CACHE INTERNAL "List of all plugin targets")
endfunction()

function(add_ashape_plugin PLUGIN_NAME SOURCE_FILE)
    add_raytracer_plugin(${PLUGIN_NAME} shapes ${SOURCE_FILE} ${ARGN} src/core/AShape.cpp src/Math/Matrix4x4.cpp)
endfunction()

# Disable deprecated K&R style warnings for Graphics Gems code
set_source_files_properties(src/Math/Roots3And4.c PROPERTIES
    COMPILE_FLAGS "-Wno-deprecated-non-prototype"
)

# Shape plugins
# Bounded shapes (inherit from AShape)
add_ashape_plugin(sphere            src/plugins/Shapes/Sphere.cpp)
add_ashape_plugin(box               src/plugins/Shapes/Box.cpp)
add_ashape_plugin(rectangle         src/plugins/Shapes/Rectangle.cpp)
add_ashape_plugin(triangle          src/plugins/Shapes/Triangle.cpp)
add_ashape_plugin(limited_cylinder  src/plugins/Shapes/LimitedCylinder.cpp)
add_ashape_plugin(limited_cone      src/plugins/Shapes/LimitedCone.cpp)
add_ashape_plugin(limited_hourglass src/plugins/Shapes/LimitedHourglass.cpp)
add_ashape_plugin(torus             src/plugins/Shapes/Torus.cpp            src/Math/Roots3And4.c)
add_ashape_plugin(tanglecube        src/plugins/Shapes/Tanglecube.cpp       src/Math/Roots3And4.c)
add_ashape_plugin(mesh
    src/plugins/Shapes/Mesh.cpp
    src/parsers/ObjParser.cpp
    src/plugins/Shapes/MeshTriangle.cpp
)

# Infinite shapes (inherit directly from IShape - no AShape needed)
add_raytracer_plugin(cylinder  shapes src/plugins/Shapes/Cylinder.cpp)
add_raytracer_plugin(plane     shapes src/plugins/Shapes/Plane.cpp)
add_raytracer_plugin(cone      shapes src/plugins/Shapes/Cone.cpp)
add_raytracer_plugin(hourglass shapes src/plugins/Shapes/Hourglass.cpp)

# Material plugins
add_raytracer_plugin(lambertian     materials src/plugins/Materials/Lambertian.cpp)
add_raytracer_plugin(refractive     materials src/plugins/Materials/Refractive.cpp)
add_raytracer_plugin(reflective     materials src/plugins/Materials/Reflective.cpp)
add_raytracer_plugin(coloreddiffuse materials src/plugins/Materials/ColoredDiffuse.cpp)
add_raytracer_plugin(phong          materials src/plugins/Materials/Phong.cpp)
add_raytracer_plugin(chessboard     materials src/plugins/Materials/Chessboard.cpp)
add_raytracer_plugin(perlinnoise    materials src/plugins/Materials/PerlinNoise.cpp)
add_raytracer_plugin(image_texture  materials src/plugins/Materials/ImageTexture.cpp)

# Light plugins
add_raytracer_plugin(pointlight       lights src/plugins/Lights/PointLight.cpp)
add_raytracer_plugin(directionallight lights src/plugins/Lights/DirectionalLight.cpp)

# Create plugin directories
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/shapes)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/materials)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/lights)

# Convenience targets for building groups of plugins
add_custom_target(shapes
    DEPENDS sphere cylinder rectangle box limited_cylinder plane tanglecube
    DEPENDS sphere cylinder limited_cylinder rectangle box plane torus
    DEPENDS cone limited_cone hourglass limited_hourglass 
    COMMENT "Building all shape plugins..."
)

add_custom_target(materials
    DEPENDS lambertian refractive reflective coloreddiffuse phong chessboard perlinnoise image_texture
    COMMENT "Building all material plugins..."
)

add_custom_target(lights
    DEPENDS pointlight directionallight
    COMMENT "Building all light plugins..."
)

add_custom_target(plugins
    DEPENDS shapes materials lights
    COMMENT "Building all plugins..."
)

# ── Help Generator Utility ─────────────────────────────────────────────────────
add_executable(generate_help
    tools/generate_help.cpp
    src/core/PluginManager.cpp
    src/core/PluginLoader.cpp
)

target_include_directories(generate_help PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(generate_help PRIVATE raytracer_flags)

# Link libdl on Linux (not needed on macOS)
if(NOT APPLE)
    target_link_libraries(generate_help PRIVATE dl)
endif()

# Set platform-specific plugin extension
if(APPLE)
    target_compile_definitions(generate_help PRIVATE PLUGIN_EXTENSION=".dylib")
else()
    target_compile_definitions(generate_help PRIVATE PLUGIN_EXTENSION=".so")
endif()

# Custom target that runs generate_help after ALL plugins are fully built
add_custom_target(help_txt ALL
    COMMAND generate_help
    DEPENDS generate_help ${ALL_PLUGIN_TARGETS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating help.txt from plugin metadata"
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/help.txt
)

# Make raytracer depend on help_txt so help.txt is always up-to-date
add_dependencies(raytracer help_txt)

# ── Texture Conversion ─────────────────────────────────────────────────────────
# Try to find ImageMagick (magick or convert command)
find_program(IMAGEMAGICK_CONVERT NAMES magick convert)

if(NOT IMAGEMAGICK_CONVERT)
    # Try to install via package manager as fallback
    message(STATUS "ImageMagick not found in system, attempting to install...")

    include(ExternalProject)

    if(APPLE)
        # Use brew on macOS
        execute_process(
            COMMAND brew install imagemagick
            RESULT_VARIABLE BREW_RESULT
            OUTPUT_QUIET ERROR_QUIET
        )
        find_program(IMAGEMAGICK_CONVERT NAMES magick convert)
    else()
        # Use apt on Linux
        execute_process(
            COMMAND sudo apt-get update
            RESULT_VARIABLE APT_UPDATE_RESULT
            OUTPUT_QUIET ERROR_QUIET
        )
        execute_process(
            COMMAND sudo apt-get install -y imagemagick
            RESULT_VARIABLE APT_RESULT
            OUTPUT_QUIET ERROR_QUIET
        )
        find_program(IMAGEMAGICK_CONVERT NAMES magick convert)
    endif()
endif()

if(IMAGEMAGICK_CONVERT)
    message(STATUS "Found ImageMagick: ${IMAGEMAGICK_CONVERT}")

    if(EXISTS ${PROJECT_SOURCE_DIR}/textures/convert_images.sh)
        add_custom_target(convert_textures
            COMMAND ${CMAKE_COMMAND} -E env PATH=$ENV{PATH}:/usr/bin:/usr/local/bin ${CMAKE_COMMAND} -E chdir ${PROJECT_SOURCE_DIR}/textures ./convert_images.sh
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Converting texture images to PPM format"
        )

        # Run texture conversion before building raytracer
        add_dependencies(raytracer convert_textures)
    endif()
else()
    message(WARNING "ImageMagick could not be found or installed - texture conversion disabled.")
endif()
