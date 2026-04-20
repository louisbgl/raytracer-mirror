#pragma once

#include "../../Interfaces/ILight.hpp"

class PointLight : public ILight {
public:
    PointLight(Vec3 position, Vec3 color);

    double get_light_data(const Vec3& hit_point, Vec3& direction, Vec3& color) const override;

private:
    Vec3 _position;
    Vec3 _color;
};