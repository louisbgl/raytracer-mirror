#include "Interfaces/IShape.hpp"
#include "Interfaces/ILight.hpp"
#include "Interfaces/IMaterial.hpp"
#include "DataTypes/Camera.hpp"
#include "DataTypes/HitRecord.hpp"
#include "DataTypes/Ray.hpp"
#include "DataTypes/Scene.hpp"
#include "core/Image.hpp"
#include "plugins/Shapes/Sphere.hpp"
#include "plugins/Materials/Lambertian.hpp"
#include "plugins/Lights/PointLight.hpp"
#include <iostream>

Vec3 trace(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) return Vec3(0, 0, 0);

    HitRecord record;
    if (scene.world().get_closest_hit(ray, 0.001, 1000, record)) {
        // Step 3a: Direct lighting
        Vec3 ambient = Vec3(0.1, 0.1, 0.15);  // Slight blue ambient light
        Vec3 color = ambient;  // Start with ambient lighting

        for (const auto& light : scene.lights()) {
            Vec3 lightDir, lightColor;
            double lightDistance = light->get_light_data(record.point, lightDir, lightColor);

            Ray shadowRay(record.point, lightDir);
            HitRecord shadowRecord;

            if (!scene.world().get_closest_hit(shadowRay, 0.001, lightDistance, shadowRecord)) {
                color += record.material->shade(record, lightDir, lightColor);
            }
        }

        // Step 3b: Indirect lighting (recursion)
        Vec3 attenuation;
        Ray scattered;
        if (record.material->scatter(ray, record, attenuation, scattered)) {
            color += attenuation * trace(scattered, scene, depth - 1);
        }

        return color;
    }

    // Step 4: No hit - return background
    return Vec3(67, 67, 67);
}

int main() {
    int width = 1000;
    int height = 1000;
    Image image(width, height);
    // Camera positioned slightly above and back, looking at the scene
    Camera camera(Vec3(0, 1, 2), Vec3(0, 0, -2), Vec3(0, 1, 0), 60, 1.0);

    // Create materials with nice colors
    auto red_material = std::make_shared<Lambertian>(Vec3(0.8, 0.2, 0.2));
    auto green_material = std::make_shared<Lambertian>(Vec3(0.2, 0.8, 0.3));
    auto blue_material = std::make_shared<Lambertian>(Vec3(0.2, 0.3, 0.9));
    auto yellow_material = std::make_shared<Lambertian>(Vec3(0.9, 0.9, 0.2));
    auto purple_material = std::make_shared<Lambertian>(Vec3(0.7, 0.2, 0.8));
    auto ground_material = std::make_shared<Lambertian>(Vec3(0.5, 0.5, 0.5));

    // Large ground sphere (fake floor)
    Sphere ground(Vec3(0, -100.5, -3), 100, ground_material);

    // Center large sphere
    Sphere center_sphere(Vec3(0, 0, -3), 0.8, blue_material);

    // Left sphere
    Sphere left_sphere(Vec3(-1.5, -0.2, -2.5), 0.5, red_material);

    // Right sphere
    Sphere right_sphere(Vec3(1.3, -0.1, -2.8), 0.6, green_material);

    // Small floating spheres
    Sphere small_top(Vec3(0.5, 1.2, -2.5), 0.3, yellow_material);
    Sphere small_left(Vec3(-0.8, 1, -2.25), 0.25, purple_material);

    World world({
        std::make_shared<Sphere>(ground),
        std::make_shared<Sphere>(center_sphere),
        std::make_shared<Sphere>(left_sphere),
        std::make_shared<Sphere>(right_sphere),
        std::make_shared<Sphere>(small_top),
        std::make_shared<Sphere>(small_left)
    });

    // Main light from top-left (bright white)
    PointLight main_light(Vec3(-3, 4, 1), Vec3(15, 15, 15));

    // Fill light from right (dimmer, warmer)
    PointLight fill_light(Vec3(4, 2, 0), Vec3(3, 2.5, 2));

    Scene scene(world, camera, {
        std::make_shared<PointLight>(main_light),
        std::make_shared<PointLight>(fill_light)
    });

    // Render loop
    int max_depth = 50;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / (width - 1);
            float v = 1.0f - static_cast<float>(y) / (height - 1);

            Ray ray = camera.getRay(u, v);
            Vec3 color = trace(ray, scene, max_depth);
            image.setPixel(x, y, color);
        }
    }
    image.writePPM("output.ppm");
}