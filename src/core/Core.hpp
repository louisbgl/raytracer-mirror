#pragma once

#include "../src/DataTypes/Scene.hpp"
#include "Image.hpp"
#include <memory>
#include <string>

class Logger;

class Core {
public:
    Core(std::string inputFile, bool logging = false);
    ~Core();

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
    bool _logging;
    std::unique_ptr<Logger> _logger;
    Scene _scene;
    double _t_min = 0.001;
    double _t_max = 1000.0;
    Vec3 _baseAmbient = Vec3(25, 25, 38);
    Vec3 _backgroundColor = Vec3(135, 206, 235);
    std::string _outputFile = "output.ppm";

    bool  _loadScene();
    Image _render();
    Image _renderNoAA();
    Image _renderSSAA(int samples);
    void  _writeOutput(Image& image);

    Vec3 trace(const Ray& ray, const Scene& scene, int depth);
};