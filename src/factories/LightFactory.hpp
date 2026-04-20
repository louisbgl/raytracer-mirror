#pragma once

#include "../Interfaces/ILight.hpp"
#include "../plugins/Lights/PointLight.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>

class LightFactory {
public:
    /**
     * @brief Creates a light based on type and configuration.
     * @param type The type of light to create (e.g., "point", "directional").
     * @param config The libconfig Setting containing light parameters.
     * @return A shared pointer to the created light, or nullptr if type is unknown.
     */
    static std::shared_ptr<ILight> create(
        const std::string& type,
        const libconfig::Setting& config
    ) {
        if (type == "point") {
            double px = config["position"]["x"];
            double py = config["position"]["y"];
            double pz = config["position"]["z"];

            int cr = config["color"]["r"];
            int cg = config["color"]["g"];
            int cb = config["color"]["b"];

            double intensity = config["intensity"];

            return std::make_shared<PointLight>(
                Vec3(px, py, pz),
                Vec3(cr, cg, cb) * intensity
            );
        }
        return nullptr;
    }
};
