#include "Core.hpp"

#include <chrono>
#include <iostream>
#include <memory>

#include "../parsers/SceneParser.hpp"
#include "Logger.hpp"
#include "ProgressBar.hpp"

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

    if (_logging) {
        _logger = std::make_unique<Logger>();
        _logger->logScene(_inputFile, _scene);
    }
    return true;
}

Image Core::_render() {
    int width  = _scene.camera().getWidth();
    int height = _scene.camera().getHeight();
    Image image(width, height);

    std::unique_ptr<ProgressBar> pb;
    if (_logging)
        pb = std::make_unique<ProgressBar>(height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / (width - 1);
            float v = 1.0f - static_cast<float>(y) / (height - 1);
            image.setPixel(x, y, trace(_scene.camera().getRay(u, v), _scene, 50));
        }
        if (_logging)
            pb->update(y + 1);
    }

    if (_logging) {
        pb->finish();
        std::cout << "  Log: " << _logger->path() << std::endl;
    }
    return image;
}

void Core::_writeOutput(Image& image) {
    image.writePPM(_outputFile);
}

Vec3 Core::trace(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) return _backgroundColor;

    HitRecord record;
    if (scene.world().get_closest_hit(ray, _t_min, _t_max, record)) {
        Vec3 color = _baseAmbient * scene.ambientMultiplier();

        for (const auto& light : scene.lights()) {
            Vec3 lightDir, lightColor;
            double lightDistance = light->get_light_data(record.point, lightDir, lightColor);

            Ray shadowRay(record.point, lightDir);
            HitRecord shadowRecord;

            Vec3 viewDir = -ray.direction();
            if (!scene.world().get_closest_hit(shadowRay, _t_min, lightDistance, shadowRecord)) {
                color += record.material->shade(record, lightDir, lightColor, viewDir) *
                         scene.diffuseMultiplier();
            }
        }

        Vec3 attenuation;
        Ray scattered;
        if (record.material->scatter(ray, record, attenuation, scattered))
            color += attenuation * trace(scattered, scene, depth - 1);

        return color;
    }
    return _backgroundColor;
}
