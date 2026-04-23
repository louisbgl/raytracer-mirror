#pragma once

#include "../../Interfaces/IShape.hpp"

// Rectangle shape: axis-aligned to the XY plane (perpendicular to Z-axis).
// NOTE: Currently hard-coded to Z-axis orientation. Full rotation support will be added
// when the general shape transformation system is implemented.
class Rectangle : public IShape {
public:
    Rectangle(Vec3 position, double width, double height, std::shared_ptr<IMaterial> material);
    ~Rectangle() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _width;
    double _height;
    std::shared_ptr<IMaterial> _material;
    Vec3 _normal;
};
