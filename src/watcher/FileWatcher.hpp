
#pragma once

#include <sys/inotify.h>
#include <sys/types.h>
#include "./InotifyWrapper/InotifyInstance.hpp"
#include "./InotifyWrapper/InotifyWatch.hpp"
#include "./InotifyWrapper/InotifyEventReader.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>



class FileWatcher 
{
    public:
        FileWatcher();
        ~FileWatcher() = default;
        
        void addTargetPath(std::string_view scene_path, uint32_t mask = IN_CLOSE_WRITE | IN_MODIFY);
        void clearChangeFlag() { _fileChanged = false; }
        bool watchFileEvents();
        std::string getWatchedPath() const { return _watchedPath; }

    private:
        std::string _watchedPath;
        InotifyInstance _inotify_instance;
        InotifyEventReader _inotify_reader;
        std::vector<InotifyWatch> _inotify_watcher;
        bool _fileChanged = false;
};
