#pragma once

#include "../src/DataTypes/Scene.hpp"
#include "../src/DataTypes/RenderStats.hpp"
#include "Image.hpp"
#include "RenderSampler.hpp"
#include "core/ProgressBar.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
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

    // UI mode hooks — set before simulate(), no-op in CLI mode
    void setCancelFlag(std::atomic<bool>* flag)  { _cancelFlag = flag; }
    void setThreadOverride(int n)                { _threadOverride = n; }
    void setProgressTarget(std::atomic<int>* rows, std::atomic<int>* total) {
        _progressRows  = rows;
        _progressTotal = total;
    }

    void setPreviewMode(float resScale = 0.25f, int maxBounces = 3) {
        _previewMode = true;
        _previewResScale = resScale;
        _previewMaxBounces = maxBounces;
    }
    void setRowCallback(std::function<void(int y, const uint8_t* rgba, int width)> cb) {
        _rowCallback = std::move(cb);
    }
    void setDimensionsCallback(std::function<void(int w, int h)> cb) {
        _dimensionsCallback = std::move(cb);
    }

private:
    std::string _inputFile;
    bool _logging;
    Scene _scene;
    std::unique_ptr<Image> _backgroundImage;
    std::unique_ptr<Logger> _logger;
    mutable RenderStats _stats;
    double _t_min = 0.001;
    double _t_max = 1e6;
    int _maxRayBounces = 50;
    int _maxSubdivDepth = 2;

    // UI hooks (all null in CLI mode)
    std::atomic<bool>* _cancelFlag     = nullptr;
    std::atomic<int>*  _progressRows   = nullptr;
    std::atomic<int>*  _progressTotal  = nullptr;
    int                _threadOverride = 0;

    // Preview mode
    bool _previewMode = false;
    float _previewResScale = 0.25f;
    int _previewMaxBounces = 3;
    std::function<void(int y, const uint8_t* rgba, int width)> _rowCallback;
    std::function<void(int w, int h)> _dimensionsCallback;

    bool  _loadScene();
    Image _render();
    void  _writeOutput(Image& image);

    Vec3 _computePixelColor(int x, int y, int width, int height) const;
    Vec3 _computePixelColorSSAA(int x, int y, int width, int height, int samples) const;
    Vec3 _computePixelColorAdaptiveSSAA(int x, int y, int width, int height, double threshold) const;

    Vec3 _trace(const Ray& ray, int depth, double screenU, double screenV) const;
    Vec3 _computeAmbient(const Ray& ray, const HitRecord& record) const;
    Vec3 _computeLighting(const Ray& ray, const HitRecord& record) const;

    Vec3 _sampleBackground(double screenU, double screenV) const;
    Vec3 _sampleSubPixel(int x, int y, double offX, double offY, int width, int height) const;
    Vec3 _adaptiveSubdivide(int x, int y, double offX, double offY, double size, int width, int height, double threshold, int depth) const;

    int _getTotalThreads(const RendererConfig& rc) const;
    std::function<Vec3(int, int)> _getComputePixelLambda(const RendererConfig& rc, int width, int height) const;
};