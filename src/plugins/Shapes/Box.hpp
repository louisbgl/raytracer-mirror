#pragma once

#include "../../Interfaces/IShape.hpp"

// Axis-aligned box shape. Full rotation support will be added
// when the general shape transformation system is implemented.
class Box : public IShape {
public:
    Box(Vec3 position, double width, double height, double depth, std::shared_ptr<IMaterial> material);
    ~Box() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _width;
    double _height;
    double _depth;
    std::shared_ptr<IMaterial> _material;
};
