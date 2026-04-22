#pragma once

#include "../../Interfaces/IShape.hpp"
#include <string>

class Box : public IShape {
public:
    Box(Vec3 position, double width, double height, double depth, const std::string& orientation, std::shared_ptr<IMaterial> material);
    ~Box() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _width;
    double _height;
    double _depth;
    std::string _orientation;
    std::shared_ptr<IMaterial> _material;
};
