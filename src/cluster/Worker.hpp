#pragma once

#include <atomic>
#include <string>

class Worker {
public:
    Worker(const std::string& host, int port);

    void run();

private:
    std::string _host;
    int _port;
    std::atomic<int> _progress{0};
    std::atomic<int> _totalRowsRendered{0};
};
