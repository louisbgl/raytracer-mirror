#pragma once

#include "../Interfaces/IShape.hpp"
#include "../core/PluginLoader.hpp"
#include <libconfig.h++>
#include <memory>
#include <string>
#include <unordered_map>

class ShapeFactory {
public:
    /**
     * @brief Creates a shape based on type and configuration.
     * @param type The type of shape to create (e.g., "sphere", "plane").
     * @param config The libconfig Setting containing shape parameters.
     * @param material The material to apply to the shape.
     * @return A shared pointer to the created shape, or nullptr if type is unknown.
     */
    static std::shared_ptr<IShape> create(
        const std::string& type,
        const libconfig::Setting& config,
        std::shared_ptr<IMaterial> material
    );

private:
    using ShapeCreator = std::shared_ptr<IShape>(*)(const libconfig::Setting&, std::shared_ptr<IMaterial>);

    static Vec3 _getRotation(const libconfig::Setting& config);

    // Bounded shapes (AShape - support rotation)
    static std::shared_ptr<IShape> _createSphere           (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createLimitedCylinder  (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createLimitedCone      (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createLimitedHourglass (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createRectangle        (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createBox              (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createTorus            (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createTanglecube       (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);

    // Infinite shapes (IShape - shape-specific orientation)
    static std::shared_ptr<IShape> _createPlane            (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createCylinder         (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createCone             (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createHourglass        (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
    static std::shared_ptr<IShape> _createTriangle         (const libconfig::Setting& config, std::shared_ptr<IMaterial> material);
};
