#include "Tanglecube.hpp"
#include "../../Math/QuarticSolver.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>
#include <algorithm>

Tanglecube::Tanglecube(Vec3 rotation, Vec3 translation, double scale, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _scale(scale), _material(material) {}

double Tanglecube::evaluate(double x, double y, double z) const {
    double x_scaled = x / _scale;
    double y_scaled = y / _scale;
    double z_scaled = z / _scale;

    double x2 = x_scaled * x_scaled;
    double y2 = y_scaled * y_scaled;
    double z2 = z_scaled * z_scaled;

    double x4 = x2 * x2;
    double y4 = y2 * y2;
    double z4 = z2 * z2;

    return x4 - 5.0 * x2 + y4 - 5.0 * y2 + z4 - 5.0 * z2 + 11.8;
}

Vec3 Tanglecube::gradient(double x, double y, double z) const {
    double x_scaled = x / _scale;
    double y_scaled = y / _scale;
    double z_scaled = z / _scale;

    double inv_scale = 1.0 / _scale;

    double gx = (4.0 * x_scaled * x_scaled * x_scaled - 10.0 * x_scaled) * inv_scale;
    double gy = (4.0 * y_scaled * y_scaled * y_scaled - 10.0 * y_scaled) * inv_scale;
    double gz = (4.0 * z_scaled * z_scaled * z_scaled - 10.0 * z_scaled) * inv_scale;

    return Vec3(gx, gy, gz);
}

bool Tanglecube::hitLocal(const Ray& ray, HitRecord& record) const {
    Vec3 O = ray.origin();
    Vec3 D = ray.direction();

    double ox = O.x(), oy = O.y(), oz = O.z();
    double dx = D.x(), dy = D.y(), dz = D.z();

    double sx = 1.0 / _scale;
    ox *= sx;
    oy *= sx;
    oz *= sx;
    dx *= sx;
    dy *= sx;
    dz *= sx;

    double ox2 = ox * ox, ox3 = ox2 * ox, ox4 = ox2 * ox2;
    double oy2 = oy * oy, oy3 = oy2 * oy, oy4 = oy2 * oy2;
    double oz2 = oz * oz, oz3 = oz2 * oz, oz4 = oz2 * oz2;

    double dx2 = dx * dx, dx3 = dx2 * dx, dx4 = dx2 * dx2;
    double dy2 = dy * dy, dy3 = dy2 * dy, dy4 = dy2 * dy2;
    double dz2 = dz * dz, dz3 = dz2 * dz, dz4 = dz2 * dz2;

    double coeff_4 = dx4 + dy4 + dz4;

    double coeff_3 = 4.0 * (ox * dx3 + oy * dy3 + oz * dz3);

    double coeff_2 = 6.0 * (ox2 * dx2 + oy2 * dy2 + oz2 * dz2)
                   - 5.0 * (dx2 + dy2 + dz2);

    double coeff_1 = 4.0 * (ox3 * dx + oy3 * dy + oz3 * dz)
                   - 10.0 * (ox * dx + oy * dy + oz * dz);

    double coeff_0 = (ox4 - 5.0 * ox2) + (oy4 - 5.0 * oy2) + (oz4 - 5.0 * oz2) + 11.8;

    QuarticRoots roots = QuarticSolver::solve(coeff_0, coeff_1, coeff_2, coeff_3, coeff_4);

    double closest_t = std::numeric_limits<double>::infinity();
    bool found_hit = false;
    const double epsilon = 1e-6;

    for (int i = 0; i < roots.count; ++i) {
        double t = roots.roots[i];
        if (t > epsilon && t < closest_t) {
            closest_t = t;
            found_hit = true;
        }
    }

    if (found_hit) {
        record.t = closest_t;
        record.point = ray.at(closest_t);
        record.material = _material;

        Vec3 outward_normal = gradient(record.point.x(), record.point.y(), record.point.z());
        outward_normal = normalize(outward_normal);
        record.normal = outward_normal;
        record.front_face = true;

        Vec3 d = normalize(record.point);
        double phi = std::atan2(-d.z(), d.x()) + Math::PI;
        double cos_theta = std::clamp(-d.y(), -1.0, 1.0);
        record.u = phi * Math::INV_TWO_PI;
        record.v = std::acos(cos_theta) * Math::INV_PI;

        return true;
    }
    return false;
}

AABB Tanglecube::computeLocalAABB() const {
    double bound = _scale * 2.5;
    Vec3 min(-bound, -bound, -bound);
    Vec3 max(bound, bound, bound);
    return AABB(min, max);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double scale, std::shared_ptr<IMaterial>* material) {
    return new Tanglecube(Vec3(rx, ry, rz), Vec3(tx, ty, tz), scale, *material);
}
