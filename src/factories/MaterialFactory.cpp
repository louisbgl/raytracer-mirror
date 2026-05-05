#include "MaterialFactory.hpp"
#include "../utils/ConfigUtils.hpp"
#include "../core/PluginManager.hpp"
#include <libconfig.h++>
#include <memory>

std::shared_ptr<IMaterial> MaterialFactory::create(const std::string& type, const libconfig::Setting& config) {
    static std::unordered_map<std::string, MaterialCreator> creators = {
        {"lambertian", _createLambertian},
        {"refractive", _createRefractive},
        {"reflective", _createReflective},
        {"coloreddiffuse", _createColoredDiffuse},
        {"phong", _createPhong},
        {"perlinnoise", _createPerlinNoise},
        {"chessboard", _createChessboard},
        {"image_texture", _createImageTexture},
        {"normalmapped", _createNormalMapped}
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

    auto createFunc = reinterpret_cast<IMaterial* (*)(Vec3C)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.toCStruct()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createColoredDiffuse(const libconfig::Setting& config) {
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("coloreddiffuse");
    
    auto createFunc = reinterpret_cast<IMaterial* (*)(Vec3C)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(color.toCStruct()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createRefractive(const libconfig::Setting& config) {
    double opacity = ConfigUtils::getNumber(config["opacity"]);
    double refractiveIndex = ConfigUtils::getNumber(config["refractiveIndex"]);
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("refractive");

    auto createFunc = reinterpret_cast<IMaterial* (*)(
        double,
        double,
        Vec3C
    )>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(
        opacity,
        refractiveIndex,
        color.toCStruct()
    ));
}

std::shared_ptr<IMaterial> MaterialFactory::_createReflective(const libconfig::Setting& config) {
    double reflectivity = ConfigUtils::getNumber(config["reflectivity"]);
    Vec3 color = ConfigUtils::parseColor(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("reflective");

    auto createFunc = reinterpret_cast<IMaterial* (*)(
        double,
        Vec3C
    )>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(
        reflectivity,
        color.toCStruct()
    ));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPhong(const libconfig::Setting& config) {
    Vec3 color = ConfigUtils::parseColor(config);
    double shininess = config.exists("shininess") ? (double)config["shininess"] : 32.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("phong");

    auto createFunc = reinterpret_cast<IMaterial* (*)(
        Vec3C,
        double
    )>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(
        color.toCStruct(),
        shininess
    ));
}

std::shared_ptr<IMaterial> MaterialFactory::_createChessboard(const libconfig::Setting& config) {
    double scale = config.exists("scale") ? (double)config["scale"] : 1.0;
    Vec3 color1 = ConfigUtils::parseColor(config, "color1");
    Vec3 color2 = ConfigUtils::parseColor(config, "color2");
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("chessboard");

    auto createFunc = reinterpret_cast<IMaterial* (*)(
        Vec3C,
        Vec3C,
        double
    )>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(
        color1.toCStruct(),
        color2.toCStruct(),
        scale
    ));
}

std::shared_ptr<IMaterial> MaterialFactory::_createPerlinNoise(const libconfig::Setting& config)
{
    Vec3 color = ConfigUtils::parseColor(config);
    double scale = config.exists("scale") ? config["scale"] : 1.0;
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("perlinnoise");

    auto createFunc = reinterpret_cast<IMaterial* (*)(
        Vec3C,
        double
    )>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(
        color.toCStruct(),
        scale
    ));
}

std::shared_ptr<IMaterial> MaterialFactory::_createImageTexture(const libconfig::Setting& config) {
    std::string path = config["path"].c_str();
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("image_texture");

    auto createFunc = reinterpret_cast<IMaterial* (*)(const char*)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(path.c_str()));
}

std::shared_ptr<IMaterial> MaterialFactory::_createNormalMapped(const libconfig::Setting& config) {
    std::string path = config["path"].c_str();
    Vec3 albedo = config.exists("color") ? ConfigUtils::parseColor(config) : Vec3(255, 255, 255);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("normalmapped");

    auto createFunc = reinterpret_cast<IMaterial* (*)(const char*, Vec3C)>(rawCreateFunc);
    return std::shared_ptr<IMaterial>(createFunc(path.c_str(), albedo.toCStruct()));
}
