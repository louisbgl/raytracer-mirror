#include "MaterialFactory.hpp"

PluginLoader MaterialFactory::_pluginLoader;
std::unordered_map<std::string, void*> MaterialFactory::_createFunctions;

std::shared_ptr<IMaterial> MaterialFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, MaterialCreator> creators = {
        {"lambertian", _createLambertian},
        {"transparent", _createTransparent}
    };

    if (!_ensureLoaded(type)) return nullptr;

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config);
    }
    return nullptr;
}

bool MaterialFactory::_ensureLoaded(const std::string& type) {
    // Normalize to lowercase for plugin path
    std::string normalizedType = type;
    if (!normalizedType.empty() && normalizedType[0] >= 'A' && normalizedType[0] <= 'Z') {
        normalizedType[0] = normalizedType[0] + ('a' - 'A');
    }

    if (_createFunctions.find(normalizedType) != _createFunctions.end()) return true;

    std::string pluginPath = "./plugins/materials/" + normalizedType + ".so";
    if (!_pluginLoader.load(pluginPath)) return false;

    void* createFunc = _pluginLoader.getSymbol(pluginPath, "create");
    if (!createFunc) return false;

    _createFunctions[normalizedType] = createFunc;
    return true;
}

std::shared_ptr<IMaterial> MaterialFactory::_createLambertian(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(_createFunctions["lambertian"]);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b));
}

std::shared_ptr<IMaterial> MaterialFactory::_createTransparent(const libconfig::Setting& config) {
    double opacity = config["opacity"];
    double refractiveIndex = config["refractiveIndex"];
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double)>(_createFunctions["transparent"]);
    return std::shared_ptr<IMaterial>(createFunc(opacity, refractiveIndex, r, g, b));
}