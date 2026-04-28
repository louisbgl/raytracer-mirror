#pragma once

#include "../Interfaces/IMaterial.hpp"
#include "../core/PluginLoader.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>
#include <unordered_map>

class MaterialFactory {
public:
    /**
     * @brief Creates a material based on type and configuration.
     * @param type The type of material to create (e.g., "lambertian").
     * @param config The libconfig Setting containing material parameters.
     * @return A shared pointer to the created material, or nullptr if type is unknown.
     */
    static std::shared_ptr<IMaterial> create(
        const std::string& type,
        const libconfig::Setting& config
    );

private:
    using MaterialCreator = std::shared_ptr<IMaterial>(*)(const libconfig::Setting&);

    static std::shared_ptr<IMaterial> _createLambertian     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createRefractive     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createReflective     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createColoredDiffuse (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createPhong          (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createChessboard     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createPerlinNoise    (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createImageTexture   (const libconfig::Setting& config);
};
