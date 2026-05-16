#include "FreeRoamController.hpp"
#include <cmath>

FreeRoamController::FreeRoamController(const Camera& initialCamera)
    : _position(initialCamera.getPosition())
    , _up(normalize(initialCamera.getUp()))
    , _width(static_cast<int>(initialCamera.getWidth()))
    , _height(static_cast<int>(initialCamera.getHeight()))
    , _fov(initialCamera.getFov())
{
    Vec3 forward = normalize(initialCamera.getLookAt() - initialCamera.getPosition());

    // Build orthonormal basis
    Vec3 arbitraryVec = (fabs(_up.y()) < 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 basisRight = normalize(cross(arbitraryVec, _up));
    Vec3 basisForward = normalize(cross(_up, basisRight));

    // Project forward onto plane perpendicular to up
    Vec3 forwardFlat = normalize(forward - _up * dot(forward, _up));

    _yaw = atan2(dot(forwardFlat, basisRight), dot(forwardFlat, basisForward)) * 180.0f / M_PI;
    _pitch = asin(dot(forward, _up)) * 180.0f / M_PI;
}

void FreeRoamController::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Z:      _moveForward  = true; break;
            case sf::Keyboard::S:      _moveBackward = true; break;
            case sf::Keyboard::Q:      _moveLeft     = true; break;
            case sf::Keyboard::D:      _moveRight    = true; break;
            case sf::Keyboard::Space:  _moveUp       = true; break;
            case sf::Keyboard::LShift: _moveDown     = true; break;
            case sf::Keyboard::Escape: _exitFlag     = true; break;
            default: break;
        }
    } else if (event.type == sf::Event::KeyReleased) {
        switch (event.key.code) {
            case sf::Keyboard::Z:      _moveForward  = false; break;
            case sf::Keyboard::S:      _moveBackward = false; break;
            case sf::Keyboard::Q:      _moveLeft     = false; break;
            case sf::Keyboard::D:      _moveRight    = false; break;
            case sf::Keyboard::Space:  _moveUp       = false; break;
            case sf::Keyboard::LShift: _moveDown     = false; break;
            default: break;
        }
    } else if (event.type == sf::Event::MouseMoved && _mouseCapture) {
        sf::Vector2i currentPos = sf::Mouse::getPosition(window);
        sf::Vector2i delta = currentPos - _lastMousePos;

        // Ignore synthetic events from setPosition (delta == 0)
        if (delta.x == 0 && delta.y == 0)
            return;

        // Skip first event to avoid snap from initial centering
        if (_firstMouseEvent) {
            _firstMouseEvent = false;
            sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
            sf::Mouse::setPosition(center, window);
            _lastMousePos = center;
            return;
        }

        _yaw   += delta.x * MOUSE_SENSITIVITY;
        _pitch -= delta.y * MOUSE_SENSITIVITY;

        // Clamp pitch to avoid gimbal lock
        if (_pitch > PITCH_LIMIT)  _pitch = PITCH_LIMIT;
        if (_pitch < -PITCH_LIMIT) _pitch = -PITCH_LIMIT;

        // Re-center cursor to prevent hitting window edges
        sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
        sf::Mouse::setPosition(center, window);
        _lastMousePos = center;

        _dirty = true;
    }
}

void FreeRoamController::update(float deltaTime) {
    bool hasMovement = _moveForward || _moveBackward || _moveLeft || _moveRight || _moveUp || _moveDown;

    if (!hasMovement)
        return;

    Vec3 forward = _computeForward();
    Vec3 right   = _computeRight();

    Vec3 movement(0, 0, 0);

    if (_moveForward)  movement = movement + forward;
    if (_moveBackward) movement = movement - forward;
    if (_moveRight)    movement = movement + right;
    if (_moveLeft)     movement = movement - right;
    if (_moveUp)       movement = movement + _up;
    if (_moveDown)     movement = movement - _up;

    // Normalize to avoid faster diagonal movement
    double movementLen = sqrt(movement.x() * movement.x() +
                             movement.y() * movement.y() +
                             movement.z() * movement.z());
    if (movementLen > 0.001) {
        movement = movement * (1.0 / movementLen);
    }

    _position = _position + movement * MOVE_SPEED * deltaTime;
    _dirty = true;
}

Camera FreeRoamController::getCamera() const {
    Vec3 forward = _computeForward();
    Vec3 lookAt = _position + forward;

    return Camera(_height, _width, _position, lookAt, _up, _fov);
}

void FreeRoamController::enableMouseCapture(sf::RenderWindow& window) {
    _mouseCapture = true;
    _firstMouseEvent = true;
    // Center cursor on entry
    sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
    sf::Mouse::setPosition(center, window);
    _lastMousePos = center;
    window.setMouseCursorVisible(false);
}

void FreeRoamController::disableMouseCapture(sf::RenderWindow& window) {
    _mouseCapture = false;
    window.setMouseCursorVisible(true);
}

Vec3 FreeRoamController::_computeForward() const {
    float yawRad   = _yaw   * M_PI / 180.0f;
    float pitchRad = _pitch * M_PI / 180.0f;

    // Build orthonormal basis with arbitrary up
    Vec3 arbitraryVec = (fabs(_up.y()) < 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 right = normalize(cross(arbitraryVec, _up));
    Vec3 realForward = normalize(cross(_up, right));

    // Rotate around up (yaw) and right (pitch)
    Vec3 forward = realForward * cos(yawRad) + right * sin(yawRad);
    forward = forward * cos(pitchRad) + _up * sin(pitchRad);

    return normalize(forward);
}

Vec3 FreeRoamController::_computeRight() const {
    Vec3 forward = _computeForward();
    return normalize(cross(forward, _up));
}
