#include "MaterialFactory.hpp"
#include "../core/PluginManager.hpp"
#include <libconfig.h++>
#include <memory>

std::shared_ptr<IMaterial> MaterialFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, MaterialCreator> creators = {
        {"lambertian", _createLambertian},
        {"transparent", _createTransparent},
        {"coloreddiffuse", _createColoredDiffuse},
        {"phong", _createPhong},
        {"perlinnoise", _createPerlinNoise},
        {"chessboard", _createChessboard},
        {"image_texture", _createImageTexture}
    };

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config);
    }
    return nullptr;
}

std::shared_ptr<IMaterial> MaterialFactory::_createLambertian(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("lambertian");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b));
}

std::shared_ptr<IMaterial> MaterialFactory::_createColoredDiffuse(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("coloreddiffuse");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b));
}

std::shared_ptr<IMaterial> MaterialFactory::_createTransparent(const libconfig::Setting& config) {
    double opacity = config["opacity"];
    double refractiveIndex = config["refractiveIndex"];
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("transparent");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(opacity, refractiveIndex, r, g, b));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPhong(const libconfig::Setting& config) {
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    double shininess = config.exists("shininess") ? (double)config["shininess"] : 32.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("phong");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(rawCreateFunc);
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
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("chessboard");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(r1, g1, b1, r2, g2, b2, scale));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPerlinNoise(const libconfig::Setting& config)
{
    int r = config["color"]["r"];
    int g = config["color"]["g"];
    int b = config["color"]["b"];
    double scale = config.exists("scale") ? config["scale"] : 1.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("perlinnoise");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(r, g, b, scale));
}

std::shared_ptr<IMaterial> MaterialFactory::_createImageTexture(const libconfig::Setting& config) {
    std::string path = config["path"].c_str();

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("image_texture");
    auto createFunc = reinterpret_cast<IMaterial* (*)(const char*)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(path.c_str()));
}
