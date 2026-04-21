#pragma once

#include "../Interfaces/ILight.hpp"
#include "../core/PluginLoader.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>
#include <unordered_map>

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
    );

private:
    using LightCreator = std::shared_ptr<ILight>(*)(const libconfig::Setting&);

    static PluginLoader _pluginLoader;
    static std::unordered_map<std::string, void*> _createFunctions;

    /**
     * @brief Tries to ensure the plugin for the specified type is loaded.
     * @param type The type of plugin to ensure (e.g., "point").
     * @return True if the plugin is loaded successfully, false otherwise.
     */
    static bool _ensureLoaded(const std::string& type);

    static std::shared_ptr<ILight> _createPointLight(const libconfig::Setting& config);
};
