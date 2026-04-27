#pragma once

#include <memory>
#include <string>

#include <Interfaces/IShape.hpp>
#include <Math/AABB.hpp>
#include <Math/BVHNode.hpp>

class Mesh : public IShape {
public:
    Mesh(const std::string& filename, std::shared_ptr<IMaterial> material);
    ~Mesh() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;
    AABB boundingBox() const override;

private:
    std::shared_ptr<BVHNode> _bvh;
    AABB _aabb;
};