#include "UIApp.hpp"

UIApp::UIApp() {
    _window.create(sf::VideoMode(800, 600), "Raytracer");

}

UIApp::~UIApp() {}

int UIApp::run() {

    while (_window.isOpen()) {
        while (_window.pollEvent(_event)) {
            if (_event.type == sf::Event::Closed) {
                _window.close();
            }
        }

        _window.clear(sf::Color::Red);

        _window.display();
    }
    return 0;
}
