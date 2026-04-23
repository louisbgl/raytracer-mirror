#include "Box.hpp"
#include <cmath>

Box::Box(Vec3 position, double width, double height, double depth, std::shared_ptr<IMaterial> material)
    : _position(position), _width(width), _height(height), _depth(depth), _material(material) {}

bool Box::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    double closest_t = t_max;
    Vec3 normal;
    bool found_hit = false;

    Vec3 min_corner = _position;
    Vec3 max_corner = _position + Vec3(_width, _height, _depth);

    // Test X = min_x plane
    if (std::abs(ray.direction().x()) > 1e-6) {
        double t = (min_corner.x() - ray.origin().x()) / ray.direction().x();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.y() >= min_corner.y() && hit_point.y() <= max_corner.y() &&
                hit_point.z() >= min_corner.z() && hit_point.z() <= max_corner.z()) {
                closest_t = t;
                normal = Vec3(-1, 0, 0);
                found_hit = true;
            }
        }
    }

    // Test X = max_x plane
    if (std::abs(ray.direction().x()) > 1e-6) {
        double t = (max_corner.x() - ray.origin().x()) / ray.direction().x();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.y() >= min_corner.y() && hit_point.y() <= max_corner.y() &&
                hit_point.z() >= min_corner.z() && hit_point.z() <= max_corner.z()) {
                closest_t = t;
                normal = Vec3(1, 0, 0);
                found_hit = true;
            }
        }
    }

    // Test Y = min_y plane
    if (std::abs(ray.direction().y()) > 1e-6) {
        double t = (min_corner.y() - ray.origin().y()) / ray.direction().y();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.x() >= min_corner.x() && hit_point.x() <= max_corner.x() &&
                hit_point.z() >= min_corner.z() && hit_point.z() <= max_corner.z()) {
                closest_t = t;
                normal = Vec3(0, -1, 0);
                found_hit = true;
            }
        }
    }

    // Test Y = max_y plane
    if (std::abs(ray.direction().y()) > 1e-6) {
        double t = (max_corner.y() - ray.origin().y()) / ray.direction().y();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.x() >= min_corner.x() && hit_point.x() <= max_corner.x() &&
                hit_point.z() >= min_corner.z() && hit_point.z() <= max_corner.z()) {
                closest_t = t;
                normal = Vec3(0, 1, 0);
                found_hit = true;
            }
        }
    }

    // Test Z = min_z plane
    if (std::abs(ray.direction().z()) > 1e-6) {
        double t = (min_corner.z() - ray.origin().z()) / ray.direction().z();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.x() >= min_corner.x() && hit_point.x() <= max_corner.x() &&
                hit_point.y() >= min_corner.y() && hit_point.y() <= max_corner.y()) {
                closest_t = t;
                normal = Vec3(0, 0, -1);
                found_hit = true;
            }
        }
    }

    // Test Z = max_z plane
    if (std::abs(ray.direction().z()) > 1e-6) {
        double t = (max_corner.z() - ray.origin().z()) / ray.direction().z();
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            if (hit_point.x() >= min_corner.x() && hit_point.x() <= max_corner.x() &&
                hit_point.y() >= min_corner.y() && hit_point.y() <= max_corner.y()) {
                closest_t = t;
                normal = Vec3(0, 0, 1);
                found_hit = true;
            }
        }
    }

    if (found_hit) {
        record.t = closest_t;
        record.point = ray.at(closest_t);
        record.material = _material;
        record.set_face_normal(ray, normal);
        return true;
    }
    return false;
}

extern "C" IShape* create(double x, double y, double z, double width, double height, double depth, std::shared_ptr<IMaterial>* material) {
    return new Box(Vec3(x, y, z), width, height, depth, *material);
}
