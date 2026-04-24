#include "LightFactory.hpp"
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
    double px = config["position"]["x"];
    double py = config["position"]["y"];
    double pz = config["position"]["z"];

    int cr = config["color"]["r"];
    int cg = config["color"]["g"];
    int cb = config["color"]["b"];

    double intensity = config["intensity"];

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("pointlight");
    auto createFunc = reinterpret_cast<ILight* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<ILight>(createFunc(px, py, pz, cr, cg, cb, intensity));
}

std::shared_ptr<ILight> LightFactory::_createDirectionalLight(const libconfig::Setting& config) {
    double nx = config["direction"]["x"];
    double ny = config["direction"]["y"];
    double nz = config["direction"]["z"];

    int cr = config["color"]["r"];
    int cg = config["color"]["g"];
    int cb = config["color"]["b"];

    double intensity = config["intensity"];

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("directionallight");
    auto createFunc = reinterpret_cast<ILight* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<ILight>(createFunc(nx, ny, nz, cr, cg, cb, intensity));
}