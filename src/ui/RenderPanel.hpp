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

    void setShowReloadIndicator(bool show) { _showReloadIndicator = show; }

    // check after handleEvent, cleared by UIApp
    bool wantsCancel() const { return _sigCancel; }
    bool wantsBack()   const { return _sigBack;   }
    void clearSignals()      { _sigCancel = false; _sigBack = false; }

private:
    void drawButton(sf::RenderWindow& window, sf::FloatRect rect, const std::string& label, sf::Color bg);

    void drawHeader(sf::RenderWindow& window, float ww, float headerH);
    void drawImageFitted(sf::RenderWindow& window, float x, float y, float w, float h);
    void drawReloadIndicator(sf::RenderWindow& window, float ww, float wh);

    void drawRenderingTitle(sf::RenderWindow& window, float ww, float cx, float headerH);
    void drawLiveArea(sf::RenderWindow& window, float ww, float cx, float imgY, float imgH);
    void drawProgressStrip(sf::RenderWindow& window, float stripY, float ww, float stripH);
    void drawCancelButton(sf::RenderWindow& window, float ww, float stripY, float controlsH);
    void drawLiveToggle(sf::RenderWindow& window, float ww, float stripY, float controlsH);

    void drawDoneTitle(sf::RenderWindow& window, float cx, float headerH);
    void drawDoneStrip(sf::RenderWindow& window, float ww, float cx, float stripY, float controlsH);

    bool wasClicked(const sf::Event& event, const sf::RenderWindow& window, sf::FloatRect rect) const;

    sf::Font& _font;
    std::string _scenePath;
    int _rows  = 0;
    int _total = 0;
    bool _sigCancel = false;
    bool _sigBack   = false;
    bool _showReloadIndicator = false;
    bool _liveMode  = true;
    bool _hasTexture = false;

    sf::Texture _renderTex;
    sf::Sprite _renderSprite;

    sf::FloatRect _cancelRect = {};
    sf::FloatRect _backRect = {};
    sf::FloatRect _liveRect = {};
};
