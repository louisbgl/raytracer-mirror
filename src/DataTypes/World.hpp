#pragma once

#include "Vec3.hpp"
#include "HitRecord.hpp"
#include "../Interfaces/IShape.hpp"
#include <vector>

class World {
public:
    World() = default;
    World(std::vector<std::shared_ptr<IShape>> objects) : _objects(std::move(objects)) {}

    /**
     * @brief Gets the objects in the world.
     * @return A reference to the vector of objects.
     */
    const std::vector<std::shared_ptr<IShape>>& objects() const { return _objects; }

    /**
     * @brief Checks if a certain object is already in the world.
     * @param object The object to check for.
     * @return True if the object is in the world, false otherwise.
     */
    bool has_object(std::shared_ptr<IShape> object) const {
        for (const auto& obj : _objects) {
            if (obj == object) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Adds an object to the world.
     * @param object The object to add.
     */
    void add_object(std::shared_ptr<IShape> object) {
        _objects.push_back(std::move(object));
    }

    /**
     * @brief Removes an object from the world.
     * @param object The object to remove.
     * @return True if the object was removed, false otherwise.
     */
    bool remove_object(std::shared_ptr<IShape> object) {
        for (auto it = _objects.begin(); it != _objects.end(); ++it) {
            if (*it == object) {
                _objects.erase(it);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Gets the closest hit for a given ray.
     * @param ray The ray to check.
     * @param t_min The minimum distance to consider.
     * @param t_max The maximum distance to consider.
     * @param hit [Out] The hit record to fill.
     * @return True if a hit was found, false otherwise.
     */
    bool get_closest_hit(const Ray& ray, double t_min, double t_max, HitRecord& hit) const {
        HitRecord temp_hit(Vec3(0, 0, 0), Vec3(0, 0, 0), 0, false, nullptr);
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& object : _objects) {
            if (object->hit(ray, t_min, closest_so_far, temp_hit)) {
                hit_anything = true;
                closest_so_far = temp_hit.t;
                hit = temp_hit;
            }
        }

        return hit_anything;
    }

private:
    std::vector<std::shared_ptr<IShape>> _objects;
};