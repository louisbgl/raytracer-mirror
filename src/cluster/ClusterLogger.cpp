#include "ClusterLogger.hpp"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

ClusterLogger::ClusterLogger(const std::string& prefix)
    : _prefix(prefix)
{
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream ss;
    ss << "logs/cluster_" << prefix << "_"
       << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";
    _path = ss.str();
}

void ClusterLogger::_openIfNeeded()
{
    if (_file.is_open())
        return;
    std::filesystem::create_directories("logs");
    _file.open(_path);
    if (!_file.is_open()) {
        std::cerr << "ClusterLogger: failed to open " << _path << "\n";
        return;
    }
    _file << "[" << _timestamp() << "] [INFO ] [system] Log started — " << _prefix << std::endl;
}

void ClusterLogger::info(const std::string& source, const std::string& msg)
{
    _write("INFO ", source, msg);
}

void ClusterLogger::warn(const std::string& source, const std::string& msg)
{
    _write("WARN ", source, msg);
}

void ClusterLogger::error(const std::string& source, const std::string& msg)
{
    _write("ERROR", source, msg);
}

void ClusterLogger::_write(const std::string& level, const std::string& source, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _openIfNeeded();
    if (!_file.is_open())
        return;
    _file << "[" << _timestamp() << "] [" << level << "] [" << source << "] " << msg << std::endl;
}

std::string ClusterLogger::_timestamp()
{
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}
