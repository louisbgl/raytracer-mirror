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
        float half_width = (static_cast<float>(_width) / static_cast<float>(_height)) * half_height;

        Vec3 lower_left_corner = _position + forward - half_width * right - half_height * up;
        Vec3 horizontal = 2 * half_width * right;
        Vec3 vertical = 2 * half_height * up;
        Vec3 ray_direction = lower_left_corner + u * horizontal + v * vertical - _position;

        return Ray(_position, ray_direction);
    }

    /**
     * @brief Gets the height of the camera resolution.
     * @return The height in pixels.
     */
    double getHeight() const { return _height; }

    /**
     * @brief Gets the width of the camera resolution.
     * @return The width in pixels.
     */
    double getWidth() const { return _width; }

    /**
     * @brief Gets the position of the camera in world space.
     * @return The camera position as a Vec3.
     */
    Vec3 getPosition() const { return _position; }

    /**
     * @brief Gets the look-at point of the camera.
     * @return The look-at target as a Vec3.
     */
    Vec3 getLookAt() const { return _look_at; }

    /**
     * @brief Gets the up vector of the camera.
     * @return The up direction as a Vec3.
     */
    Vec3 getUp() const { return _up; }

    /**
     * @brief Gets the field of view of the camera.
     * @return The field of view in degrees.
     */
    float getFov() const { return _fov; }

    /**
     *  @brief Sets the width of the camera resolution.
     *  @param width The new width in pixels.
     */
    void setWidth(int width) { _width = width; }

    /**
     *  @brief Sets the height of the camera resolution.
     *  @param height The new height in pixels.
     */
    void setHeight(int height) { _height = height; }

private:
    int _height;
    int _width;
    Vec3 _position;
    Vec3 _look_at;
    Vec3 _up;
    float _fov;
};