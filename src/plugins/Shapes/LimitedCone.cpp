
#include "./LimitedCone.hpp"
#include "DataTypes/Vec3.hpp"

#include <cmath>
#include <iostream>



LimitedCone::LimitedCone(Vec3 pos, double radius, double height, std::shared_ptr<IMaterial>material)
{
    _position = pos;
    _radius = radius;
    _material = material;
    _height = height;
}

bool LimitedCone::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
{
    Vec3 oc = ray.origin() - _position;
    
    double k = _radius; //k = slope of a cone.
    double k_sqrd = k * k;

    double a = (ray.direction().x() * ray.direction().x()) + (ray.direction().z() * ray.direction().z()) - (k_sqrd * ray.direction().y() * ray.direction().y());
    double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z() - k_sqrd * oc.y() * ray.direction().y());
    double c = (oc.x() * oc.x()) + (oc.z() * oc.z()) - (k_sqrd * oc.y() * oc.y());

    double discriminant = ( b * b) - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }

    double disc_sqrd = std::sqrt(discriminant);
    double t = (-b - disc_sqrd) / (2.0 * a);
    if (t < t_min || t > t_max) {
        t = (-b + disc_sqrd) / (2.0 * a);
        if (t < t_min || t> t_max) {
            return false;
        }
    }

    Vec3 hitpoint = ray.at(t);
    double y_min = _position.y() - _height; 
    double y_max = _position.y();
    if (hitpoint.y() < y_min || hitpoint.y() > y_max) {
        return false;
    }

    double rel_x = hitpoint.x() - _position.x();
    double rel_z = hitpoint.z() - _position.z();
    double relative_y = hitpoint.y() - _position.y();

    double cone_r = std::sqrt((rel_x * rel_x) + (rel_z * rel_z));
    double norm_y = (relative_y > 0) ? -cone_r * k : cone_r * k;
    Vec3 outward_norm = Vec3(rel_x, norm_y, rel_z);
    outward_norm = normalize(outward_norm);

    record.t = t;
    record.point = hitpoint;
    record.material = _material;
    record.set_face_normal(ray, outward_norm);

    return true;
}

extern "C" IShape* create(double x, double y, double z, double radius, double height, std::shared_ptr<IMaterial>* material) {
    return new LimitedCone(Vec3(x, y, z), radius, height, *material);
}

