#include "ShapeFactory.hpp"
#include "../utils/ConfigUtils.hpp"
#include "Interfaces/IShape.hpp"
#include "core/AShape.hpp"
#include "../core/PluginManager.hpp"
#include <memory>
#include <iostream>
#include <unordered_set>

std::unordered_map<std::string, ShapeFactory::ShapeCreator> ShapeFactory::creators = {
    // Bounded shapes
    {"sphere", _createSphere},
    {"limited_cylinder", _createLimitedCylinder},
    {"limited_cone", _createLimitedCone},
    {"limited_hourglass", _createLimitedHourglass},
    {"rectangle", _createRectangle},
    {"triangle", _createTriangle},
    {"mesh", _createMesh},
    {"box", _createBox},
    {"torus", _createTorus},
    {"tanglecube", _createTanglecube},
    {"menger_sponge", _createMengerSponge},
    {"mobius_strip", _createMobiusStrip},

    // Infinite shapes
    {"plane", _createPlane},
    {"cylinder", _createCylinder},
    {"cone", _createCone},
    {"hourglass", _createHourglass},
};

std::shared_ptr<IShape> ShapeFactory::create(const std::string& type, const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config, material);
    }
    std::cout << "Error: Unknown shape type '" << type << std::endl;
    std::cout << "Available shapes: ";
    for (const auto& pair : creators) {
        std::cout << pair.first << " ";
    }
    std::cout << std::endl;
    return nullptr;
}

std::shared_ptr<IShape> ShapeFactory::create(const std::string& type, const libconfig::Setting& config, std::shared_ptr<IMaterial> material, const Matrix4x4& parentTransform) {
    auto shape = create(type, config, material);
    if (!shape) return nullptr;

    if (auto aShape = dynamic_cast<AShape*>(shape.get())) {
        aShape->applyParentTransform(parentTransform);
    } else {
        static std::unordered_set<std::string> warned;
        if (warned.insert(type).second && parentTransform != Matrix4x4::identity())
            std::cerr << "Warning: '" << type << "' does not support subscene transforms. Object placed at original position." << std::endl;
    }

    return shape;
}

Vec3 ShapeFactory::_getRotation(const libconfig::Setting& config) {
    if (config.exists("rotation")) {
        return ConfigUtils::parseVec3(config["rotation"]);
    }
    return Vec3(0, 0, 0);
}

Vec3 ShapeFactory::_getScale(const libconfig::Setting& config) {
    if (config.exists("scale")) {
        return ConfigUtils::parseVec3(config["scale"]);
    }
    return Vec3(1, 1, 1);
}

std::shared_ptr<IShape> ShapeFactory::_createSphere(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("sphere");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        radius,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_cylinder");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        radius,
        height,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_cone");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        radius,
        height,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config["radius"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("limited_hourglass");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        radius,
        height,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createRectangle(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double width = ConfigUtils::getNumber(config["width"]);
    double height = ConfigUtils::getNumber(config["height"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("rectangle");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        width, height,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createTriangle(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    Vec3 v0 = ConfigUtils::parseVec3(config["v0"]);
    Vec3 v1 = ConfigUtils::parseVec3(config["v1"]);
    Vec3 v2 = ConfigUtils::parseVec3(config["v2"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("triangle");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        Vec3C,
        Vec3C,
        Vec3C,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        v0.toCStruct(),
        v1.toCStruct(),
        v2.toCStruct(),
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createBox(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double width = ConfigUtils::getNumber(config["width"]);
    double height = ConfigUtils::getNumber(config["height"]);
    double depth = ConfigUtils::getNumber(config["depth"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("box");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        width, height,
        depth,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createTorus(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double majorRadius = ConfigUtils::getNumber(config["major_radius"]);
    double minorRadius = ConfigUtils::getNumber(config["minor_radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("torus");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        majorRadius, minorRadius,
        &material
    ));
}

// TODO: adapt the tangle cube so it supports the scale & rotation of AShape, then remove the hardcoded 0/1 values and use the same pattern as the other shapes
std::shared_ptr<IShape> ShapeFactory::_createTanglecube(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    // Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double scale = ConfigUtils::getNumber(config["scale"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("tanglecube");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);
    
    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createMengerSponge(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    int iterations = static_cast<int>(ConfigUtils::getNumber(config["iterations"]));
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("menger_sponge");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        int,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        iterations,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createMobiusStrip(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    Vec3 scale    = _getScale(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    double radius = ConfigUtils::getNumber(config, "radius", 5.0);
    double width = ConfigUtils::getNumber(config, "width", 2.0);
    double thickness = ConfigUtils::getNumber(config, "thickness", 0.2);
    int twists = static_cast<int>(ConfigUtils::getNumber(config, "twists", 1));
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("mobius_strip");

    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        Vec3C,
        double,
        double,
        double,
        int,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        radius,
        width,
        thickness,
        twists,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createPlane(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    Vec3 normal = ConfigUtils::parseVec3(config["normal"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("plane");
    
    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        position.toCStruct(),
        normal.toCStruct(),
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    Vec3 axis(ax, ay, az);
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("cylinder");
    
    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        position.toCStruct(),
        axis.toCStruct(),
        radius,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    Vec3 axis(ax, ay, az);
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("cone");
    
    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        position.toCStruct(),
        axis.toCStruct(),
        radius,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 position = ConfigUtils::parsePosition(config);
    double ax = config.exists("axis") ? (double)config["axis"]["x"] : 0.0;
    double ay = config.exists("axis") ? (double)config["axis"]["y"] : 1.0;
    double az = config.exists("axis") ? (double)config["axis"]["z"] : 0.0;
    Vec3 axis(ax, ay, az);
    double radius = ConfigUtils::getNumber(config["radius"]);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("hourglass");
    
    auto createFunc = reinterpret_cast<IShape* (*)(
        Vec3C,
        Vec3C,
        double,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        position.toCStruct(),
        axis.toCStruct(),
        radius,
        &material
    ));
}

std::shared_ptr<IShape> ShapeFactory::_createMesh(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    std::string path = config["path"].c_str();
    Vec3 rotation = _getRotation(config);
    Vec3 position = ConfigUtils::parsePosition(config);
    Vec3 scale = _getScale(config);
    auto rawCreateFunc = PluginManager::instance().getCreateFunction("mesh");
    
    auto createFunc = reinterpret_cast<IShape* (*)(
        const char*,
        Vec3C,
        Vec3C,
        Vec3C,
        std::shared_ptr<IMaterial>*
    )>(rawCreateFunc);

    return std::shared_ptr<IShape>(createFunc(
        path.c_str(),
        rotation.toCStruct(),
        position.toCStruct(),
        scale.toCStruct(),
        &material
    ));
}
