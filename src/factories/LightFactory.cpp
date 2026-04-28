#include "LightFactory.hpp"
#include "../utils/ConfigUtils.hpp"
#include "../core/PluginManager.hpp"

std::shared_ptr<ILight> LightFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, LightCreator> creators = {
        {"point", _createPointLight},
        {"directional", _createDirectionalLight},
    };

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config);
    }
    return nullptr;
}

std::shared_ptr<ILight> LightFactory::_createPointLight(const libconfig::Setting& config) {
    Vec3 position = ConfigUtils::parsePosition(config);
    Vec3 color = ConfigUtils::parseColor(config);
    double intensity = ConfigUtils::getNumber(config["intensity"]);

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("pointlight");
    auto createFunc = reinterpret_cast<ILight* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<ILight>(createFunc(position.x(), position.y(), position.z(), color.x(), color.y(), color.z(), intensity));
}

std::shared_ptr<ILight> LightFactory::_createDirectionalLight(const libconfig::Setting& config) {
    Vec3 direction = ConfigUtils::parseVec3(config["direction"]);
    Vec3 color = ConfigUtils::parseColor(config);
    double intensity = ConfigUtils::getNumber(config["intensity"]);

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("directionallight");
    auto createFunc = reinterpret_cast<ILight* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<ILight>(createFunc(direction.x(), direction.y(), direction.z(), color.x(), color.y(), color.z(), intensity));
}