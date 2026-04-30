#pragma once

#include <memory>
#include <string>
#include <limits>

#include <core/AShape.hpp>
#include <Math/BVHNode.hpp>

class Mesh : public AShape {
public:
    Mesh(const std::string& filename, Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material);
    ~Mesh() override = default;

    bool hitLocal(const Ray& localRay, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    std::shared_ptr<BVHNode> _bvh;
};
