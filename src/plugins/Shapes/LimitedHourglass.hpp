
#pragma once

#include "../../Interfaces/IShape.hpp"

class LimitedHourglass : public IShape {
public:
    LimitedHourglass(Vec3 pos, double radius, double height, std::shared_ptr<IMaterial> material);
    ~LimitedHourglass() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _radius;
    double _height;
    std::shared_ptr<IMaterial> _material;
};
