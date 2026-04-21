#include "LightFactory.hpp"

PluginLoader LightFactory::_pluginLoader;
std::unordered_map<std::string, void*> LightFactory::_createFunctions;

std::shared_ptr<ILight> LightFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, LightCreator> creators = {
        {"point", _createPointLight},
    };

    if (!_ensureLoaded(type)) return nullptr;

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config);
    }
    return nullptr;
}

bool LightFactory::_ensureLoaded(const std::string& type) {
    if (_createFunctions.find(type) != _createFunctions.end()) return true;

    std::string pluginPath = "./plugins/lights/" + type + "light.so";
    if (!_pluginLoader.load(pluginPath)) return false;

    void* createFunc = _pluginLoader.getSymbol(pluginPath, "create");
    if (!createFunc) return false;

    _createFunctions[type] = createFunc;
    return true;
}

std::shared_ptr<ILight> LightFactory::_createPointLight(const libconfig::Setting& config) {
    double px = config["position"]["x"];
    double py = config["position"]["y"];
    double pz = config["position"]["z"];

    int cr = config["color"]["r"];
    int cg = config["color"]["g"];
    int cb = config["color"]["b"];

    double intensity = config["intensity"];

    auto createFunc = reinterpret_cast<ILight* (*)(double, double, double, double, double, double)>(_createFunctions["point"]);
    return std::shared_ptr<ILight>(createFunc(px, py, pz, cr * intensity, cg * intensity, cb * intensity));
}
