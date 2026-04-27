#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger::Logger() {
    std::filesystem::create_directories("logs");
    _path = "logs/raytracer_" + _timestamp() + ".log";
    _out.open(_path);
    if (!_out.is_open())
        std::cerr << "Logger: could not open " << _path << std::endl;
}

Logger::~Logger() {
    if (_warnings.empty()) {
        _write("warnings", "none");
    } else {
        for (const auto& w : _warnings)
            _write("warnings", w);
    }
}

void Logger::logScene(const std::string& scenePath, const Scene& scene) {
    const Camera& cam = scene.camera();
    long long pixels = static_cast<long long>(cam.getWidth()) * static_cast<long long>(cam.getHeight());

    _write("scene",  "file: " + scenePath);
    _write("scene",  "resolution: "
        + std::to_string(static_cast<int>(cam.getWidth()))
        + " x "
        + std::to_string(static_cast<int>(cam.getHeight()))
        + "  (" + _fmtNum(pixels) + " pixels)");
    _write("scene",  "shapes: "    + std::to_string(scene.world().objects().size())
        + "   lights: "   + std::to_string(scene.lights().size())
        + "   materials: " + std::to_string(scene.materialCount()));

    auto fmt = [](double v) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v;
        return ss.str();
    };
    Vec3 pos = cam.getPosition();
    Vec3 at  = cam.getLookAt();
    _write("camera", "position: (" + fmt(pos.x()) + ", " + fmt(pos.y()) + ", " + fmt(pos.z()) + ")");
    _write("camera", "look_at:  (" + fmt(at.x())  + ", " + fmt(at.y())  + ", " + fmt(at.z())  + ")"
        + "   fov: " + fmt(cam.getFov()) + "\xc2\xb0");

    const auto& rc = scene.rendererConfig();

    _write("renderer", "output: " + rc.outputFile);

    if (rc.aaEnabled && rc.aaSamples > 1) {
        _write("renderer", "antialiasing: enabled (" + rc.aaMethod + ", "
            + std::to_string(rc.aaSamples) + " samples)");
    } else {
        _write("renderer", "antialiasing: disabled");
    }

    std::ostringstream ambient;
    ambient << "ambient: " << rc.ambientColor << "  multiplier: " << fmt(rc.ambientMultiplier);
    _write("renderer", ambient.str());

    _write("renderer", "diffuse multiplier: " + fmt(rc.diffuseMultiplier));

    if (!rc.backgroundImage.empty()) {
        _write("renderer", "background: image (" + rc.backgroundImage + ")");
    } else {
        std::ostringstream bg;
        bg << "background: " << rc.backgroundColor;
        _write("renderer", bg.str());
    }
}

void Logger::logTiming(double parseS, double renderS, double writeS, long long pixelCount) {
    double totalS = parseS + renderS + writeS;

    _write("timing", "parse: "  + _fmtTime(parseS)
        + "   render: " + _fmtTime(renderS)
        + "   write: "  + _fmtTime(writeS)
        + "   total: "  + _fmtTime(totalS));

    if (pixelCount > 0) {
        double perPixel   = renderS / static_cast<double>(pixelCount);
        long long pixPerS = static_cast<long long>(pixelCount / renderS);
        _write("perf", "time/pixel: " + _fmtTime(perPixel)
            + "   pixels/sec: " + _fmtNum(pixPerS));
    }
}

void Logger::warn(const std::string& message) {
    _warnings.push_back(message);
}

void Logger::_write(const std::string& tag, const std::string& line) {
    _out << std::left << std::setw(12) << ("[" + tag + "]") << line << "\n";
}

std::string Logger::_timestamp() {
    auto now  = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

std::string Logger::_fmtTime(double s) {
    std::ostringstream ss;
    if (s < 0.001)
        ss << std::fixed << std::setprecision(2) << (s * 1e6) << "\xc2\xb5s";
    else if (s < 1.0)
        ss << std::fixed << std::setprecision(2) << (s * 1000.0) << "ms";
    else if (s < 60.0)
        ss << std::fixed << std::setprecision(3) << s << "s";
    else
        ss << static_cast<int>(s) / 60 << "m"
           << std::fixed << std::setprecision(1) << std::fmod(s, 60.0) << "s";
    return ss.str();
}

std::string Logger::_fmtNum(long long n) {
    std::string s = std::to_string(n);
    int pos = static_cast<int>(s.size()) - 3;
    while (pos > 0) {
        s.insert(static_cast<size_t>(pos), ",");
        pos -= 3;
    }
    return s;
}
