#pragma once

#include "../DataTypes/Scene.hpp"
#include "DataTypes/RendererConfig.hpp"
#include <libconfig.h++>
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
    std::string _currentFile;  // Temporary for error messages

    void _parseRenderer(libconfig::Config& config, Scene& scene);
    void _parseCamera(libconfig::Config& config, Scene& scene);
    void _parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);
    void _parseShapes(libconfig::Config& config, const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap, World& world);
    void _parseLights(libconfig::Config& config, std::vector<std::shared_ptr<ILight>>& lights);

    int _validateAASamples(int samples) const;
    int _validateAOSamples(int samples) const;
    std::string _validateAAMethod(const std::string& method) const;

    void _parseLighting(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseBackground(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseAntialiasing(const libconfig::Setting& renderer, RendererConfig& config);
    void _parseThreads(const libconfig::Setting& renderer, RendererConfig& config);
};
