#pragma once

#include "../DataTypes/Scene.hpp"
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
    void parseCamera(libconfig::Config& config, Scene& scene);
    void parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);
    void parseShapes(libconfig::Config& config, const std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap, World& world);
    void parseLights(libconfig::Config& config, std::vector<std::shared_ptr<ILight>>& lights);
};
