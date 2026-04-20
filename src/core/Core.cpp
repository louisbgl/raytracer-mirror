#include "Core.hpp"
#include "../parsers/SceneParser.hpp"
#include "Image.hpp"
#include <iostream>

bool Core::simulate() {
    try {
        SceneParser parser;
        _scene = parser.parse(_inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load scene: " << e.what() << std::endl;
        return false;
    }

    int max_depth = 50;
    int width = _scene.camera().getWidth();
    int height = _scene.camera().getHeight();
    Image image(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / (width - 1);
            float v = 1.0f - static_cast<float>(y) / (height - 1);

            Ray ray = _scene.camera().getRay(u, v);
            Vec3 color = trace(ray, _scene, max_depth);
            image.setPixel(x, y, color);
        }
    }
    image.writePPM(_outputFile);
    return true;
}

Vec3 Core::trace(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) return Vec3(0, 0, 0);

    HitRecord record;
    if (scene.world().get_closest_hit(ray, _t_min, _t_max, record)) {
        Vec3 color = _baseAmbient * scene.ambientMultiplier();

        for (const auto& light : scene.lights()) {
            Vec3 lightDir, lightColor;
            double lightDistance = light->get_light_data(record.point, lightDir, lightColor);

            Ray shadowRay(record.point, lightDir);
            HitRecord shadowRecord;

            if (!scene.world().get_closest_hit(shadowRay, _t_min, lightDistance, shadowRecord)) {
                color += record.material->shade(record, lightDir, lightColor) * scene.diffuseMultiplier();
            }
        }

        Vec3 attenuation;
        Ray scattered;
        if (record.material->scatter(ray, record, attenuation, scattered)) {
            color += attenuation * trace(scattered, scene, depth - 1);
        }
        return color;
    }
    return _backgroundColor;
}