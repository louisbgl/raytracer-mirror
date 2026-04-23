#include "Tanglecube.hpp"
#include "../../Math/QuarticSolver.hpp"
#include <cmath>

Tanglecube::Tanglecube(Vec3 position, double scale, std::shared_ptr<IMaterial> material)
    : _position(position), _scale(scale), _material(material) {}

double Tanglecube::evaluate(double x, double y, double z) const {
    double x_scaled = (x - _position.x()) / _scale;
    double y_scaled = (y - _position.y()) / _scale;
    double z_scaled = (z - _position.z()) / _scale;

    double x2 = x_scaled * x_scaled;
    double y2 = y_scaled * y_scaled;
    double z2 = z_scaled * z_scaled;

    double x4 = x2 * x2;
    double y4 = y2 * y2;
    double z4 = z2 * z2;

    return x4 - 5.0 * x2 + y4 - 5.0 * y2 + z4 - 5.0 * z2 + 11.8;
}

Vec3 Tanglecube::gradient(double x, double y, double z) const {
    double x_scaled = (x - _position.x()) / _scale;
    double y_scaled = (y - _position.y()) / _scale;
    double z_scaled = (z - _position.z()) / _scale;

    double inv_scale = 1.0 / _scale;

    double gx = (4.0 * x_scaled * x_scaled * x_scaled - 10.0 * x_scaled) * inv_scale;
    double gy = (4.0 * y_scaled * y_scaled * y_scaled - 10.0 * y_scaled) * inv_scale;
    double gz = (4.0 * z_scaled * z_scaled * z_scaled - 10.0 * z_scaled) * inv_scale;

    return Vec3(gx, gy, gz);
}

bool Tanglecube::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    Vec3 O = ray.origin() - _position;
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

    double closest_t = t_max;
    bool found_hit = false;

    for (int i = 0; i < roots.count; ++i) {
        double t = roots.roots[i];
        if (t > t_min && t < closest_t) {
            closest_t = t;
            found_hit = true;
        }
    }

    if (found_hit) {
        record.t = closest_t;
        record.point = ray.at(closest_t);
        record.material = _material;

        Vec3 normal = gradient(record.point.x(), record.point.y(), record.point.z());
        normal = normalize(normal);
        record.set_face_normal(ray, normal);

        return true;
    }
    return false;
}

extern "C" IShape* create(double x, double y, double z, double scale, std::shared_ptr<IMaterial>* material) {
    return new Tanglecube(Vec3(x, y, z), scale, *material);
}
