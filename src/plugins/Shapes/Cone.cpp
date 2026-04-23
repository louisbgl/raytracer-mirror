
#include "./Cone.hpp"
#include "DataTypes/Vec3.hpp"

#include <cmath>




Cone::Cone(Vec3 pos, double radius, std::shared_ptr<IMaterial>material)
{
    _position = pos;
    _radius = radius;
    _material = material;
}

bool Cone::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
{
    Vec3 oc = ray.origin() - _position;
    double k2 = _radius * _radius;

    double a = (ray.direction().x() * ray.direction().x()) + (ray.direction().z() * ray.direction().z()) - (k2 * ray.direction().y() * ray.direction().y());
    double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z() - k2 * oc.y() * ray.direction().y());
    double c = (oc.x() * oc.x()) + (oc.z() * oc.z()) - (k2 * oc.y() * oc.y());

    double discriminant = (b * b) - 4 * a * c;
    if (discriminant < 0) { 
        return false;
    }

    double disc_sqrd = std::sqrt(discriminant);
    double t = (-b - disc_sqrd) / (2.0 * a);
    
    if (t < t_min || t > t_max) {
        t = (-b + disc_sqrd) / (2.0 * a);
        if (t < t_min || t > t_max) { 
            return false;
        }
    }

    Vec3 hitpoint = ray.at(t);
    
    // Gonna assume the "bottom" part of the hourglass for the basic cone
    if (hitpoint.y() > _position.y()) {
        return false;
    } 

    double rel_x = hitpoint.x() - _position.x();
    double rel_z = hitpoint.z() - _position.z();
    
    double cone_r = std::sqrt(rel_x * rel_x + rel_z * rel_z);
    Vec3 outward_norm = Vec3(rel_x, cone_r * _radius, rel_z);
    outward_norm = normalize(outward_norm);

    record.t = t;
    record.point = hitpoint;
    record.material = _material;
    record.set_face_normal(ray, outward_norm);

    return true;
}

extern "C" IShape* create(double x, double y, double z, double radius, std::shared_ptr<IMaterial>* material) {
    return new Cone(Vec3(x, y, z), radius, *material);
}

