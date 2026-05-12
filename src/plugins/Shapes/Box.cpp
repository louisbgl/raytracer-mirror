#include "Box.hpp"
#include "../PluginMetadata.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>

Box::Box(Vec3 rotation, Vec3 translation, Vec3 scale, double width, double height, double depth, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation, scale, material), _width(width), _height(height), _depth(depth) {}

bool Box::hitLocal(const Ray& ray, HitRecord& record) const {
    Vec3 min_corner(-_width / 2.0, -_height / 2.0, -_depth / 2.0);
    Vec3 max_corner(_width / 2.0, _height / 2.0, _depth / 2.0);

    double t_min = 0.0;
    double t_max = std::numeric_limits<double>::infinity();
    int hit_axis = -1;
    bool hit_max_face = false;

    // Slab method: intersect ray with each pair of parallel planes
    for (int axis = 0; axis < 3; axis++) {
        if (std::abs(ray.direction()[axis]) < 1e-8) {
            // Ray parallel to slab - check if origin is inside
            if (ray.origin()[axis] < min_corner[axis] || ray.origin()[axis] > max_corner[axis]) {
                return false;
            }
            continue;
        }

        double invD = 1.0 / ray.direction()[axis];
        double t0 = (min_corner[axis] - ray.origin()[axis]) * invD;
        double t1 = (max_corner[axis] - ray.origin()[axis]) * invD;

        bool swapped = false;
        if (invD < 0.0) {
            std::swap(t0, t1);
            swapped = true;
        }

        if (t0 > t_min) {
            t_min = t0;
            hit_axis = axis;
            hit_max_face = swapped;
        }
        if (t1 < t_max) {
            t_max = t1;
        }

        if (t_max <= t_min) {
            return false;
        }
    }

    // Check if intersection is behind ray origin
    if (t_min < 0) {
        return false;
    }

    // Compute normal based on which face was hit
    Vec3 normal(0, 0, 0);
    if (hit_axis == 0) {
        normal = hit_max_face ? Vec3(1, 0, 0) : Vec3(-1, 0, 0);
    } else if (hit_axis == 1) {
        normal = hit_max_face ? Vec3(0, 1, 0) : Vec3(0, -1, 0);
    } else {
        normal = hit_max_face ? Vec3(0, 0, 1) : Vec3(0, 0, -1);
    }

    record.t = t_min;
    record.point = ray.at(t_min);
    record.material = _material;
    record.set_face_normal(ray, normal);

    Vec3 hit = record.point;
    if (hit_axis == 0) {
        record.u = (hit.z() + _depth / 2.0) / _depth;
        record.v = (hit.y() + _height / 2.0) / _height;
    } else if (hit_axis == 1) {
        record.u = (hit.x() + _width / 2.0) / _width;
        record.v = (hit.z() + _depth / 2.0) / _depth;
    } else {
        record.u = (hit.x() + _width / 2.0) / _width;
        record.v = (hit.y() + _height / 2.0) / _height;
    }

    return true;
}

AABB Box::computeLocalAABB() const {
    Vec3 min(-_width / 2.0, -_height / 2.0, -_depth / 2.0);
    Vec3 max(_width / 2.0, _height / 2.0, _depth / 2.0);
    return AABB(min, max);
}

extern "C" IShape* create(
    Vec3C rotation,
    Vec3C translation,
    Vec3C scale,
    double width, double height,
    double depth,
    std::shared_ptr<IMaterial>* material
) {
    return new Box(
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
        width, height,
        depth,
        *material
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "box",
        .pluralForm = "boxes",
        .helpText = "Box (position (x, y, z), width, height, depth, material, [rotation (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}
