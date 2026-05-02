#include "SceneParser.hpp"
#include "../factories/MaterialFactory.hpp"
#include "../factories/ShapeFactory.hpp"
#include "../factories/LightFactory.hpp"
#include "../core/PluginManager.hpp"
#include "../utils/ConfigUtils.hpp"
#include "../DataTypes/RendererConfig.hpp"
#include "../Math/Matrix4x4.hpp"
#include <libconfig.h++>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

Scene SceneParser::parse(const std::string& filename) {
    _currentFile = filename;
    libconfig::Config config;

    _libConfigReadFile(config, filename);

    Scene scene;

    _parseRenderer(config, scene);
    _parseCamera(config, scene);

    std::unordered_map<std::string, std::shared_ptr<IMaterial>> materialMap;
    _parseMaterials(config, materialMap);

    World world;
    _parseShapes(config, materialMap, world);

    std::vector<std::shared_ptr<ILight>> lights;
    _parseLights(config, lights);

    std::set<std::string> loadingStack;
    _parseSubscenes(config, materialMap, world, lights, Matrix4x4::identity(), loadingStack, 0);

    scene.set_world(world);
    for (auto& light : lights) {
        scene.add_light(light);
    }
    scene.setMaterialCount(static_cast<int>(materialMap.size()));

    return scene;
}

void SceneParser::_libConfigReadFile(libconfig::Config& config, const std::string& filename) {
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
}

void SceneParser::_parseRenderer(libconfig::Config& config, Scene& scene) {
    RendererConfig rendererConfig;

    try {
        const libconfig::Setting& renderer = config.lookup("renderer");

        _parseLighting(renderer, rendererConfig);
        _parseBackground(renderer, rendererConfig);
        _parseAntialiasing(renderer, rendererConfig);
        _parseThreads(renderer, rendererConfig);

        if (renderer.exists("output")) {
            rendererConfig.outputFile = renderer["output"].c_str();
        }
        _parseToneMapping(renderer, rendererConfig);
    } catch (const libconfig::SettingNotFoundException& nfex) {
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Renderer configuration type error at: " << tex.getPath() << std::endl;
    }

    scene.setRendererConfig(rendererConfig);
}

void SceneParser::_parseLighting(const libconfig::Setting& renderer, RendererConfig& config) {
    if (!renderer.exists("lighting")) return;

    const libconfig::Setting& lighting = renderer["lighting"];

    if (lighting.exists("ambientColor")) {
        config.ambientColor = ConfigUtils::parseColor(lighting, "ambientColor");
    }
    if (lighting.exists("ambientMultiplier")) {
        config.ambientMultiplier = ConfigUtils::getNumber(lighting["ambientMultiplier"]);
    }
    if (lighting.exists("diffuseMultiplier")) {
        config.diffuseMultiplier = ConfigUtils::getNumber(lighting["diffuseMultiplier"]);
    }

    if (!lighting.exists("ambientOcclusion")) return;

    const libconfig::Setting& ao = lighting["ambientOcclusion"];

    if (ao.exists("enabled")) {
        config.aoEnabled = ao["enabled"];
    }
    if (ao.exists("samples")) {
        config.aoSamples = _validateAOSamples(ao["samples"]);
    }
    if (ao.exists("radius")) {
        config.aoRadius = ConfigUtils::getNumber(ao["radius"]);
    }

}

void SceneParser::_parseBackground(const libconfig::Setting& renderer, RendererConfig& config) {
    if (!renderer.exists("background")) return;

    const libconfig::Setting& bg = renderer["background"];

    if (bg.exists("color")) {
        config.backgroundColor = ConfigUtils::parseColor(bg);
    }
    if (bg.exists("image")) {
        config.backgroundImage = bg["image"].c_str();
        if (!std::filesystem::exists(config.backgroundImage)) {
            std::cerr << "Warning: Background image not found: " << config.backgroundImage << std::endl;
            config.backgroundImage.clear();
        }
    }
}

void SceneParser::_parseAntialiasing(const libconfig::Setting& renderer, RendererConfig& config) {
    if (!renderer.exists("antialiasing")) return;

    const libconfig::Setting& aa = renderer["antialiasing"];

    if (aa.exists("enabled")) config.aaEnabled = aa["enabled"];
    if (aa.exists("samples")) config.aaSamples = _validateAASamples(aa["samples"]);
    if (aa.exists("method")) config.aaMethod = _validateAAMethod(aa["method"].c_str());
    if (aa.exists("threshold")) config.aaThreshold = ConfigUtils::getNumber(aa["threshold"]);
}

void SceneParser::_parseCamera(libconfig::Config& config, Scene& scene) {
    try {
        int height = ConfigUtils::getNumber(config.lookup("camera.resolution.height"));
        int width = ConfigUtils::getNumber(config.lookup("camera.resolution.width"));
        Vec3 pos = ConfigUtils::parseVec3(config.lookup("camera.position"));
        Vec3 look_at = ConfigUtils::parseVec3(config.lookup("camera.look_at"));
        Vec3 up = ConfigUtils::parseVec3(config.lookup("camera.up"));
        double fov = ConfigUtils::getNumber(config.lookup("camera.fieldOfView"));

        Camera camera(
            height, width,
            pos,
            look_at,
            up,
            fov
        );
        scene.set_camera(camera);
    } catch (const libconfig::SettingNotFoundException& nfex) {
        std::cerr << "Camera configuration incomplete: " << nfex.getPath() << std::endl;
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Camera configuration type error at: " << tex.getPath() << std::endl;
    }
}

void SceneParser::_parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap) {
    try {
        const libconfig::Setting& materials = config.lookup("materials");
        for (int i = 0; i < materials.getLength(); ++i) {
            const libconfig::Setting& matType = materials[i];
            std::string typeName = matType.getName();
            for (int j = 0; j < matType.getLength(); ++j)
                _parseMaterialInstance(matType[j], typeName, materialMap);
        }
    } catch (const libconfig::SettingNotFoundException&) {
        std::cerr << "Materials section not found in " << _currentFile << std::endl;
    }
}

void SceneParser::_parseMaterialInstance(const libconfig::Setting& mat, const std::string& typeName,
                                         std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap) {
    try {
        std::string name = mat["name"].c_str();
        std::shared_ptr<IMaterial> material = MaterialFactory::create(typeName, mat);
        if (material)
            materialMap[name] = material;
        else
            std::cerr << "Failed to create material of type: " << typeName << std::endl;
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Material type error in " << typeName << ": " << tex.getPath() << std::endl;
    }
}

void SceneParser::_parseShapes(libconfig::Config& config,
                               const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                               World& world, const Matrix4x4& parentTransform) {
    try {
        const libconfig::Setting& shapes = config.lookup("shapes");
        for (int i = 0; i < shapes.getLength(); ++i) {
            const libconfig::Setting& shapeType = shapes[i];
            std::string typeName = PluginManager::instance().getSingular(shapeType.getName());
            for (int j = 0; j < shapeType.getLength(); ++j)
                _parseShapeInstance(shapeType[j], typeName, materialMap, world, parentTransform);
        }
    } catch (const libconfig::SettingNotFoundException&) {
        std::cerr << "Shapes section not found in " << _currentFile << std::endl;
    }
}

void SceneParser::_parseShapeInstance(const libconfig::Setting& shape, const std::string& typeName,
                                      const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                                      World& world, const Matrix4x4& parentTransform) {
    try {
        std::string matName = shape["material"].c_str();
        auto it = materialMap.find(matName);
        if (it == materialMap.end()) {
            std::cerr << "Material not found: " << matName << " for " << typeName << std::endl;
            return;
        }
        std::shared_ptr<IShape> obj = ShapeFactory::create(typeName, shape, it->second, parentTransform);
        if (obj)
            world.add_object(obj);
        else
            std::cerr << "Failed to create shape of type: " << typeName << std::endl;
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Shape type error in " << typeName << ": " << tex.getPath() << std::endl;
    }
}

void SceneParser::_parseLights(libconfig::Config& config,
                               std::vector<std::shared_ptr<ILight>>& lights,
                               const Matrix4x4& parentTransform) {
    try {
        const libconfig::Setting& lightSettings = config.lookup("lights");
        for (int i = 0; i < lightSettings.getLength(); ++i) {
            const libconfig::Setting& lightType = lightSettings[i];
            if (!lightType.isList() && !lightType.isArray()) continue;
            std::string typeName = lightType.getName();
            for (int j = 0; j < lightType.getLength(); ++j)
                _parseLightInstance(lightType[j], typeName, lights, parentTransform);
        }
    } catch (const libconfig::SettingNotFoundException&) {
        std::cerr << "Lights section not found in " << _currentFile << std::endl;
    }
}

void SceneParser::_parseLightInstance(const libconfig::Setting& light, const std::string& typeName,
                                      std::vector<std::shared_ptr<ILight>>& lights,
                                      const Matrix4x4& parentTransform) {
    try {
        std::shared_ptr<ILight> obj = LightFactory::create(typeName, light, parentTransform);
        if (obj)
            lights.push_back(obj);
        else
            std::cerr << "Failed to create light of type: " << typeName << std::endl;
    } catch (const libconfig::SettingTypeException& tex) {
        std::cerr << "Light type error in " << typeName << ": " << tex.getPath() << std::endl;
    }
}

void SceneParser::_parseSubscenes(libconfig::Config& config,
                                   std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                                   World& world, std::vector<std::shared_ptr<ILight>>& lights,
                                   const Matrix4x4& parentTransform,
                                   std::set<std::string>& loadingStack, int depth) {
    if (depth > MAX_SUBSCENE_DEPTH) {
        std::cerr << "Error: Maximum subscene depth exceeded. Possible circular reference in " << _currentFile << std::endl;
        return;
    }

    if (!config.exists("scenes")) return;

    const libconfig::Setting& scenes = config.lookup("scenes");
    for (int i = 0; i < scenes.getLength(); ++i) {
        const libconfig::Setting& subscene = scenes[i];
        if (!subscene.exists("path")) {
            std::cerr << "Subscene entry missing 'path' field in " << _currentFile << ". Skipping." << std::endl;
            continue;
        }

        std::string subsceneFile = subscene["path"];
        if (loadingStack.count(subsceneFile)) {
            std::cerr << "Error: Detected circular subscene reference: " << subsceneFile << " is already being loaded. Skipping." << std::endl;
            continue;
        }
        loadingStack.insert(subsceneFile);

        Vec3 pos = ConfigUtils::parsePosition(subscene);
        Vec3 rot = subscene.exists("rotation") ? ConfigUtils::parseVec3(subscene["rotation"]) : Vec3(0, 0, 0);
        Vec3 scale = subscene.exists("scale")  ? ConfigUtils::parseVec3(subscene["scale"])    : Vec3(1, 1, 1);
        Matrix4x4 subSceneTransform = parentTransform * Matrix4x4::translate(pos) * Matrix4x4::rotate(rot) * Matrix4x4::scale(scale);

        try {
            libconfig::Config subConfig;

            _libConfigReadFile(subConfig, subsceneFile);
            _parseMaterials(subConfig, materialMap);
            _parseShapes(subConfig, materialMap, world, subSceneTransform);
            _parseLights(subConfig, lights, subSceneTransform);
            _parseSubscenes(subConfig, materialMap, world, lights, subSceneTransform, loadingStack, depth + 1);
        } catch (const libconfig::FileIOException& fioex) {
            std::cerr << "I/O error while reading subscene file: " << subsceneFile << std::endl;
        } catch (const libconfig::ParseException& pex) {
            std::cerr << "Parse error in subscene file " << subsceneFile << " at " << pex.getFile() << ":" << pex.getLine()
                      << " - " << pex.getError() << std::endl;
        }

        loadingStack.erase(subsceneFile);
    }
}

int SceneParser::_validateAASamples(int samples) const {
    if (samples < 1) {
        std::cerr << "Warning: AntiAliasing samples must be >= 1, using 1" << std::endl;
        return 1;
    }
    if (samples > 64) {
        std::cerr << "Warning: AntiAliasing samples > 64 may be very slow" << std::endl;
    }
    return samples;
}

int SceneParser::_validateAOSamples(int samples) const {
    if (samples < 1) {
        std::cerr << "Warning: Ambient Occlusion samples must be >= 1, using 1" << std::endl;
        return 1;
    }
    if (samples > 128) {
        std::cerr << "Warning: Ambient Occlusion samples > 128 may be very slow" << std::endl;
    }
    return samples;
}

std::string SceneParser::_validateAAMethod(const std::string& method) const {
    static const std::unordered_set<std::string> supportedMethods = {
        "ssaa",
        "adaptive"
    };

    if (supportedMethods.find(method) != supportedMethods.end()) {
        return method;
    }

    std::cerr << "Warning: Unknown AntiAliasing method '" << method << "', using default 'ssaa'" << std::endl;
    return "ssaa";
}

void SceneParser::_parseThreads(const libconfig::Setting& renderer, RendererConfig& config)
{
    if (!renderer.exists("multithreading")) {
        return;
    }

    const libconfig::Setting& mtRenderer = renderer["multithreading"];

    if (mtRenderer.exists("enabled")) {
        config.multithreadingEnabled = mtRenderer["enabled"];
    }

    if (mtRenderer.exists("threads")) {
        config.threadCount = static_cast<int>(ConfigUtils::getNumber(mtRenderer["threads"]));
    }
}

void SceneParser::_parseToneMapping(const libconfig::Setting& renderer, RendererConfig& config) {
    if (!renderer.exists("toneMapping"))
        return;
    const libconfig::Setting& tm = renderer["toneMapping"];
    if (tm.exists("enabled"))
        config.toneMappingEnabled = tm["enabled"];
    if (tm.exists("strength"))
        config.toneMappingStrength = std::clamp(ConfigUtils::getNumber(tm["strength"]), 0.0, 1.0);
}
