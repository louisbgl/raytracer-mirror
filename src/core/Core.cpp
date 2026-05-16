#include "Core.hpp"
#include "RenderSampler.hpp"

#include <algorithm>
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
    if (!cancelled && !_previewMode)
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

int         Core::sceneWidth()  const { return _scene.camera().getWidth(); }
int         Core::sceneHeight() const { return _scene.camera().getHeight(); }
std::string Core::outputFile()  const { return _scene.rendererConfig().outputFile; }
Camera      Core::getCamera()   const { return _scene.camera(); }

bool Core::_loadScene() {
    try {
        SceneParser parser;
        _scene = parser.parse(_inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load scene: " << e.what() << std::endl;
        return false;
    }

    // Apply camera override if set (for free-roam mode)
    if (_cameraOverride) {
        _scene.setCamera(*_cameraOverride);
    }

    if (_previewMode) {
        static constexpr int PREVIEW_MAX_DIM = 480;
        auto& cam = _scene.camera();
        int pw = std::max(1, static_cast<int>(cam.getWidth()  * _previewResScale));
        int ph = std::max(1, static_cast<int>(cam.getHeight() * _previewResScale));
        int longest = std::max(pw, ph);
        if (longest > PREVIEW_MAX_DIM) {
            float cap = static_cast<float>(PREVIEW_MAX_DIM) / static_cast<float>(longest);
            pw = std::max(1, static_cast<int>(pw * cap));
            ph = std::max(1, static_cast<int>(ph * cap));
        }
        cam.setWidth(pw);
        cam.setHeight(ph);
        _maxRayBounces = _previewMaxBounces;
        auto cfg = _scene.rendererConfig();
        cfg.aaEnabled = false;
        cfg.aoEnabled = false;
        _scene.setRendererConfig(cfg);
    }

    if (_dimensionsCallback)
        _dimensionsCallback(static_cast<int>(_scene.camera().getWidth()),
                            static_cast<int>(_scene.camera().getHeight()));

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

Image Core::_render()
{
    const RendererConfig rc = _scene.rendererConfig();
    int h = _scene.camera().getHeight();
    int w = _scene.camera().getWidth();
    Image image(w, h);

    int total_threads = _threadOverride > 0 ? _threadOverride : _getTotalThreads(rc);
    if (_progressTotal) _progressTotal->store(h);

    int thread_rows = h / total_threads;

    std::function<Vec3(int, int)> computePixel = _getComputePixelLambda(rc, w, h);

    std::vector<std::thread> threads;
    std::unique_ptr<ProgressBar> progbar = nullptr;
    if (_logging)
        progbar = std::make_unique<ProgressBar>(h);

    for (int t = 0; t < total_threads; ++t) {
        int first_row = t * thread_rows;
        int last_row  = (t == total_threads - 1) ? h : (t + 1) * thread_rows;

        threads.emplace_back([this, &image, &computePixel, first_row, last_row, w, progbar = progbar.get()]() {
            thread_local std::vector<uint8_t> rowBuf;
            for (int y = first_row; y < last_row; ++y) {
                if (_cancelFlag && _cancelFlag->load(std::memory_order_relaxed))
                    return;
                for (int x = 0; x < w; ++x)
                    image.setPixel(x, y, computePixel(x, y));
                if (_rowCallback) {
                    rowBuf.resize(w * 4);
                    for (int x = 0; x < w; ++x) {
                        Vec3 c = image.getPixel(x, y);
                        rowBuf[x * 4 + 0] = static_cast<uint8_t>(std::clamp(c.x(), 0.0, 255.0));
                        rowBuf[x * 4 + 1] = static_cast<uint8_t>(std::clamp(c.y(), 0.0, 255.0));
                        rowBuf[x * 4 + 2] = static_cast<uint8_t>(std::clamp(c.z(), 0.0, 255.0));
                        rowBuf[x * 4 + 3] = 255;
                    }
                    _rowCallback(y, rowBuf.data(), w);
                }
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

    return image;
}

Image Core::renderSlice(int firstRow, int lastRow)
{
    if (!_loadScene())
        throw std::runtime_error("renderSlice: failed to load scene");

    const RendererConfig rc = _scene.rendererConfig();
    int w = _scene.camera().getWidth();
    int h = _scene.camera().getHeight();
    int rowCount = lastRow - firstRow;
    Image slice(w, rowCount);

    int total_threads = _threadOverride > 0 ? _threadOverride : _getTotalThreads(rc);
    if (_progressTotal) _progressTotal->store(rowCount);

    int thread_rows = rowCount / total_threads;

    std::function<Vec3(int, int)> computePixel = _getComputePixelLambda(rc, w, h);

    std::vector<std::thread> threads;
    for (int t = 0; t < total_threads; ++t) {
        int first = firstRow + t * thread_rows;
        int last  = (t == total_threads - 1) ? lastRow : firstRow + (t + 1) * thread_rows;

        threads.emplace_back([this, &slice, &computePixel, first, last, firstRow, w]() {
            for (int y = first; y < last; ++y) {
                if (_cancelFlag && _cancelFlag->load(std::memory_order_relaxed))
                    return;
                for (int x = 0; x < w; ++x)
                    slice.setPixel(x, y - firstRow, computePixel(x, y));
                if (_progressRows)
                    _progressRows->fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& thrd : threads)
        thrd.join();

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
        HitRecord shadowRecord(Vec3(0, 0, 0), Vec3(0, 0, 0), 0.0, true, nullptr);

        double t = _t_min;
        Vec3 transmittance(1, 1, 1);
        static constexpr double epsilon = 1e-4;

        bool fullyOccluded = false;
        while (t < lightDistance) {
            _stats.shadowRaysCast++;
            if (!_scene.world().get_closest_hit(shadowRay, t, lightDistance, shadowRecord))
                break;

            if (!shadowRecord.front_face) {
                t = shadowRecord.t + epsilon;
                continue;
            }

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
        _adaptiveSubdivide(x, y, offX,         offY,        half, width, height, threshold, depth + 1) +
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

void Core::setCancelFlag(std::atomic<bool>* flag) {
    _cancelFlag = flag;
}

void Core::setThreadOverride(int n) {
    _threadOverride = n;
}

void Core::setProgressTarget(std::atomic<int>* rows, std::atomic<int>* total) {
    _progressRows  = rows;
    _progressTotal = total;
}

void Core::setPreviewMode(float resScale, int maxBounces) {
    _previewMode = true;
    _previewResScale = resScale;
    _previewMaxBounces = maxBounces;
}

void Core::setRowCallback(std::function<void(int y, const uint8_t* rgba, int width)> cb) {
    _rowCallback = std::move(cb);
}

void Core::setDimensionsCallback(std::function<void(int w, int h)> cb) {
    _dimensionsCallback = std::move(cb);
}

void Core::setCameraOverride(const Camera& camera) {
    _cameraOverride = camera;
}