#pragma once

#include "../network/Message.hpp"
#include "../network/TcpServer.hpp"
#include "../network/TcpSocket.hpp"
#include "../core/Image.hpp"
#include "ClusterLogger.hpp"
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <chrono>

static constexpr int COORDINATOR_PORT = 6767;
static constexpr int CHUNK_SIZE       = 10;

struct WorkerInfo {
    std::unique_ptr<TcpSocket> socket;
    std::string ip;
    int firstRow           = 0;
    int lastRow            = 0;
    int missedTimeouts     = 0;
    int pixelsReceived     = 0;
    int totalRowsRendered  = 0;
    std::chrono::steady_clock::time_point startTime{};
    bool done              = false;
};

class Coordinator {
public:
    explicit Coordinator(const std::string& sceneFile);

    void run();

private:
    std::string _sceneFile;
    std::string _sceneContent;
    std::string _outputFile;
    std::vector<WorkerInfo> _workers;
    std::queue<std::pair<int, int>> _pendingChunks;
    mutable std::mutex _statsMutex;
    std::mutex _chunksMutex;
    ClusterLogger _log{"coordinator"};
    std::atomic<int> _chunksCompleted{0};
    std::atomic<int> _coordinatorRowsRendered{0};
    int _totalChunks  = 0;
    int _imageWidth   = 0;
    int _imageHeight  = 0;
    std::chrono::steady_clock::time_point _renderStart;

    void _waitForWorkers();
    void _distributeWork();
    void _monitorAndCollect(Image& image);
    void _assignNextChunk(int workerIdx);
    void _handleTimeout(std::vector<struct pollfd>& pfds, int& doneCount);
    void _handlePixels(int workerIdx, const Message& msg, Image& image,
                       std::vector<struct pollfd>& pfds, int& doneCount);
    void _renderLocalChunks(Image& image);
    void _drawDashboard() const;
};
