#pragma once

#include <fstream>
#include <mutex>
#include <string>

class ClusterLogger {
public:
    explicit ClusterLogger(const std::string& prefix);

    void info (const std::string& source, const std::string& msg);
    void warn (const std::string& source, const std::string& msg);
    void error(const std::string& source, const std::string& msg);

    const std::string& path() const { return _path; }

private:
    std::ofstream      _file;
    mutable std::mutex _mutex;
    std::string        _path;
    std::string        _prefix;

    void _openIfNeeded();
    void _write(const std::string& level, const std::string& source, const std::string& msg);
    static std::string _timestamp();
};
