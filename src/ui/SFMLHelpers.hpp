#pragma once
#include <SFML/Graphics.hpp>

namespace sfh {

inline void centerXY(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(x, y);
}

inline void centerXTop(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top);
    t.setPosition(x, y);
}

inline void alignRight(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width, b.top + b.height / 2.f);
    t.setPosition(x, y);
}

inline void alignLeft(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left, b.top + b.height / 2.f);
    t.setPosition(x, y);
}

inline void alignLeftTop(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left, b.top);
    t.setPosition(x, y);
}

} // namespace sfh
