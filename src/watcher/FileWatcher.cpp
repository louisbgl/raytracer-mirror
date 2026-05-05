
#include "./FileWatcher.hpp"

#include <linux/limits.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <climits>



FileWatcher::FileWatcher()
{
    _iFd = inotify_init1(IN_NONBLOCK);
    if (_iFd == -1) { throw std::runtime_error("inotify_init1 failed !"); }
    _wd = 0;
}

FileWatcher::~FileWatcher()
{
    if (_iFd != -1) { close(_iFd); }
}

void FileWatcher::addTargetPath(const std::string& scene_path)
{
    _wd = inotify_add_watch(_iFd, scene_path.c_str(), IN_CLOSE_WRITE | IN_MODIFY);
    if (_wd == -1) { throw std::runtime_error("inotify_add_watch failed for file: " + scene_path); }
    _watchedFiles[_wd] = scene_path;
    _watchedPath = scene_path;
}

bool FileWatcher::watchFileEvents()
{
    const ssize_t buf_size = (sizeof(const struct inotify_event) + NAME_MAX + 1);
    char BUFFER[buf_size];
    memset(BUFFER, 0, buf_size);
    ssize_t read_size = 0;
    const struct inotify_event *ievent;

    read_size = read(_iFd, BUFFER, buf_size);
    if (read_size == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return _fileChanged;
        }

        throw std::runtime_error("Read failed on fd: " + std::to_string(_iFd));
    }

    if (read_size <= 0) { return _fileChanged; }
    
    for (char *read_ptr = BUFFER; read_ptr < BUFFER + read_size;
            read_ptr += sizeof(struct inotify_event) + ievent->len) {
        ievent = (const struct inotify_event *) read_ptr;

        if (ievent->mask & IN_MODIFY || (ievent->mask & IN_CLOSE_WRITE)) { _fileChanged = true; }
    }
    
    return _fileChanged;
}
