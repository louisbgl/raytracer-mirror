#include "Rectangle.hpp"
#include <cmath>

Rectangle::Rectangle(Vec3 rotation, Vec3 translation, double width, double height, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _width(width), _height(height), _material(material) {}

bool Rectangle::hitLocal(const Ray& ray, HitRecord& record) const {
    // Rectangle in XY plane at z=0, centered at origin
    if (std::abs(ray.direction().z()) < 1e-6) {
        return false;
    }

    double t = -ray.origin().z() / ray.direction().z();

    if (t < 0) {
        return false;
    }

    Vec3 hit_point = ray.at(t);
    double dx = std::abs(hit_point.x());
    double dy = std::abs(hit_point.y());

    if (dx > _width / 2.0 || dy > _height / 2.0) {
        return false;
    }

    record.point = hit_point;
    record.t = t;
    record.material = _material;
    record.set_face_normal(ray, Vec3(0, 0, 1));

    return true;
}

AABB Rectangle::computeLocalAABB() const {
    Vec3 min(-_width / 2.0, -_height / 2.0, 0);
    Vec3 max(_width / 2.0, _height / 2.0, 0);
    return AABB(min, max);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double width, double height, std::shared_ptr<IMaterial>* material) {
    return new Rectangle(Vec3(rx, ry, rz), Vec3(tx, ty, tz), width, height, *material);
}
