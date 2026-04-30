#pragma once

#include "../src/DataTypes/Scene.hpp"
#include "Image.hpp"
#include "core/ProgressBar.hpp"
#include <memory>
#include <span>
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
    Scene _scene;
    std::unique_ptr<Image> _backgroundImage;
    std::unique_ptr<Logger> _logger;
    double _t_min = 0.001;
    double _t_max = 1e6;
    int _maxRayBounces = 50;
    int _maxSubdivDepth = 3;

    bool  _loadScene();
    Image _render();
    void  _writeOutput(Image& image);

    Vec3 _computePixelColor(int x, int y, int width, int height) const;
    Vec3 _computePixelColorSSAA(int x, int y, int width, int height, int samples) const;
    Vec3 _computePixelColorAdaptiveSSAA(int x, int y, int width, int height, double threshold) const;

    Vec3 _trace(const Ray& ray, int depth, double screenU, double screenV) const;

    Vec3 _sampleBackground(double screenU, double screenV) const;
    Vec3 _sampleSubPixel(int x, int y, double offX, double offY, int width, int height) const;
    Vec3 _adaptiveSubdivide(int x, int y, double offX, double offY, double size, int width, int height, double threshold, int depth) const;

    double _computeVariance(std::span<const Vec3> colors) const;
};