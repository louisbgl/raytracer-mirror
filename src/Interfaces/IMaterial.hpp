#pragma once

#include "../DataTypes/Vec3.hpp"
#include "../DataTypes/Ray.hpp"

class HitRecord; // Forward declaration to avoid circular dependency

/**
 * @brief Interface for materials that define how rays interact with surfaces.
 */
class IMaterial {
public:
    virtual ~IMaterial() = default;

    /**
     * @brief [Direct Lighting] Determines how the surface reacts to a specific light source.
     * @details This is called by the Core for EVERY visible light in the scene. 
     * It handles the "Active" lighting (the bright spots and the base color).
     * @param record The data from the hit (normal, point, etc.).
     * @param lightDir The direction vector pointing from the hit point TO the light.
     * @param lightColor The color and intensity of the light source.
     * @return The Vec3 color contribution (how much this light brightens the pixel).
     */
    virtual Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor, const Vec3& viewDir) const = 0;

    /**
     * @brief [Indirect Lighting] Determines if and how light bounces off the surface.
     * @details This is called once per hit to handle "Passive" lighting like mirrors, 
     * glass, or matte reflections. If it returns true, the Core will trace the 'scattered' ray.
     * @param ray_in The incoming ray that hit the surface.
     * @param record The data from the hit.
     * @param attenuation [Out] How much the reflected light is filtered by the material's color.
     * @param scattered [Out] The new ray produced by the bounce.
     * @return true if the ray continues (reflection/refraction), false if it stops (absorbed).
     */
    virtual bool scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const = 0;
};