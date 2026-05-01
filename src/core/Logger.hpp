#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "../DataTypes/Scene.hpp"
#include "../DataTypes/RenderStats.hpp"

class Logger {
public:
    Logger();
    ~Logger();

    const std::string& path() const { return _path; }

    void logScene(const std::string& scenePath, const Scene& scene);
    void logTiming(double parseS, double renderS, double writeS, long long pixelCount);
    void logStats(const RenderStats& stats, const RendererConfig& rc);
    void warn(const std::string& message);

private:
    std::string   _path;
    std::ofstream _out;
    std::vector<std::string> _warnings;

    void _write(const std::string& tag, const std::string& line);
    void _logCamera(const Camera& cam);
    void _logRenderer(const RendererConfig& rc);
    void _logAA(const RendererConfig& rc);
    void _logAO(const RendererConfig& rc);
    void _logMultithreading(const RendererConfig& rc);
    static std::string _timestamp();
    static std::string _fmtTime(double s);
    static std::string _fmtNum(long long n);
    static std::string _fmtDouble(double v);
};
