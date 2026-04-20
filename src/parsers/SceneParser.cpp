#include "SceneParser.hpp"
#include "../factories/MaterialFactory.hpp"
#include "../factories/ShapeFactory.hpp"
#include "../factories/LightFactory.hpp"
#include <libconfig.h++>
#include <iostream>
#include <unordered_map>

Scene SceneParser::parse(const std::string& filename) {
    libconfig::Config config;

    try {
        config.readFile(filename.c_str());
    } catch (const libconfig::FileIOException& fioex) {
        std::cerr << "I/O error while reading file: " << filename << std::endl;
        throw;
    } catch (const libconfig::ParseException& pex) {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        throw;
    }

    Scene scene;

    parseCamera(config, scene);

    std::unordered_map<std::string, std::shared_ptr<IMaterial>> materialMap;
    parseMaterials(config, materialMap);

    World world;
    parseShapes(config, materialMap, world);

    std::vector<std::shared_ptr<ILight>> lights;
    parseLights(config, lights);

    // Construct final scene with all components
    scene = Scene(world, scene.camera(), lights);

    std::cout << "Scene file parsed successfully." << std::endl;
    return scene;
}

void SceneParser::parseCamera(libconfig::Config& config, Scene& scene) {
    try {
        int height = config.lookup("camera.resolution.height");
        int width = config.lookup("camera.resolution.width");
        double pos_x = config.lookup("camera.position.x");
        double pos_y = config.lookup("camera.position.y");
        double pos_z = config.lookup("camera.position.z");
        double look_x = config.lookup("camera.look_at.x");
        double look_y = config.lookup("camera.look_at.y");
        double look_z = config.lookup("camera.look_at.z");
        double up_x = config.lookup("camera.up.x");
        double up_y = config.lookup("camera.up.y");
        double up_z = config.lookup("camera.up.z");
        double fov = config.lookup("camera.fieldOfView");

        Camera camera(
            height, width,
            Vec3(pos_x, pos_y, pos_z),
            Vec3(look_x, look_y, look_z),
            Vec3(up_x, up_y, up_z),
            fov
        );
        scene.set_camera(camera);
        std::cout << "Camera configured: " << width << "x" << height << ", FOV: " << fov << std::endl;
    } catch (const libconfig::SettingNotFoundException& nfex) {
        std::cerr << "Camera configuration incomplete: " << nfex.getPath() << std::endl;
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Camera configuration type error at: " << tex.getPath() << std::endl;
    }
}

void SceneParser::parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap) {
    try {
        const libconfig::Setting& materials = config.lookup("materials");

        for (int i = 0; i < materials.getLength(); ++i) {
            const libconfig::Setting& matType = materials[i];
            std::string matTypeName = matType.getName();

            for (int j = 0; j < matType.getLength(); ++j) {
                try {
                    const libconfig::Setting& mat = matType[j];
                    std::string name = mat["name"].c_str();
                    std::shared_ptr<IMaterial> material = MaterialFactory::create(matTypeName, mat);

                    if (material) {
                        materialMap[name] = material;
                        std::cout << "Created material: " << name << " (type: " << matTypeName << ")" << std::endl;
                    } else {
                        std::cerr << "Failed to create material of type: " << matTypeName << std::endl;
                    }
                } catch (const libconfig::SettingTypeException& tex) {
                    std::cerr << "Material type error in " << matTypeName << "[" << j << "]: " << tex.getPath() << std::endl;
                }
            }
        }
    } catch (const libconfig::SettingNotFoundException& nfex) {
        std::cerr << "Materials section not found in config" << std::endl;
    }
}

void SceneParser::parseShapes(libconfig::Config& config, const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap, World& world) {
    try {
        const libconfig::Setting& shapes = config.lookup("shapes");

        for (int i = 0; i < shapes.getLength(); ++i) {
            const libconfig::Setting& shapeType = shapes[i];
            std::string typeName = shapeType.getName();

            std::string factoryType = typeName;
            if (factoryType.back() == 's') {
                factoryType.pop_back();  // "spheres" -> "sphere"
            }

            for (int j = 0; j < shapeType.getLength(); ++j) {
                try {
                    const libconfig::Setting& shape = shapeType[j];
                    std::string matName = shape["material"].c_str();

                    auto it = materialMap.find(matName);
                    if (it == materialMap.end()) {
                        std::cerr << "Material not found: " << matName << " for " << factoryType << "[" << j << "]" << std::endl;
                        continue;
                    }

                    std::shared_ptr<IMaterial> material = it->second;
                    std::shared_ptr<IShape> obj = ShapeFactory::create(factoryType, shape, material);

                    if (obj) {
                        world.add_object(obj);
                        std::cout << "Created " << factoryType << " with material " << matName << std::endl;
                    } else {
                        std::cerr << "Failed to create shape of type: " << factoryType << std::endl;
                    }
                } catch (const libconfig::SettingTypeException& tex) {
                    std::cerr << "Shape type error in " << typeName << "[" << j << "]: " << tex.getPath() << std::endl;
                }
            }
        }
    } catch (const libconfig::SettingNotFoundException& nfex) {
        std::cerr << "Shapes section not found in config" << std::endl;
    }
}

void SceneParser::parseLights(libconfig::Config& config, std::vector<std::shared_ptr<ILight>>& lights) {
    try {
        const libconfig::Setting& lightSettings = config.lookup("lights");

        for (int i = 0; i < lightSettings.getLength(); ++i) {
            const libconfig::Setting& lightType = lightSettings[i];
            std::string typeName = lightType.getName();

            if (!lightType.isList() && !lightType.isArray()) continue;

            for (int j = 0; j < lightType.getLength(); ++j) {
                try {
                    const libconfig::Setting& light = lightType[j];
                    std::shared_ptr<ILight> lightObj = LightFactory::create(typeName, light);

                    if (lightObj) {
                        lights.push_back(lightObj);
                        std::cout << "Created " << typeName << " light" << std::endl;
                    } else {
                        std::cerr << "Failed to create light of type: " << typeName << std::endl;
                    }
                } catch (const libconfig::SettingTypeException& tex) {
                    std::cerr << "Light type error in " << typeName << "[" << j << "]: " << tex.getPath() << std::endl;
                }
            }
        }
    } catch (const libconfig::SettingNotFoundException& nfex) {
        std::cerr << "Lights section not found in config" << std::endl;
    }
}
