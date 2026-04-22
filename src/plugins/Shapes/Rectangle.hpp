#pragma once

#include "../../Interfaces/IShape.hpp"

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
