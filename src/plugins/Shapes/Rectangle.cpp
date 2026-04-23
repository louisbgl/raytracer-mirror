#include "Rectangle.hpp"
#include <cmath>

Rectangle::Rectangle(Vec3 position, double width, double height, std::shared_ptr<IMaterial> material)
    : _position(position), _width(width), _height(height), _material(material), _normal(0, 0, 1) {}

bool Rectangle::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    // NOTE: Rectangle is currently hard-coded to the XY plane (perpendicular to Z-axis).
    // It only works for axis-aligned rectangles. Full rotation support will be added
    // when the general shape transformation system is implemented.
    double direction_z = ray.direction().z();

    if (std::abs(direction_z) < 1e-6) {
        return false;
    }

    double t = (_position.z() - ray.origin().z()) / direction_z;

    if (t < t_min || t > t_max) {
        return false;
    }

    Vec3 hit_point = ray.at(t);
    double dx = std::abs(hit_point.x() - _position.x());
    double dy = std::abs(hit_point.y() - _position.y());

    if (dx > _width / 2.0 || dy > _height / 2.0) {
        return false;
    }

    record.point = hit_point;
    record.t = t;
    record.material = _material;
    record.set_face_normal(ray, _normal);

    return true;
}

extern "C" IShape* create(double x, double y, double z, double width, double height, std::shared_ptr<IMaterial>* material) {
    return new Rectangle(Vec3(x, y, z), width, height, *material);
}
