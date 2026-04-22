#pragma once

#include "../../Interfaces/ILight.hpp"

class DirectionalLight : public ILight {
public:
    DirectionalLight(Vec3 direction, Vec3 color, double intensity);

    double get_light_data(const Vec3& hit_point, Vec3& direction, Vec3& color) const override;

private:
    Vec3 _direction;
    Vec3 _color;
    double _intensity;
};