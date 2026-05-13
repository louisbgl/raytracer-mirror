#pragma once

#include "../network/TcpServer.hpp"
#include "../network/TcpSocket.hpp"
#include "../core/Image.hpp"
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>

static constexpr int COORDINATOR_PORT = 6767;
static constexpr int CHUNK_SIZE       = 10;

struct WorkerInfo {
    std::unique_ptr<TcpSocket> socket;
    std::string ip;
    int firstRow;
    int lastRow;
    int missedHeartbeats;
    int pixelsReceived;
    int totalRowsRendered;
    std::chrono::steady_clock::time_point startTime;
    bool done;
};

class Coordinator {
public:
    explicit Coordinator(const std::string& sceneFile);

    void run();

private:
    std::string _sceneFile;
    std::string _sceneContent;
    std::vector<WorkerInfo> _workers;
    std::queue<std::pair<int, int>> _pendingChunks;
    mutable std::mutex _statsMutex;
    int _imageWidth  = 0;
    int _imageHeight = 0;

    void _waitForWorkers();
    void _distributeWork();
    void _monitorAndCollect(Image& image);
    void _assignNextChunk(int workerIdx);
    void _printLeaderboard() const;
};
