#include "Core.hpp"
#include "RenderSampler.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <cmath>
#include <thread>
#include <vector>

#include "../parsers/SceneParser.hpp"
#include "../Math/Constants.hpp"
#include "DataTypes/Vec3.hpp"
#include "Logger.hpp"
#include "ProgressBar.hpp"
#include "core/Image.hpp"

Core::Core(std::string inputFile, bool logging)
    : _inputFile(std::move(inputFile)), _logging(logging) {}

Core::~Core() = default;

bool Core::simulate() {
    using Clock = std::chrono::steady_clock;
    auto elapsed = [](auto a, auto b) {
        return std::chrono::duration<double>(b - a).count();
    };

    auto t0 = Clock::now();
    if (!_loadScene()) return false;

    auto t1 = Clock::now();
    Image image = _render();

    auto t2 = Clock::now();
    bool cancelled = _cancelFlag && _cancelFlag->load(std::memory_order_relaxed);
    if (!cancelled)
        _writeOutput(image);

    auto t3 = Clock::now();
    if (_logging) {
        _logger->logTiming(elapsed(t0, t1), elapsed(t1, t2), elapsed(t2, t3),
            static_cast<long long>(_scene.camera().getWidth()) * _scene.camera().getHeight());
        _logger->logStats(_stats, _scene.rendererConfig());
    }

    return true;
}

bool Core::loadScene() { return _loadScene(); }

int Core::sceneWidth()  const { return _scene.camera().getWidth(); }
int Core::sceneHeight() const { return _scene.camera().getHeight(); }

bool Core::_loadScene() {
    try {
        SceneParser parser;
        _scene = parser.parse(_inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load scene: " << e.what() << std::endl;
        return false;
    }

    const auto& bgImage = _scene.rendererConfig().backgroundImage;
    if (!bgImage.empty()) {
        _backgroundImage = Image::readFile(bgImage);
        if (!_backgroundImage) {
            std::cerr << "Warning: Failed to load background image, using solid color" << std::endl;
        }
    }

    if (_logging) {
        _logger = std::make_unique<Logger>();
        _logger->logScene(_inputFile, _scene);
    }
    return true;
}

void Core::_renderRows(Image& image, int firstRow, int lastRow, int rowOffset)
{
    const RendererConfig rc = _scene.rendererConfig();
    int w = _scene.camera().getWidth();
    int h = _scene.camera().getHeight();
    int rowCount = lastRow - firstRow;

    int total_threads = _threadOverride > 0 ? _threadOverride : _getTotalThreads(rc);
    if (_progressTotal) _progressTotal->store(rowCount);

    int thread_rows = rowCount / total_threads;

    std::function<Vec3(int, int)> computePixel = _getComputePixelLambda(rc, w, h);

    std::vector<std::thread> threads;
    std::unique_ptr<ProgressBar> progbar = nullptr;
    if (_logging)
        progbar = std::make_unique<ProgressBar>(rowCount);

    for (int t = 0; t < total_threads; ++t) {
        int first = firstRow + t * thread_rows;
        int last  = (t == total_threads - 1) ? lastRow : firstRow + (t + 1) * thread_rows;

        threads.emplace_back([this, &image, &computePixel, first, last, w, rowOffset, progbar = progbar.get()]() {
            for (int y = first; y < last; ++y) {
                if (_cancelFlag && _cancelFlag->load(std::memory_order_relaxed))
                    return;
                for (int x = 0; x < w; ++x)
                    image.setPixel(x, y - rowOffset, computePixel(x, y));
                if (_progressRows)
                    _progressRows->fetch_add(1, std::memory_order_relaxed);
                if (progbar)
                    progbar->update(1);
            }
        });
    }

    for (auto& thrd : threads)
        thrd.join();

    if (_logging && progbar) {
        progbar->finish();
        std::cout << "  Log: " << _logger->path() << std::endl;
    }
}

Image Core::_render()
{
    int h = _scene.camera().getHeight();
    int w = _scene.camera().getWidth();
    Image image(w, h);
    _renderRows(image, 0, h);
    return image;
}

Image Core::renderSlice(int firstRow, int lastRow)
{
    if (!_loadScene())
        throw std::runtime_error("renderSlice: failed to load scene");

    int w = _scene.camera().getWidth();
    // Allocate only the rows we need — pixels stored at y=0..(lastRow-firstRow-1)
    Image slice(w, lastRow - firstRow);
    _renderRows(slice, firstRow, lastRow, firstRow);
    return slice;
}

void Core::_writeOutput(Image& image) {
    image.writeFile(_scene.rendererConfig().outputFile);
}

Vec3 Core::_computePixelColor(int x, int y, int width, int height) const {
    float u = static_cast<float>(x) / (width - 1);
    float v = 1.0f - static_cast<float>(y) / (height - 1);
    return _trace(_scene.camera().getRay(u, v), _maxRayBounces, u, v);
}

Vec3 Core::_computePixelColorSSAA(int x, int y, int width, int height, int samples) const {
    thread_local std::mt19937 gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    Vec3 pixelColor(0, 0, 0);
    float baseU = static_cast<float>(x) / (width - 1);
    float baseV = 1.0f - static_cast<float>(y) / (height - 1);

    for (int s = 0; s < samples; ++s) {
        float u = (static_cast<float>(x) + dist(gen)) / width;
        float v = 1.0f - (static_cast<float>(y) + dist(gen)) / height;
        pixelColor = pixelColor + _trace(_scene.camera().getRay(u, v), _maxRayBounces, baseU, baseV);
    }
    _stats.ssaaSamples += samples;

    return pixelColor / static_cast<double>(samples);
}

Vec3 Core::_computePixelColorAdaptiveSSAA(int x, int y, int width, int height, double threshold) const {
    return _adaptiveSubdivide(x, y, 0.0, 0.0, 1.0, width, height, threshold, 0);
}

Vec3 Core::_trace(const Ray& ray, int depth, double screenU, double screenV) const {
    if (depth <= 0) return _sampleBackground(screenU, screenV);

    HitRecord record;
    if (!_scene.world().get_closest_hit(ray, _t_min, _t_max, record))
        return _sampleBackground(screenU, screenV);

    Vec3 color = _computeAmbient(ray, record) + _computeLighting(ray, record);

    Vec3 attenuation;
    Ray scattered;
    if (record.material->scatter(ray, record, attenuation, scattered)) {
        _stats.scatterBounces++;
        color += attenuation * _trace(scattered, depth - 1, screenU, screenV);
    }

    return color;
}

Vec3 Core::_computeAmbient(const Ray& ray, const HitRecord& record) const {
    const auto& rc = _scene.rendererConfig();

    double occlusion = 1.0;
    if (rc.aoEnabled) {
        int hits = 0;
        for (int i = 0; i < rc.aoSamples; ++i) {
            Vec3 aoDir = RenderSampler::randomHemisphere(record.normal);
            Ray aoRay(record.point + record.normal * 1e-4, aoDir);
            HitRecord aoRecord;
            if (_scene.world().get_closest_hit(aoRay, _t_min, rc.aoRadius, aoRecord))
                ++hits;
        }
        _stats.aoRaysCast += rc.aoSamples;
        _stats.aoRaysHit  += hits;
        occlusion = 1.0 - static_cast<double>(hits) / rc.aoSamples;
    }

    Vec3 viewDir = normalize(-ray.direction());
    return record.material->shade(record, record.normal, rc.ambientColor * rc.ambientMultiplier, viewDir) * occlusion;
}

Vec3 Core::_computeLighting(const Ray& ray, const HitRecord& record) const {
    const auto& rc = _scene.rendererConfig();
    Vec3 color(0, 0, 0);
    Vec3 viewDir = -ray.direction();

    for (const auto& light : _scene.lights()) {
        Vec3 lightDir, lightColor;
        double lightDistance = light->get_light_data(record.point, lightDir, lightColor);

        Ray shadowRay(record.point + record.normal * 1e-4, lightDir);
        HitRecord shadowRecord;

        double t = _t_min;
        Vec3 transmittance(1, 1, 1);
        static constexpr double epsilon = 1e-4;

        bool fullyOccluded = false;
        while (t < lightDistance) {
            _stats.shadowRaysCast++;
            if (!_scene.world().get_closest_hit(shadowRay, t, lightDistance, shadowRecord))
                break;

            transmittance *= shadowRecord.material->shadowTransmittance();

            if (length(transmittance) < epsilon) {
                transmittance = Vec3(0, 0, 0);
                fullyOccluded = true;
                break;
            }

            t = shadowRecord.t + epsilon;
        }
        if (fullyOccluded)
            _stats.shadowRaysHit++;

        color += record.material->shade(record, lightDir, lightColor, viewDir) * rc.diffuseMultiplier * transmittance;
    }

    return color;
}

Vec3 Core::_sampleBackground(double screenU, double screenV) const {
    if (_backgroundImage) {
        return _backgroundImage->sample(screenU, screenV);
    }
    return _scene.rendererConfig().backgroundColor;
}

Vec3 Core::_sampleSubPixel(int x, int y, double offX, double offY, int width, int height) const {
    double subU = (static_cast<double>(x) + offX) / width;
    double subV = 1.0 - (static_cast<double>(y) + offY) / height;
    return _trace(_scene.camera().getRay(subU, subV), _maxRayBounces, subU, subV);
}

Vec3 Core::_adaptiveSubdivide(int x, int y, double offX, double offY, double size, int width, int height, double threshold, int depth) const {
    double half = size / 2.0;
    std::array<Vec3, 4> corners = {
        _sampleSubPixel(x, y, offX,         offY,        width, height),
        _sampleSubPixel(x, y, offX + size,   offY,        width, height),
        _sampleSubPixel(x, y, offX,          offY + size, width, height),
        _sampleSubPixel(x, y, offX + size,   offY + size, width, height),
    };

    if (depth >= _maxSubdivDepth || RenderSampler::computeVariance(corners) <= threshold) {
        _stats.adaptiveSamples    += 4;
        _stats.adaptiveMaxSamples += 4 * (1LL << (2 * (_maxSubdivDepth - depth)));
        return (corners[0] + corners[1] + corners[2] + corners[3]) / 4.0;
    }

    return (
        _adaptiveSubdivide(x, y, offX,        offY,        half, width, height, threshold, depth + 1) +
        _adaptiveSubdivide(x, y, offX + half,  offY,        half, width, height, threshold, depth + 1) +
        _adaptiveSubdivide(x, y, offX,         offY + half, half, width, height, threshold, depth + 1) +
        _adaptiveSubdivide(x, y, offX + half,  offY + half, half, width, height, threshold, depth + 1)
    ) / 4.0;
}

int Core::_getTotalThreads(const RendererConfig& rc) const {
    if (!rc.multithreadingEnabled)
        return 1;
    int maxThreads = std::thread::hardware_concurrency();
    if (rc.threadCount < 0)
        return maxThreads;
    if (rc.threadCount > maxThreads)
        return maxThreads;
    return rc.threadCount == 0 ? maxThreads : rc.threadCount;
}

std::function<Vec3(int, int)> Core::_getComputePixelLambda(const RendererConfig& rc, int width, int height) const {
    auto toneMap = [tmEnabled = rc.toneMappingEnabled, tmStrength = rc.toneMappingStrength](Vec3 c) -> Vec3 {
        return tmEnabled ? RenderSampler::toneMapACES(c, tmStrength) : c;
    };
    if (rc.aaEnabled && rc.aaSamples > 1 && rc.aaMethod == "ssaa") {
        return [this, w = width, h = height, samples = rc.aaSamples, toneMap](int x, int y) {
            return toneMap(_computePixelColorSSAA(x, y, w, h, samples));
        };
    } else if (rc.aaEnabled && rc.aaMethod == "adaptive") {
        return [this, w = width, h = height, threshold = rc.aaThreshold, toneMap](int x, int y) {
            return toneMap(_computePixelColorAdaptiveSSAA(x, y, w, h, threshold));
        };
    } else {
        return [this, w = width, h = height, toneMap](int x, int y) {
            return toneMap(_computePixelColor(x, y, w, h));
        };
    }
}