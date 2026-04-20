#pragma once

#include "../Interfaces/IMaterial.hpp"
#include "../plugins/Materials/Lambertian.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>

class MaterialFactory {
public:
    /**
     * @brief Creates a material based on type and configuration.
     * @param type The type of material to create (e.g., "Lambertian").
     * @param config The libconfig Setting containing material parameters.
     * @return A shared pointer to the created material, or nullptr if type is unknown.
     */
    static std::shared_ptr<IMaterial> create(
        const std::string& type,
        const libconfig::Setting& config
    ) {
        if (type == "Lambertian") {
            int r = config["color"]["r"];
            int g = config["color"]["g"];
            int b = config["color"]["b"];
            Vec3 color(r / 255.0, g / 255.0, b / 255.0);
            return std::make_shared<Lambertian>(color);
        }
        return nullptr;
    }
};
