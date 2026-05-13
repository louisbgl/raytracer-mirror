
#pragma once

#include <system_error>
#include <string>
#include <cerrno>



class InotifyException: public std::system_error
{
    public:
        explicit InotifyException(const std::string& msg) : std::system_error(errno, std::generic_category(), msg) {}
};
