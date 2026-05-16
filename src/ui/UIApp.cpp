#include "UIApp.hpp"
#include "core/Core.hpp"
#include "core/PluginManager.hpp"
#include <cstdint>
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
    cancelPreview();
    joinRenderThread();
}

int UIApp::run() {
    while (_window.isOpen()) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - _lastFrameTime;
        _lastFrameTime = now;

        handleEvents();
        update(delta.count());
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
            case UIState::FreeRoam:
                if (_freeRoamController)
                    _freeRoamController->handleEvent(event, _window);
                break;
        }
    }
}

void UIApp::update(float deltaTime) {
    switch (_state) {
        case UIState::Browser:
            if (_browser.selectionChanged()) {
                cancelPreview();
                if (_browser.hasSelection())
                    spawnPreviewThread();
                _browser.clearSignals();
            } else if (_browser.wantsLaunch()) {
                cancelPreview();
                toRendering(_browser.selected());
                _browser.clearSignals();
            } else if (_browser.wantsFreeRoam()) {
                cancelPreview();
                toFreeRoam(_browser.selected());
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
            _panel.update(_pixelBuffer);
            if (_panel.wantsBack()) {
                _panel.clearSignals();
                toBrowser();
            }
            break;

        case UIState::FreeRoam:
            if (!_freeRoamController) break;

            _freeRoamController->update(deltaTime);

            // Trigger new render only if camera moved AND no render active
            if (_freeRoamController->hasMoved()) {
                _freeRoamController->clearDirtyFlag();
                if (!_freeRoamRenderActive.load()) {
                    spawnFreeRoamRenderThread();
                }
            }

            if (_freeRoamController->wantsExit()) {
                _freeRoamCancelFlag.store(true);
                if (_freeRoamThread.joinable())
                    _freeRoamThread.join();
                _freeRoamController->disableMouseCapture(_window);
                _freeRoamController.reset();
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
        case UIState::Browser:   _browser.draw(_window, &_previewPixelBuffer); break;
        case UIState::Rendering: _panel.drawRendering(_window); break;
        case UIState::Done:      _panel.drawDone(_window);      break;
        case UIState::FreeRoam:
            // Draw live FreeRoam preview
            if (_freeRoamPixelBuffer.width > 0 && _freeRoamPixelBuffer.height > 0) {
                sf::Texture tex;
                sf::Image img;
                img.create(_freeRoamPixelBuffer.width, _freeRoamPixelBuffer.height);

                {
                    std::lock_guard<std::mutex> swapLock(_freeRoamSwapMutex);
                    std::lock_guard<std::mutex> bufLock(_freeRoamPixelBuffer.mutex);
                    for (int y = 0; y < _freeRoamPixelBuffer.height; ++y) {
                        for (int x = 0; x < _freeRoamPixelBuffer.width; ++x) {
                            int idx = (y * _freeRoamPixelBuffer.width + x) * 4;
                            img.setPixel(x, y, sf::Color(
                                _freeRoamPixelBuffer.rgba[idx + 0],
                                _freeRoamPixelBuffer.rgba[idx + 1],
                                _freeRoamPixelBuffer.rgba[idx + 2],
                                _freeRoamPixelBuffer.rgba[idx + 3]
                            ));
                        }
                    }
                }

                tex.loadFromImage(img);
                sf::Sprite sprite(tex);
                // Fit to window
                float ww = static_cast<float>(_window.getSize().x);
                float wh = static_cast<float>(_window.getSize().y);
                float imgW = static_cast<float>(tex.getSize().x);
                float imgH = static_cast<float>(tex.getSize().y);
                float scale = std::min(ww / imgW, wh / imgH);
                sprite.setScale(scale, scale);
                sprite.setPosition((ww - imgW * scale) / 2.f, (wh - imgH * scale) / 2.f);
                _window.draw(sprite);
            }

            // Draw FPS counter
            {
                static auto lastFpsUpdate = std::chrono::steady_clock::now();
                static int frameCount = 0;
                static float currentFps = 0.0f;

                frameCount++;
                auto now = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration<float>(now - lastFpsUpdate).count();
                if (elapsed >= 0.5f) {
                    currentFps = frameCount / elapsed;
                    frameCount = 0;
                    lastFpsUpdate = now;
                }

                sf::Text fpsText;
                fpsText.setFont(_font);
                fpsText.setCharacterSize(20);
                fpsText.setFillColor(sf::Color::White);
                fpsText.setOutlineColor(sf::Color::Black);
                fpsText.setOutlineThickness(2.f);
                fpsText.setString("FPS: " + std::to_string(static_cast<int>(currentFps + 0.5f)));
                fpsText.setPosition(10.f, 10.f);
                _window.draw(fpsText);
            }
            break;
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
    cancelPreview();
    _previewPixelBuffer.init(0, 0);
    _state = UIState::Browser;
    _browser.refresh();
}

void UIApp::toDone() {
    _state = UIState::Done;
}

void UIApp::toFreeRoam(const std::string& scenePath) {
    _scenePath = scenePath;
    _freeRoamCancelFlag.store(false);

    // Use preview camera (already loaded, has scene position/orientation)
    _freeRoamCamera = _previewCamera;

    _freeRoamController = std::make_unique<FreeRoamController>(_freeRoamCamera);
    _freeRoamController->enableMouseCapture(_window);

    pushToast("ZQSD to move, mouse to look, ESC to exit", ToastType::Info);

    _state = UIState::FreeRoam;
    spawnFreeRoamRenderThread();
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
        core.setDimensionsCallback([this](int w, int h) {
            _pixelBuffer.init(w, h);
            _pixelBuffer.totalRows.store(h);
        });
        core.setRowCallback([this](int y, const uint8_t* rgba, int w) {
            _pixelBuffer.setRow(y, rgba, w * 4);
            _pixelBuffer.rowsComplete.fetch_add(1, std::memory_order_relaxed);
        });
        core.simulate();
        _doneFlag.store(true);
    });
}

void UIApp::joinRenderThread() {
    if (_renderThread.joinable())
        _renderThread.join();
}

void UIApp::spawnPreviewThread() {
    std::string path = _browser.selected();
    _previewThread = std::thread([this, path]() {
        Core core(path);
        core.setCancelFlag(&_previewCancelFlag);
        core.setPreviewMode();
        core.setDimensionsCallback([this](int w, int h) {
            _previewPixelBuffer.init(w, h);
            _previewPixelBuffer.totalRows.store(h);
        });
        core.setRowCallback([this](int y, const uint8_t* rgba, int w) {
            _previewPixelBuffer.setRow(y, rgba, w * 4);
            _previewPixelBuffer.rowsComplete.fetch_add(1, std::memory_order_relaxed);
        });
        bool success = core.loadScene();
        if (success) {
            _previewCamera = core.getCamera();
            core.simulate();
        }
    });
}

void UIApp::cancelPreview() {
    _previewCancelFlag.store(true);
    if (_previewThread.joinable())
        _previewThread.join();
    _previewPixelBuffer.init(0, 0);
    _previewCancelFlag.store(false);
}

void UIApp::spawnFreeRoamRenderThread() {
    // Cancel and join previous render
    _freeRoamCancelFlag.store(true);
    if (_freeRoamThread.joinable())
        _freeRoamThread.join();
    _freeRoamCancelFlag.store(false);

    _freeRoamRenderActive.store(true);

    std::string path = _scenePath;
    Camera cam = _freeRoamController->getCamera();

    _freeRoamThread = std::thread([this, path, cam]() {
        try {
            Core core(path);
            core.setCancelFlag(&_freeRoamCancelFlag);
            core.setPreviewMode(0.75f, 10);
            core.setCameraOverride(cam);
            core.setDimensionsCallback([this](int w, int h) {
                _freeRoamBackBuffer.init(w, h);
                _freeRoamBackBuffer.totalRows.store(h);
            });
            core.setRowCallback([this](int y, const uint8_t* rgba, int w) {
                _freeRoamBackBuffer.setRow(y, rgba, w * 4);
                _freeRoamBackBuffer.rowsComplete.fetch_add(1, std::memory_order_relaxed);
            });
            core.simulate();

            // Swap buffers when render complete
            {
                std::lock_guard<std::mutex> lock(_freeRoamSwapMutex);
                _freeRoamPixelBuffer.swapBuffers(_freeRoamBackBuffer);
            }
        } catch (...) {
            // Ensure flag cleared even on error
        }
        _freeRoamRenderActive.store(false);
    });
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
