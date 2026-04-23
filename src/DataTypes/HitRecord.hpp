#pragma once

#include <memory>
#include "Ray.hpp"
#include "../Interfaces/IMaterial.hpp"

class HitRecord {
public:
    HitRecord() = default;
    HitRecord(Vec3 point, Vec3 normal, double t, bool front_face, std::shared_ptr<IMaterial> material)
        : point(point), normal(normal), t(t), front_face(front_face), material(std::move(material)) {}

    Vec3 point;
    Vec3 normal;
    double t;
    bool front_face;
    std::shared_ptr<IMaterial> material;
    double u = 0.0;
    double v = 0.0;

    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};