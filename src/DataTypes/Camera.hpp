#pragma once

#include "Vec3.hpp"
#include "Ray.hpp"

class Camera {
public:
    Camera() : _height(720), _width(1280), _position(0, 0, 0), _look_at(0, 0, -1), _up(0, 1, 0), _fov(90) {}
    Camera(int height, int width, Vec3 position, Vec3 look_at, Vec3 up, float fov)
        : _height(height), _width(width), _position(position),
          _look_at(look_at), _up(up), _fov(fov) {}

    /**
     * @brief Generates a ray from the camera through the viewport at normalized coordinates (u, v).
     * @param u The horizontal coordinate on the viewport (0 to 1).
     * @param v The vertical coordinate on the viewport (0 to 1).
     * @return A Ray originating from the camera position and passing through the specified viewport coordinates.
     */
    Ray getRay(float u, float v) const {
        Vec3 forward = normalize(_look_at - _position);
        Vec3 right = normalize(cross(forward, _up));
        Vec3 up = cross(right, forward);

        float theta = _fov * M_PI / 180.0f;
        float half_height = tan(theta / 2);
        float half_width = (_width / _height) * half_height;

        Vec3 lower_left_corner = _position + forward - half_width * right - half_height * up;
        Vec3 horizontal = 2 * half_width * right;
        Vec3 vertical = 2 * half_height * up;
        Vec3 ray_direction = lower_left_corner + u * horizontal + v * vertical - _position;

        return Ray(_position, ray_direction);
    }

private:
    int _height;
    int _width;
    Vec3 _position;
    Vec3 _look_at;
    Vec3 _up;
    float _fov;
};