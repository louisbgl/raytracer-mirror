
#include "./FileWatcher.hpp"

#include <linux/limits.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <climits>



FileWatcher::FileWatcher(): _inotify_instance(), _inotify_reader(_inotify_instance)
{
}

void FileWatcher::addTargetPath(std::string_view scene_path, uint32_t mask)
{
    _watchedPath = std::string(scene_path);
    _inotify_watcher.emplace_back(_inotify_instance, _watchedPath, mask);
}

bool FileWatcher::watchFileEvents()
{
    auto events = _inotify_reader.read();
    if (!events) { return _fileChanged; }

    auto[begin, end] = *events;
    for (auto it = begin; it != end; ++it) {
        const InotifyEvent event = *it;
        
        if (event.isClosedWrite() || event.isModified()) {
            _fileChanged = true;
            break;
        }
    }

    return _fileChanged;
}
