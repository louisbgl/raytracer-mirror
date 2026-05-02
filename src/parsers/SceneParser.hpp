#pragma once

#include "../DataTypes/Scene.hpp"
#include "DataTypes/RendererConfig.hpp"
#include "../Math/Matrix4x4.hpp"
#include <libconfig.h++>
#include <set>
#include <string>
#include <unordered_map>
#include <memory>

/**
 * @brief Parses scene configuration files and constructs a Scene object.
 * Currently supports libconfig++ format.
 */
class SceneParser {
public:
    SceneParser() = default;
    ~SceneParser() = default;

    /**
     * @brief Parses a scene configuration file.
     * @param filename Path to the scene configuration file.
     * @return A fully constructed Scene object.
     * @throws libconfig::FileIOException if file cannot be read.
     * @throws libconfig::ParseException if file has syntax errors.
     */
    Scene parse(const std::string& filename);

private:
    std::string _currentFile;
    static constexpr int MAX_SUBSCENE_DEPTH = 100; // Not an actual limit, just a safeguard against infinite recursion

    void _libConfigReadFile(libconfig::Config& config, const std::string& filename);

    void _parseRenderer(libconfig::Config& config, Scene& scene);
    void _parseCamera(libconfig::Config& config, Scene& scene);

    void _parseMaterials(libconfig::Config& config,
                         std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);

    void _parseShapes(libconfig::Config& config,
                      const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                      World& world,
                      const Matrix4x4& parentTransform = Matrix4x4::identity());

    void _parseLights(libconfig::Config& config,
                      std::vector<std::shared_ptr<ILight>>& lights,
                      const Matrix4x4& parentTransform = Matrix4x4::identity());

    void _parseMaterialInstance(const libconfig::Setting& mat, const std::string& typeName,
                                std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);

    void _parseShapeInstance(const libconfig::Setting& shape, const std::string& typeName,
                             const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                             World& world, const Matrix4x4& parentTransform);

    void _parseLightInstance(const libconfig::Setting& light, const std::string& typeName,
                             std::vector<std::shared_ptr<ILight>>& lights,
                             const Matrix4x4& parentTransform);

    void _parseSubscenes(libconfig::Config& config,
                         std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap,
                         World& world,
                         std::vector<std::shared_ptr<ILight>>& lights,
                         const Matrix4x4& parentTransform,
                         std::set<std::string>& loadingStack,
                         int depth);

    int _validateAASamples(int samples) const;
    int _validateAOSamples(int samples) const;
    std::string _validateAAMethod(const std::string& method) const;

    void _parseLighting(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseBackground(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseAntialiasing(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseThreads(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseToneMapping(const libconfig::Setting& renderer, RendererConfig& config);
};
