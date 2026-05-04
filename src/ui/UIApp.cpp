#include "UIApp.hpp"
#include "core/Core.hpp"
#include "core/PluginManager.hpp"
#include <stdexcept>
#include <thread>

UIApp::UIApp()
    : _window(sf::VideoMode(WIN_W, WIN_H), "Raytracer - Scene Browser", sf::Style::Titlebar | sf::Style::Close)
    , _browser(_font)
    , _panel(_font)
{
    _window.setFramerateLimit(60);
    if (!_font.loadFromFile("assets/fonts/arial.ttf"))
        throw std::runtime_error("Failed to load font");
    PluginManager::instance().initialize();
}

UIApp::~UIApp() {
    joinRenderThread();
}

int UIApp::run() {
    while (_window.isOpen()) {
        handleEvents();
        update();
        draw();
    }
    return 0;
}

void UIApp::handleEvents() {
    sf::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            _window.close();

        switch (_state) {
            case UIState::Browser:   _browser.handleEvent(event, _window); break;
            case UIState::Rendering: _panel.handleEvent(event, _window);   break;
            case UIState::Done:      _panel.handleEvent(event, _window);   break;
        }
    }
}

void UIApp::update() {
    switch (_state) {
        case UIState::Browser:
            if (_browser.wantsLaunch()) {
                toRendering(_browser.selected());
                _browser.clearSignals();
            }
            break;

        case UIState::Rendering:
            _panel.update(_pixelBuffer);
            if (_doneFlag.load()) {
                joinRenderThread();
                toDone();
            } else if (_panel.wantsCancel()) {
                _cancelFlag.store(true);
                joinRenderThread();
                _panel.clearSignals();
                toBrowser();
            }
            break;

        case UIState::Done:
            if (_panel.wantsBack()) {
                _panel.clearSignals();
                toBrowser();
            }
            break;
    }
}

void UIApp::draw() {
    _window.clear(sf::Color(22, 22, 30));

    switch (_state) {
        case UIState::Browser:   _browser.draw(_window);        break;
        case UIState::Rendering: _panel.drawRendering(_window); break;
        case UIState::Done:      _panel.drawDone(_window);      break;
    }

    _window.display();
}

void UIApp::toRendering(const std::string& scenePath) {
    _scenePath = scenePath;
    _panel.setScene(scenePath);
    _pixelBuffer.init(0, 0);
    _cancelFlag.store(false);
    _doneFlag.store(false);
    _state = UIState::Rendering;
    spawnRenderThread();
}

void UIApp::toBrowser() {
    _state = UIState::Browser;
    _browser.refresh();
}

void UIApp::toDone() {
    _state = UIState::Done;
}

void UIApp::spawnRenderThread() {
    _renderThread = std::thread([this]() {
        Core core(_scenePath);
        core.setCancelFlag(&_cancelFlag);
        core.setThreadOverride(std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1));
        core.setProgressTarget(&_pixelBuffer.rowsComplete, &_pixelBuffer.totalRows);
        core.simulate();
        _doneFlag.store(true);
    });
}

void UIApp::joinRenderThread() {
    if (_renderThread.joinable())
        _renderThread.join();
}
