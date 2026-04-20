#pragma once

#include "../src/DataTypes/Scene.hpp"
#include <string>

class Core {
public:
    Core(std::string inputFile) : _inputFile(std::move(inputFile)) {}
    ~Core() = default;

    /**
     * @brief Simulates the ray tracing process and renders the scene.
     * It first load the scene from the input file
     * then it performs ray tracing.
     * @return True if the simulation was successful, false otherwise.
     * @note The output image is saved as "output.ppm".
     */
    bool simulate();

private:
    std::string _inputFile;
    Scene _scene;
    double _t_min = 0.001;
    double _t_max = 1000.0;
    Vec3 _baseAmbient = Vec3(0.1, 0.1, 0.15); // Slight blue
    Vec3 _backgroundColor = Vec3(0, 0, 0); // Black
    std::string _outputFile = "output.ppm";

    Vec3 trace(const Ray& ray, const Scene& scene, int depth);
};