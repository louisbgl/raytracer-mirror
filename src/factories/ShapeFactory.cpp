#include "ShapeFactory.hpp"
#include "../utils/ConfigUtils.hpp"
#include "Interfaces/IShape.hpp"
#include "../core/PluginManager.hpp"
#include <memory>


std::shared_ptr<IShape> ShapeFactory::create(const std::string& type, const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    static std::unordered_map<std::string, ShapeCreator> creators = {
        // Bounded shapes
        {"sphere", _createSphere},
        {"limited_cylinder", _createLimitedCylinder},
        {"limited_cone", _createLimitedCone},
        {"limited_hourglass", _createLimitedHourglass},
        {"rectangle", _createRectangle},
        {"triangle", _createTriangle},
        {"box", _createBox},
        {"torus", _createTorus},
        {"tanglecube", _createTanglecube},
        // Infinite shapes
        {"plane", _createPlane},
        {"cylinder", _createCylinder},
        {"cone", _createCone},
        {"hourglass", _createHourglass},
    };

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config, material);
    }
    return nullptr;
}

Vec3 ShapeFactory::_getRotation(const libconfig::Setting& config) {
    if (config.exists("rotation")) {
        double x = ConfigUtils::getNumber(config["rotation"]["x"]);
        double y = ConfigUtils::getNumber(config["rotation"]["y"]);
        double z = ConfigUtils::getNumber(config["rotation"]["z"]);
        return Vec3(x, y, z);
    }
    return Vec3(0, 0, 0);
}

std::shared_ptr<IShape> ShapeFactory::_createSphere(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("sphere");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_cylinder");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_cone");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_hourglass");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createRectangle(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double width = ConfigUtils::getNumber(config["width"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("rectangle");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), width, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createTriangle(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double v0x = config["v0"]["x"], v0y = config["v0"]["y"], v0z = ConfigUtils::getNumber(config["v0"]["z"]);
    double v1x = config["v1"]["x"], v1y = config["v1"]["y"], v1z = ConfigUtils::getNumber(config["v1"]["z"]);
    double v2x = config["v2"]["x"], v2y = config["v2"]["y"], v2z = ConfigUtils::getNumber(config["v2"]["z"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("triangle");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createBox(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double width = ConfigUtils::getNumber(config["width"]);
    double height = ConfigUtils::getNumber(config["height"]);
    double depth = ConfigUtils::getNumber(config["depth"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("box");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), width, height, depth, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createTorus(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double majorRadius = ConfigUtils::getNumber(config["major_radius"]);
    double minorRadius = ConfigUtils::getNumber(config["minor_radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("torus");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), majorRadius, minorRadius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createTanglecube(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double scale = ConfigUtils::getNumber(config["scale"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("tanglecube");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), position.x(), position.y(), position.z(), scale, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createPlane(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    Vec3 normal = ConfigUtils::parseVec3(config["normal"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("plane");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(position.x(), position.y(), position.z(), normal.x(), normal.y(), normal.z(), &material));
}

std::shared_ptr<IShape> ShapeFactory::_createCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("cylinder");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(position.x(), position.y(), position.z(), ax, ay, az, radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("cone");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(position.x(), position.y(), position.z(), ax, ay, az, radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("hourglass");
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(rawCreateFunc);
    return std::shared_ptr<IShape>(createFunc(position.x(), position.y(), position.z(), ax, ay, az, radius, &material));
}
