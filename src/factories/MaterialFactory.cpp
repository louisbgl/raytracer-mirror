#include "MaterialFactory.hpp"
#include <libconfig.h++>
#include <memory>

PluginLoader MaterialFactory::_pluginLoader;
std::unordered_map<std::string, void*> MaterialFactory::_createFunctions;

std::shared_ptr<IMaterial> MaterialFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, MaterialCreator> creators = {
        {"lambertian", _createLambertian},
        {"transparent", _createTransparent},
        {"coloreddiffuse", _createColoredDiffuse},
        {"phong", _createPhong},
        {"perlinnoise", _createPerlinNoise},
        {"chessboard", _createChessboard}
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

    std::string pluginPath = "./plugins/materials/" + normalizedType + PLUGIN_EXTENSION;
    if (!_pluginLoader.load(pluginPath)) { 
        return false; 
    }
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

std::shared_ptr<IMaterial> MaterialFactory::_createColoredDiffuse(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(_createFunctions["coloreddiffuse"]);
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

std::shared_ptr<IMaterial> MaterialFactory::_createPhong(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    double shininess = config.exists("shininess") ? (double)config["shininess"] : 32.0;

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(_createFunctions["phong"]);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b, shininess));
}

std::shared_ptr<IMaterial> MaterialFactory::_createChessboard(const libconfig::Setting& config) {
    double scale = config.exists("scale") ? (double)config["scale"] : 1.0;
    int r1 = config["color1"]["r"];
    int g1 = config["color1"]["g"];
    int b1 = config["color1"]["b"];
    int r2 = config["color2"]["r"];
    int g2 = config["color2"]["g"];
    int b2 = config["color2"]["b"];

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double, double, double)>(_createFunctions["chessboard"]);
    return std::shared_ptr<IMaterial>(createFunc(r1, g1, b1, r2, g2, b2, scale));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPerlinNoise(const libconfig::Setting& config)
{
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    double scale = config.exists("scale") ? config["scale"] : 1.0;

    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(_createFunctions["perlinnoise"]);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b, scale));
}
