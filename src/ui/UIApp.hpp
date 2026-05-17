#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <string>
#include <thread>
#include <memory>
#include <chrono>
#include <mutex>

#include "PixelBuffer.hpp"
#include "RenderPanel.hpp"
#include "SceneBrowser.hpp"
#include "FreeRoamController.hpp"
#include "watcher/FileWatcher.hpp"
#include "ToastManager.hpp"
#include "DataTypes/Camera.hpp"

enum class UIState {
    Browser,
    Rendering,
    Done,
    FreeRoam,
};

class UIApp {
public:
    UIApp();
    ~UIApp();

    int run();

    static constexpr unsigned int WIN_W = 1000;
    static constexpr unsigned int WIN_H = 750;

private:
    void handleEvents();
    void update(float deltaTime);
    void draw();

    void toRendering(const std::string& scenePath);
    void toBrowser();
    void toDone();
    void toFreeRoam(const std::string& scenePath);

    /// Check if scene file has changed and trigger reload if needed
    void checkSceneFileWatch();
    void pushToast(const std::string& message, ToastType type = ToastType::Info);

    void spawnRenderThread();
    void joinRenderThread();
    void spawnPreviewThread();
    void cancelPreview();
    void spawnFreeRoamRenderThread();

    sf::RenderWindow _window;
    sf::Font         _font;
    UIState          _state = UIState::Browser;

    SceneBrowser _browser;
    RenderPanel  _panel;
    ToastManager _toastManager;
    std::unique_ptr<FreeRoamController> _freeRoamController;

    std::string       _scenePath;
    PixelBuffer       _pixelBuffer;
    std::thread       _renderThread;
    std::atomic<bool> _cancelFlag{false};
    std::atomic<bool> _doneFlag{false};
    std::atomic<bool> _renderFailed{false};
    std::atomic<bool> _renderSucceeded{false};
    std::string              _renderErrorMessage;
    std::mutex               _renderErrorMutex;

    PixelBuffer       _previewPixelBuffer;
    std::thread       _previewThread;
    std::atomic<bool> _previewCancelFlag{false};
    Camera            _previewCamera;

    // FreeRoam mode
    PixelBuffer       _freeRoamPixelBuffer;      // Display buffer
    PixelBuffer       _freeRoamBackBuffer;       // Render target
    std::mutex        _freeRoamSwapMutex;
    std::thread       _freeRoamThread;
    std::atomic<bool> _freeRoamCancelFlag{false};
    std::atomic<bool> _freeRoamRenderActive{false};
    Camera            _freeRoamCamera;
    float             _freeRoamRenderFps{0.0f};
    std::chrono::steady_clock::time_point _freeRoamLastRenderTime;
    std::vector<float> _freeRoamRenderTimes;

    // File watching for auto-reload
    std::unique_ptr<FileWatcher> _fileWatcher;
    std::chrono::steady_clock::time_point _lastWatchCheck;
    std::chrono::steady_clock::time_point _lastReloadTime;
    std::chrono::steady_clock::time_point _lastFrameTime;
    bool _showReloadIndicator{false};
    
    /// Throttle file watch checks to once per 200ms for better responsiveness
    static constexpr std::chrono::milliseconds WATCH_THROTTLE{200};
    /// Debounce rapid saves: ignore reload if last one was < 500ms ago
    static constexpr std::chrono::milliseconds RELOAD_DEBOUNCE{500};
};
