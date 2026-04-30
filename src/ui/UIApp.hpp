#pragma once

#include <SFML/Graphics.hpp>

class UIApp {
public:
    UIApp();
    ~UIApp();

    int run();

private:
    sf::RenderWindow _window;
    sf::Event _event;
};