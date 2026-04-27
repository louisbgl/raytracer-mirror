#pragma once

#include <libconfig.h++>
#include "../DataTypes/Vec3.hpp"

class ConfigUtils {
public:
    static double getNumber(const libconfig::Setting& setting) {
        if (setting.getType() == libconfig::Setting::TypeInt) {
            return static_cast<double>(static_cast<int>(setting));
        }
        return static_cast<double>(setting);
    }

    static Vec3 parseVec3(const libconfig::Setting& setting, const char* xKey = "x", const char* yKey = "y", const char* zKey = "z") {
        if (setting.exists(xKey) && setting.exists(yKey) && setting.exists(zKey)) {
            return Vec3(
                getNumber(setting[xKey]),
                getNumber(setting[yKey]),
                getNumber(setting[zKey])
            );
        }
        return Vec3(0, 0, 0);
    }

    static Vec3 parsePosition(const libconfig::Setting& config) {
        if (config.exists("position")) {
            return parseVec3(config["position"]);
        }
        return Vec3(0, 0, 0);
    }

    static Vec3 parseColor(const libconfig::Setting& config, const char* colorKey = "color") {
        if (config.exists(colorKey)) {
            return parseVec3(config[colorKey], "r", "g", "b");
        }
        return Vec3(0, 0, 0);
    }
};
