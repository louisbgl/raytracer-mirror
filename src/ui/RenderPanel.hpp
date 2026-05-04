#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "PixelBuffer.hpp"

// Owns all draw/event logic for Rendering and Done states.
class RenderPanel {
public:
    explicit RenderPanel(sf::Font& font);

    void setScene(const std::string& path);

    void update(const PixelBuffer& buf);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    void drawRendering(sf::RenderWindow& window);
    void drawDone(sf::RenderWindow& window);

    // check after handleEvent, cleared by UIApp
    bool wantsCancel() const { return _sigCancel; }
    bool wantsBack()   const { return _sigBack;   }
    void clearSignals()      { _sigCancel = false; _sigBack = false; }

private:
    void drawButton(sf::RenderWindow& window, sf::FloatRect rect,
                    const std::string& label, sf::Color bg);
    void drawHeader(sf::RenderWindow& window, float ww, float headerH);

    bool wasClicked(const sf::Event& event, const sf::RenderWindow& window,
                    sf::FloatRect rect) const;

    sf::Font&   _font;
    std::string _scenePath;
    int         _rows  = 0;
    int         _total = 0;
    bool        _sigCancel = false;
    bool        _sigBack   = false;

    sf::FloatRect _cancelRect = {};
    sf::FloatRect _backRect   = {};
};
