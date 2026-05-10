
#pragma once

#include <sys/inotify.h>
#include <unistd.h>
#include <utility>

#include "./InotifyExeception.hpp"


class InotifyInstance
{
    public:
        InotifyInstance() : _fd(::inotify_init1(IN_NONBLOCK)) {
            if (_fd == -1) { throw InotifyException("inotifyInstace init failed"); }
        }

        ~InotifyInstance() {
            if (_fd != -1) { ::close(_fd); }
        }

        InotifyInstance(const InotifyInstance&) = delete;
        InotifyInstance& operator=(const InotifyInstance) = delete;

        //Lets you copy the _fd value then replaces old _fd to -1 so it skips the close
        InotifyInstance(InotifyInstance&& other) noexcept : _fd(std::exchange(other._fd, -1)) {} 
        [[nodiscard]]int fd() const { return _fd; }

    private:
        int _fd;
};
