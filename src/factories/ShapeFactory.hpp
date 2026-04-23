#pragma once

#include "../Interfaces/IShape.hpp"
#include "../core/PluginLoader.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>
#include <unordered_map>

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
    );

private:
    using ShapeCreator = std::shared_ptr<IShape>(*)(const libconfig::Setting&, std::shared_ptr<IMaterial>);

    static PluginLoader _pluginLoader;
    static std::unordered_map<std::string, void*> _createFunctions;

    /**
     * @brief Tries to ensure the plugin for the specified type is loaded.
     * @param type The type of plugin to ensure (e.g., "sphere").
     * @return True if the plugin is loaded successfully, false otherwise.
     */
    static bool _ensureLoaded(const std::string& type);

    static std::shared_ptr<IShape> _createSphere(const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createLimitedCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
};
