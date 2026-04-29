#include "Core.hpp"

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
    _writeOutput(image);

    auto t3 = Clock::now();
    if (_logging)
        _logger->logTiming(elapsed(t0, t1), elapsed(t1, t2), elapsed(t2, t3),
            static_cast<long long>(_scene.camera().getWidth()) * _scene.camera().getHeight());

    return true;
}

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
        _backgroundImage = Image::readPPM(bgImage);
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
    const auto& rc = _scene.rendererConfig();

    int h = _scene.camera().getHeight();
    int w = _scene.camera().getWidth();
    Image image(w, h);

    int total_threads = 0;
    if (rc.multithreadingEnabled) {
        total_threads = rc.threadCount == 0 ? std::thread::hardware_concurrency() : rc.threadCount;
    }

    int thread_rows= h / total_threads; 

    std::vector<std::thread> threads;
    std::unique_ptr<ProgressBar> progbar = nullptr;
    if (_logging) {
        progbar = std::make_unique<ProgressBar>(h);
    }

    for (int x = 0; x < total_threads; ++x) {
        int first_row = x * thread_rows;
        int last_row = (x == total_threads - 1) ? image.height() : (x + 1) * thread_rows;
        
        threads.emplace_back([this, &image, first_row, last_row, &rc, progbar = progbar.get()]() {
            if (rc.aaEnabled && rc.aaSamples > 1) {
                if (rc.aaMethod == "ssaa") {
                    this->_renderSSAA(image, first_row, last_row, progbar, rc.aaSamples);
                }
            } else {
                this->_renderNoAA(image, first_row, last_row, progbar);
            }
        });
    }

    for (auto& thrd: threads) { thrd.join(); }

    if (_logging && progbar) {
        progbar->finish();
        std::cout << "  Log: " << _logger->path() << std::endl;
    }

    return image;
}

void Core::_renderNoAA(Image &image, int first_row, int last_row, ProgressBar *progbar)
{
    int w = image.width();
    int h = image.height();

    for (int y = first_row; y < last_row; ++y) {
        for (int x = 0; x < w; ++x) {
            float u = static_cast<float>(x) / (w - 1);
            float v = 1.0f - static_cast<float>(y) / (h - 1);
            image.setPixel(x, y, trace(_scene.camera().getRay(u, v), 50, u, v));
        }

        if (progbar != nullptr) {
            progbar->update(1);
        }
    }
}

void Core::_renderSSAA(Image &image, int first_row, int last_row, ProgressBar *progbar, int samples)
{
    int w = _scene.camera().getWidth();
    int h = _scene.camera().getHeight();

    // safe thread rng
    std::mt19937 gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = first_row; y < last_row; ++y) {
        for (int x = 0; x < w; ++x) {
            Vec3 pixelColor(0, 0, 0);
            float baseU = static_cast<float>(x) / (w - 1);
            float baseV = 1.0f - static_cast<float>(y) / (h - 1);
            
            for (int s = 0; s < samples; ++s) {
                float u = (static_cast<float>(x) + dist(gen)) / w;
                float v = 1.0f - (static_cast<float>(y) + dist(gen)) / h;
                pixelColor = pixelColor + trace(_scene.camera().getRay(u, v), 50, baseU, baseV);
            }

            image.setPixel(x, y, (pixelColor / static_cast<double>(samples)));
        }

        if (progbar != nullptr) {
            progbar->update(1);
        }
    }
}

void Core::_writeOutput(Image& image) {
    image.writePPM(_scene.rendererConfig().outputFile);
}

Vec3 Core::trace(const Ray& ray, int depth, double screenU, double screenV) {
    if (depth <= 0) return _sampleBackground(screenU, screenV);

    HitRecord record;
    if (_scene.world().get_closest_hit(ray, _t_min, _t_max, record)) {
        Vec3 color = _scene.rendererConfig().ambientColor * _scene.rendererConfig().ambientMultiplier;

        for (const auto& light : _scene.lights()) {
            Vec3 lightDir, lightColor;
            double lightDistance = light->get_light_data(record.point, lightDir, lightColor);

            Ray shadowRay(record.point + record.normal * 1e-4, lightDir);
            HitRecord shadowRecord;

            Vec3 viewDir = -ray.direction();
            if (!_scene.world().get_closest_hit(shadowRay, _t_min, lightDistance, shadowRecord)) {
                color += record.material->shade(record, lightDir, lightColor, viewDir) *_scene.rendererConfig().diffuseMultiplier;
            }
        }

        Vec3 attenuation;
        Ray scattered;
        if (record.material->scatter(ray, record, attenuation, scattered))
            color += attenuation * trace(scattered, depth - 1, screenU, screenV);

        return color;
    }
    return _sampleBackground(screenU, screenV);
}

Vec3 Core::_sampleBackground(double screenU, double screenV) {
    if (_backgroundImage) {
        return _backgroundImage->sample(screenU, screenV);
    }
    return _scene.rendererConfig().backgroundColor;
}
