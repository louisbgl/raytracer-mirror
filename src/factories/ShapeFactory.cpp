#include "ShapeFactory.hpp"
#include <memory>
#include "Interfaces/IShape.hpp"

PluginLoader ShapeFactory::_pluginLoader;
std::unordered_map<std::string, void*> ShapeFactory::_createFunctions;

std::shared_ptr<IShape> ShapeFactory::create(const std::string& type, const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    static std::unordered_map<std::string, ShapeCreator> creators = {
        // Bounded shapes
        {"sphere", _createSphere},
        {"limited_cylinder", _createLimitedCylinder},
        {"limited_cone", _createLimitedCone},
        {"limited_hourglass", _createLimitedHourglass},
        {"rectangle", _createRectangle},
        {"box", _createBox},
        {"torus", _createTorus},
        {"tanglecube", _createTanglecube},
        // Infinite shapes
        {"plane", _createPlane},
        {"cylinder", _createCylinder},
        {"cone", _createCone},
        {"hourglass", _createHourglass},
    };

    if (!_ensureLoaded(type)) return nullptr;

    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(config, material);
    }
    return nullptr;
}

bool ShapeFactory::_ensureLoaded(const std::string& type) {
    if (_createFunctions.find(type) != _createFunctions.end()) return true;

    std::string pluginPath = "./plugins/shapes/" + type + PLUGIN_EXTENSION;
    if (!_pluginLoader.load(pluginPath)) return false;

    void* createFunc = _pluginLoader.getSymbol(pluginPath, "create");
    if (!createFunc) return false;

    _createFunctions[type] = createFunc;
    return true;
}

Vec3 ShapeFactory::_getRotation(const libconfig::Setting& config) {
    if (config.exists("rotation")) {
        double x = config["rotation"]["x"];
        double y = config["rotation"]["y"];
        double z = config["rotation"]["z"];
        return Vec3(x, y, z);
    }
    return Vec3(0, 0, 0);
}

std::shared_ptr<IShape> ShapeFactory::_createSphere(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double radius = config["radius"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["sphere"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double radius = config["radius"];
    double height = config["height"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["limited_cylinder"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double radius = config["radius"];
    double height = config["height"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["limited_cone"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createLimitedHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double radius = config["radius"];
    double height = config["height"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["limited_hourglass"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, radius, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createRectangle(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double width = config["width"];
    double height = config["height"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["rectangle"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, width, height, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createBox(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    Vec3 rotation = _getRotation(config);
    double tx = config["position"]["x"];
    double ty = config["position"]["y"];
    double tz = config["position"]["z"];
    double width = config["width"];
    double height = config["height"];
    double depth = config["depth"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["box"]);
    return std::shared_ptr<IShape>(createFunc(rotation.x(), rotation.y(), rotation.z(), tx, ty, tz, width, height, depth, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createTorus(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double majorRadius = config["major_radius"];
    double minorRadius = config["minor_radius"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["torus"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, majorRadius, minorRadius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createTanglecube(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double scale = config["scale"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["tanglecube"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, scale, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createPlane(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double nx = config["normal"]["x"];
    double ny = config["normal"]["y"];
    double nz = config["normal"]["z"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["plane"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, nx, ny, nz, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createCylinder(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double radius = config["radius"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["cylinder"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createCone(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double radius = config["radius"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["cone"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, radius, &material));
}

std::shared_ptr<IShape> ShapeFactory::_createHourglass(const libconfig::Setting& config, std::shared_ptr<IMaterial> material) {
    double x = config["position"]["x"];
    double y = config["position"]["y"];
    double z = config["position"]["z"];
    double radius = config["radius"];
    auto createFunc = reinterpret_cast<IShape* (*)(double, double, double, double, std::shared_ptr<IMaterial>*)>(_createFunctions["hourglass"]);
    return std::shared_ptr<IShape>(createFunc(x, y, z, radius, &material));
}
