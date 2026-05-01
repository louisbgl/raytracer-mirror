#pragma once

#include "../Interfaces/ILight.hpp"
#include "../core/PluginLoader.hpp"
#include "../Math/Matrix4x4.hpp"
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

    static std::shared_ptr<ILight> create(
        const std::string& type,
        const libconfig::Setting& config,
        const Matrix4x4& parentTransform
    );

private:
    using LightCreator = std::shared_ptr<ILight>(*)(const libconfig::Setting&);
    using LightCreatorWithTransform = std::shared_ptr<ILight>(*)(const libconfig::Setting&, const Matrix4x4&);

    static std::shared_ptr<ILight> _createPointLight(const libconfig::Setting& config);
    static std::shared_ptr<ILight> _createDirectionalLight(const libconfig::Setting& config);
    static std::shared_ptr<ILight> _createPointLightTransformed(const libconfig::Setting& config, const Matrix4x4& parentTransform);
    static std::shared_ptr<ILight> _createDirectionalLightTransformed(const libconfig::Setting& config, const Matrix4x4& parentTransform);
};
