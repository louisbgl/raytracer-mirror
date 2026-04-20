#include "Core.hpp"
#include "../parsers/SceneParser.hpp"
#include <iostream>

bool Core::simulate() {
    // Load the scene from the input file
    try {
        SceneParser parser;
        _scene = parser.parse(_inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load scene: " << e.what() << std::endl;
        return false;
    }

    // Scene should now be setup correctly with world, camera, and lights
    // Render loop

    // Output ppm file
    return true;
}

Vec3 Core::trace(const Ray& ray, const Scene& scene, int depth) {
    (void)ray; // Avoid unused parameter warning
    (void)scene; // Avoid unused parameter warning
    (void)depth; // Avoid unused parameter warning
    return Vec3(0, 0, 0);
}