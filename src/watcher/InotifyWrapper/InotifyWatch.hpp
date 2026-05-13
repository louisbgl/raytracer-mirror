
#pragma once

#include <sys/inotify.h>
#include <cstdint>
#include <string_view>
#include <string>

#include "./InotifyExeception.hpp"
#include "./InotifyInstance.hpp"


class InotifyWatch
{
    public:
        InotifyWatch(const InotifyInstance& notify_instance, std::string_view path, uint32_t mask)
            : _iFd(notify_instance.fd())
            , _wd(::inotify_add_watch(_iFd, path.data(), mask))
            , _scene_path(path)
        {
            if (_wd == -1) { throw InotifyException("inotify_add_watch failed with scene path: " + std::string(path));}
        }

        ~InotifyWatch() { if (_wd != -1) { ::inotify_rm_watch(_iFd, _wd); } }

        InotifyWatch(const InotifyWatch&) = delete;
        InotifyWatch& operator=(const InotifyWatch) = delete;
        InotifyWatch(InotifyWatch&& other) noexcept
            : _iFd(other._iFd)
            , _wd(std::exchange(other._wd, -1))
            , _scene_path(std::move(other._scene_path)) {}
       
        [[nodiscard]] int wd() const { return _wd; }
        [[nodiscard]] std::string_view scene_path() const { return _scene_path; }

    private:
        int _iFd;
        int _wd;
        std::string _scene_path;
};
