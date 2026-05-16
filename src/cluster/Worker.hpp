#pragma once

#include "ClusterLogger.hpp"
#include <atomic>
#include <string>

class Worker {
public:
    Worker(const std::string& host, int port);

    void run();

    static int launch(const std::string& addr);

private:
    std::string _host;
    int _port;
    std::atomic<int> _totalRowsRendered{0};
    ClusterLogger _log{"worker"};
};

