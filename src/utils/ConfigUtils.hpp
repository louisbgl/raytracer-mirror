#pragma once

#include <libconfig.h++>

namespace ConfigUtils {
    inline double getNumber(const libconfig::Setting& setting) {
        if (setting.getType() == libconfig::Setting::TypeInt) {
            return static_cast<double>(static_cast<int>(setting));
        }
        return static_cast<double>(setting);
    }
}
