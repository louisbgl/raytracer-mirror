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

    static PluginLoader _pluginLoader;
    static std::unordered_map<std::string, void*> _createFunctions;

    /**
     * @brief Tries to ensure the plugin for the specified type is loaded.
     * @param type The type of plugin to ensure (e.g., "lambertian").
     * @return True if the plugin is loaded successfully, false otherwise.
     */
    static bool _ensureLoaded(const std::string& type);

    static std::shared_ptr<IMaterial> _createLambertian     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createTransparent    (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createColoredDiffuse (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createPhong          (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createChessboard     (const libconfig::Setting& config);
    static std::shared_ptr<IMaterial> _createPerlinNoise    (const libconfig::Setting& config);
};
