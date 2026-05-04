#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <string>
#include <thread>

#include "PixelBuffer.hpp"
#include "RenderPanel.hpp"
#include "SceneBrowser.hpp"

enum class UIState {
    Browser,
    Rendering,
    Done,
    // FreeRoam
};

class UIApp {
public:
    UIApp();
    ~UIApp();

    int run();

    static constexpr unsigned int WIN_W = 800;
    static constexpr unsigned int WIN_H = 600;

private:
    void handleEvents();
    void update();
    void draw();

    void toRendering(const std::string& scenePath);
    void toBrowser();
    void toDone();

    void spawnRenderThread();
    void joinRenderThread();

    sf::RenderWindow _window;
    sf::Font         _font;
    UIState          _state = UIState::Browser;

    SceneBrowser _browser;
    RenderPanel  _panel;

    std::string       _scenePath;
    PixelBuffer       _pixelBuffer;
    std::thread       _renderThread;
    std::atomic<bool> _cancelFlag{false};
    std::atomic<bool> _doneFlag{false};
};
