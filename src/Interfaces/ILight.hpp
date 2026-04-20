#pragma once

#include "../DataTypes/Vec3.hpp"

/**
 * @brief Interface for light sources in the scene.
 */
class ILight {
    public:
        virtual ~ILight() = default;

        /**
         * @brief Calculates the light's properties relative to a specific point in space.
         * @param hit_point The point on a surface that we are currently shading.
         * @param direction [Out] The normalized vector pointing from the hit_point TO the light.
         * @param color [Out] The color and intensity of the light at that point (considering distance/attenuation).
         * @return The distance to the light source.
         */
        virtual double get_light_data(const Vec3& hit_point, Vec3& direction, Vec3& color) const = 0;

    protected:
        ILight() = default;
};