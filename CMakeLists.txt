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
    src/plugins/Shapes/Sphere.cpp           # TODO: REMOVE
    src/plugins/Materials/Lambertian.cpp    # TODO: REMOVE
    src/plugins/Lights/PointLight.cpp       # TODO: REMOVE
)

target_include_directories(raytracer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(raytracer PRIVATE
    raytracer_flags
)

# ── Tests ──────────────────────────────────────────────────────────────────────
# Placeholder test executable (update with actual test files when available)
if(EXISTS ${PROJECT_SOURCE_DIR}/tests)
    file(GLOB TEST_SOURCES tests/*.cpp)
    if(TEST_SOURCES)
        add_executable(run_tests ${TEST_SOURCES})

        target_include_directories(run_tests PRIVATE
            ${PROJECT_SOURCE_DIR}
            ${_criterion_include}
        )

        target_link_libraries(run_tests PRIVATE
            ${_criterion_target}
            raytracer_flags
        )

        add_custom_target(tests_run
            COMMAND $<TARGET_FILE:run_tests> --xml=test-results.xml || true
            DEPENDS run_tests
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Running Criterion tests..."
        )
    endif()
endif()
