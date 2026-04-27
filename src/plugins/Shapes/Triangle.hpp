#pragma once

#include "../../Interfaces/IShape.hpp"
#include "../../Math/AABB.hpp"
#include <optional>
#include <array>

struct UV {
    double u = 0.0, v = 0.0;
};

class Triangle : public IShape {
public:
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<IMaterial> material,
             std::optional<std::array<Vec3, 3>> normals = std::nullopt,
             std::optional<std::array<UV, 3>> uvs = std::nullopt);
    ~Triangle() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;
    AABB boundingBox() const override { return _aabb; }

private:
    Vec3 _v0;
    Vec3 _edge1;
    Vec3 _edge2;
    Vec3 _normal;
    std::shared_ptr<IMaterial> _material;
    AABB _aabb;
    std::optional<std::array<Vec3, 3>> _vertexNormals;
    std::optional<std::array<UV, 3>> _vertexUVs;
    double _epsilon = 1e-8;
};
