#pragma once

#include "World.hpp"
#include "Camera.hpp"
#include "RendererConfig.hpp"
#include "../Interfaces/ILight.hpp"

class Scene {
public:
    Scene() = default;
    Scene(World world, Camera camera, std::vector<std::shared_ptr<ILight>> lights)
        : _world(std::move(world)), _camera(std::move(camera)), _lights(std::move(lights)) {}

    /**
     * @brief Gets the world of the scene.
     * @return A reference to the world.
     */
    const World& world() const { return _world; }

    /**
     * @brief Gets the world of the scene.
     * @return A reference to the world.
     */
    World& world() { return _world; }

    /**
     * @brief Gets the camera of the scene.
     * @return A reference to the camera.
     */
    const Camera& camera() const { return _camera; }

    /**
     * @brief Gets the camera of the scene.
     * @return A reference to the camera.
     */
    Camera& camera() { return _camera; }

    /**
     * @brief Gets the lights of the scene.
     * @return A reference to the vector of lights.
     */
    const std::vector<std::shared_ptr<ILight>>& lights() const { return _lights; }

    /**
     * @brief Gets the number of materials in the scene.
     * @return The material count.
     */
    int materialCount() const { return _materialCount; }

    /**
     * @brief Sets the number of materials in the scene.
     * @param n The material count.
     */
    void setMaterialCount(int n) { _materialCount = n; }

    /**
     * @brief Sets the world of the scene.
     * @param world The world to set.
     */
    void set_world(const World& world) { _world = world; }

    /**
     * @brief Sets the camera of the scene.
     * @param camera The camera to set.
     */
    void set_camera(const Camera& camera) { _camera = camera; }

    /**
     * @brief Sets the camera of the scene (alias for consistency).
     * @param camera The camera to set.
     */
    void setCamera(const Camera& camera) { _camera = camera; }

    /**
     * @brief Gets the RendererConfig of the scene.
     * @return A reference to the RendererConfig.
     */
    const RendererConfig& rendererConfig() const { return _rendererConfig; }

    /**
     * @brief Sets the RendererConfig of the scene.
     * @param config The RendererConfig to set.
     */
    void setRendererConfig(const RendererConfig& config) { _rendererConfig = config; }

    /**
     * @brief Checks if the scene has a specific light.
     * @param light The light to check for.
     * @return True if the scene has the light, false otherwise.
     */
    bool has_light(std::shared_ptr<ILight> light) const {
        for (const auto& l : _lights) {
            if (l == light) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Adds a light to the scene.
     * @param light The light to add.
     */
    void add_light(std::shared_ptr<ILight> light) {
        _lights.push_back(std::move(light));
    }

    /**
     * @brief Removes a light from the scene.
     * @param light The light to remove.
     * @return True if the light was removed, false otherwise.
     */
    bool remove_light(std::shared_ptr<ILight> light) {
        for (auto it = _lights.begin(); it != _lights.end(); ++it) {
            if (*it == light) {
                _lights.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    World _world;
    Camera _camera;
    std::vector<std::shared_ptr<ILight>> _lights;
    int    _materialCount = 0;
    RendererConfig _rendererConfig;
};