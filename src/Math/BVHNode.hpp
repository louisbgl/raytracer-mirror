#pragma once

#include "AABB.hpp"
#include "../Interfaces/IShape.hpp"
#include "../plugins/Shapes/Triangle.hpp"
#include <vector>
#include <memory>
#include <algorithm>

class BVHNode : public IShape {
public:
    BVHNode(std::vector<std::shared_ptr<Triangle>>& triangles, size_t start, size_t end) {
        size_t count = end - start;

        if (count == 1) {
            _left = _right = triangles[start];
        } else if (count == 2) {
            _left  = triangles[start];
            _right = triangles[start + 1];
        } else {
            AABB centroidBox = triangles[start]->boundingBox();
            for (size_t i = start; i < end; i++)
                centroidBox = AABB::surrounding_box(centroidBox, triangles[i]->boundingBox());

            Vec3 span = centroidBox.max() - centroidBox.min();
            int axis = 0;
            if (span.y() > span.x()) axis = 1;
            if (span.z() > span[axis]) axis = 2;

            std::sort(triangles.begin() + start, triangles.begin() + end,
                [axis](const std::shared_ptr<Triangle>& a, const std::shared_ptr<Triangle>& b) {
                    return a->boundingBox().min()[axis] < b->boundingBox().min()[axis];
                });

            size_t mid = start + count / 2;
            _left  = std::make_shared<BVHNode>(triangles, start, mid);
            _right = std::make_shared<BVHNode>(triangles, mid, end);
        }

        _box = AABB::surrounding_box(_left->boundingBox(), _right->boundingBox());
    }

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override {
        if (!_box.hit(ray, t_min, t_max))
            return false;

        bool hitLeft  = _left->hit(ray, t_min, t_max, record);
        bool hitRight = _right->hit(ray, t_min, hitLeft ? record.t : t_max, record);

        return hitLeft || hitRight;
    }

    AABB boundingBox() const override { return _box; }

private:
    std::shared_ptr<IShape> _left;
    std::shared_ptr<IShape> _right;
    AABB _box;
};
