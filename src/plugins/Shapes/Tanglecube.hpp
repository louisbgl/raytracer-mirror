#pragma once

#include "../../Interfaces/IShape.hpp"

class Tanglecube : public IShape {
public:
    Tanglecube(Vec3 position, double scale, std::shared_ptr<IMaterial> material);
    ~Tanglecube() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _scale;
    std::shared_ptr<IMaterial> _material;

    double evaluate(double x, double y, double z) const;
    Vec3 gradient(double x, double y, double z) const;
};
