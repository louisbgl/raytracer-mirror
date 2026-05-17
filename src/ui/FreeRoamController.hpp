#pragma once

#include <SFML/Graphics.hpp>
#include "DataTypes/Camera.hpp"

/**
 * @brief Controller for free-roam camera navigation at interactive framerates.
 *
 * Handles WASD movement, mouse look (yaw/pitch), and camera state mutation.
 * Designed for real-time preview rendering at 24 fps target.
 */
class FreeRoamController {
public:
    /**
     * @brief Construct controller with initial camera state.
     * @param initialCamera Camera to clone for navigation
     */
    explicit FreeRoamController(const Camera& initialCamera);

    /**
     * @brief Process input events (keyboard, mouse motion).
     * @param event SFML event to process
     * @param window Window for mouse capture/position queries
     */
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);

    /**
     * @brief Update camera state based on accumulated input.
     * @param deltaTime Frame time in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Get current camera state.
     * @return Camera with updated position/look_at/up
     */
    Camera getCamera() const;

    /**
     * @brief Check if camera moved this frame.
     * @return True if position or orientation changed
     */
    bool hasMoved() const { return _dirty; }

    /**
     * @brief Clear dirty flag after handling movement.
     */
    void clearDirtyFlag() { _dirty = false; }

    /**
     * @brief Check if user requested exit (ESC).
     * @return True if exit signal active
     */
    bool wantsExit() const { return _exitFlag; }

    /**
     * @brief Enable mouse capture for look control.
     * @param window Window to capture mouse in
     */
    void enableMouseCapture(sf::RenderWindow& window);

    /**
     * @brief Disable mouse capture, restore cursor.
     * @param window Window to release mouse from
     */
    void disableMouseCapture(sf::RenderWindow& window);

private:
    Vec3  _position;
    Vec3  _up;
    float _yaw;   ///< Horizontal rotation (degrees)
    float _pitch; ///< Vertical rotation (degrees), clamped to ±89°
    int   _width;
    int   _height;
    float _fov;

    bool _moveForward  = false;
    bool _moveBackward = false;
    bool _moveLeft     = false;
    bool _moveRight    = false;
    bool _moveUp       = false;
    bool _moveDown     = false;

    bool _mouseCapture = false;
    sf::Vector2i _lastMousePos;
    bool _firstMouseEvent = true; ///< Skip first event to avoid snap

    bool _dirty    = false; ///< Camera changed this frame
    bool _exitFlag = false; ///< User pressed ESC

    static constexpr float MOVE_SPEED       = 20.0f;   ///< Units per second
    static constexpr float MOUSE_SENSITIVITY = 0.15f; ///< Degrees per pixel
    static constexpr float PITCH_LIMIT      = 89.0f;  ///< Max pitch to avoid gimbal lock

    /// Compute forward vector from yaw/pitch
    Vec3 _computeForward() const;

    /// Compute right vector from yaw/pitch
    Vec3 _computeRight() const;
};
