#pragma once

#include "../Interfaces/IShape.hpp"
#include "../plugins/Shapes/Sphere.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>

class ShapeFactory {
public:
    /**
     * @brief Creates a shape based on type and configuration.
     * @param type The type of shape to create (e.g., "sphere", "plane").
     * @param config The libconfig Setting containing shape parameters.
     * @param material The material to apply to the shape.
     * @return A shared pointer to the created shape, or nullptr if type is unknown.
     */
    static std::shared_ptr<IShape> create(
        const std::string& type,
        const libconfig::Setting& config,
        std::shared_ptr<IMaterial> material
    ) {
        if (type == "sphere") {
            double x = config["position"]["x"];
            double y = config["position"]["y"];
            double z = config["position"]["z"];
            double radius = config["radius"];
            return std::make_shared<Sphere>(Vec3(x, y, z), radius, material);
        }
        return nullptr;
    }
};
