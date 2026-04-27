#include "MaterialFactory.hpp"
#include "../utils/ConfigUtils.hpp"
#include "../core/PluginManager.hpp"
#include <libconfig.h++>
#include <memory>

std::shared_ptr<IMaterial> MaterialFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, MaterialCreator> creators = {
        {"lambertian", _createLambertian},
        {"refractive", _createRefractive},
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
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("lambertian");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.x(), color.y(), color.z()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createColoredDiffuse(const libconfig::Setting& config) {
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("coloreddiffuse");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.x(), color.y(), color.z()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createRefractive(const libconfig::Setting& config) {
    double opacity = ConfigUtils::getNumber(config["opacity"]);
    double refractiveIndex = ConfigUtils::getNumber(config["refractiveIndex"]);
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("refractive");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(opacity, refractiveIndex, color.x(), color.y(), color.z()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPhong(const libconfig::Setting& config) {
    Vec3 color = ConfigUtils::parseColor(config);
    double shininess = config.exists("shininess") ? (double)config["shininess"] : 32.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("phong");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.x(), color.y(), color.z(), shininess));
}

std::shared_ptr<IMaterial> MaterialFactory::_createChessboard(const libconfig::Setting& config) {
    double scale = config.exists("scale") ? (double)config["scale"] : 1.0;
    Vec3 color1 = ConfigUtils::parseColor(config, "color1");
    Vec3 color2 = ConfigUtils::parseColor(config, "color2");
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("chessboard");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color1.x(), color1.y(), color1.z(), color2.x(), color2.y(), color2.z(), scale));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPerlinNoise(const libconfig::Setting& config)
{
    Vec3 color = ConfigUtils::parseColor(config);
    double scale = config.exists("scale") ? config["scale"] : 1.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("perlinnoise");
    auto createFunc = reinterpret_cast<IMaterial* (*)(double, double, double, double)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.x(), color.y(), color.z(), scale));
}

std::shared_ptr<IMaterial> MaterialFactory::_createImageTexture(const libconfig::Setting& config) {
    std::string path = config["path"].c_str();

    auto rawCreateFunc = PluginManager::instance().getCreateFunction("image_texture");
    auto createFunc = reinterpret_cast<IMaterial* (*)(const char*)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(path.c_str()));
}
