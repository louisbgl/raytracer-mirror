
#include "./Cone.hpp"
#include "DataTypes/Vec3.hpp"

#include <cmath>
#include <iostream>




Cone::Cone(Vec3 pos, double radius, std::shared_ptr<IMaterial>material)
{
    _position = pos;
    _radius = radius;
    _material = material;
    // _height = height;
}

bool Cone::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
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
    double y_min = _position.y() - 2.0; //random unit amount tall
    double y_max = _position.y();
    if (hitpoint.y() < y_min || hitpoint.y() > y_max) {
        return false;
    }

    double cone_r = std::sqrt((hitpoint.x() * hitpoint.x()) + (hitpoint.z() * hitpoint.z()));
    Vec3 outward_norm = Vec3(hitpoint.x(), cone_r * k, hitpoint.z());
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

