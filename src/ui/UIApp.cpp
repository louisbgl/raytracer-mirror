#include "UIApp.hpp"
#include "core/Core.hpp"
#include "core/PluginManager.hpp"
#include <stdexcept>
#include <thread>

UIApp::UIApp()
    : _window(sf::VideoMode(WIN_W, WIN_H), "Raytracer - Scene Browser", sf::Style::Titlebar | sf::Style::Close)
    , _browser(_font)
    , _panel(_font)
    , _toastManager(_font)
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
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - _lastFrameTime;
        _lastFrameTime = now;

        handleEvents();
        update();
        _toastManager.update(delta.count());
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
            checkSceneFileWatch();
            
            if (_doneFlag.load()) {
                joinRenderThread();
                pushToast("Render complete.", ToastType::Success);
                toDone();
            } else if (_panel.wantsCancel()) {
                _cancelFlag.store(true);
                joinRenderThread();
                _panel.clearSignals();
                pushToast("Render cancelled.", ToastType::Warning);
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

    // Update reload indicator for rendering state
    if (_state == UIState::Rendering) {
        _panel.setShowReloadIndicator(_showReloadIndicator);
    }

    switch (_state) {
        case UIState::Browser:   _browser.draw(_window);        break;
        case UIState::Rendering: _panel.drawRendering(_window); break;
        case UIState::Done:      _panel.drawDone(_window);      break;
    }

    _toastManager.draw(_window);
    _window.display();
}

void UIApp::toRendering(const std::string& scenePath) {
    _scenePath = scenePath;
    _panel.setScene(scenePath);
    _pixelBuffer.init(0, 0);
    _cancelFlag.store(false);
    _doneFlag.store(false);
    
    _fileWatcher = std::make_unique<FileWatcher>();
    try {
        _fileWatcher->addTargetPath(scenePath);
        _lastWatchCheck = std::chrono::steady_clock::now();
        _lastReloadTime = std::chrono::steady_clock::now();
        _showReloadIndicator = false;
    } catch (const std::exception& e) {
        _fileWatcher = nullptr;
    }
    
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

void UIApp::pushToast(const std::string& message, ToastType type) {
    ToastConfig config;
    config.msg = message;
    config.type = type;
    config.duration = 3.0F;
    _toastManager.push(config);
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

void UIApp::checkSceneFileWatch() {
    if (!_fileWatcher) return;
    
    auto now = std::chrono::steady_clock::now();
    if (now - _lastWatchCheck < WATCH_THROTTLE) {
        if (_showReloadIndicator && (now - _lastReloadTime > std::chrono::milliseconds(500))) {
            _showReloadIndicator = false;
        }
        return;
    }
    
    _lastWatchCheck = now;
    
    if (!_fileWatcher->watchFileEvents()) { return; }
    
    // Debounce: ignore if last reload was too recent (< 500ms)
    if (now - _lastReloadTime < RELOAD_DEBOUNCE) {
        _fileWatcher->clearChangeFlag();
        return;
    }
    
    _lastReloadTime = now;
    _showReloadIndicator = true;
    pushToast("Scene reloaded.", ToastType::Info);
    
    _cancelFlag.store(true);
    joinRenderThread();
    
    _pixelBuffer.init(0, 0);
    _cancelFlag.store(false);
    _doneFlag.store(false);
    _fileWatcher->clearChangeFlag();
    
    spawnRenderThread();
}
