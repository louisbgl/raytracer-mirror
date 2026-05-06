#pragma once

#include "../network/TcpServer.hpp"
#include "../network/TcpSocket.hpp"
#include "../core/Image.hpp"
#include <memory>
#include <string>
#include <vector>

static constexpr int COORDINATOR_PORT = 6767;

struct WorkerInfo {
    std::unique_ptr<TcpSocket> socket;
    std::string ip;
    int firstRow;
    int lastRow;
    int missedHeartbeats;
    int rowsReceived;
    bool done;
};

class Coordinator {
public:
    explicit Coordinator(const std::string& sceneFile);

    void run();

private:
    std::string _sceneFile;
    std::vector<WorkerInfo> _workers;
    int _imageWidth  = 0;
    int _imageHeight = 0;

    void _waitForWorkers();
    void _distributeWork();
    void _monitorAndCollect(Image& image);
    void _renderLocalChunk(Image& image, int firstRow, int lastRow);
    void _reassignChunk(Image& image, int firstRow, int lastRow);
};
