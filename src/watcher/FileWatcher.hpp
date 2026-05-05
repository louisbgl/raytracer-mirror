
#pragma once

#include <map>
#include <string>
#include <atomic>




class FileWatcher 
{
    public:
        FileWatcher();
        ~FileWatcher();
        
        void addTargetPath(const std::string& scene_path);
        
        /// Check for file change events. Returns true if file was modified.
        /// Non-blocking inotify read with IN_NONBLOCK mode
        bool watchFileEvents();
        
        /// Clear the change flag (call after handling the change)
        void clearChangeFlag() { _fileChanged = false; }
        
        /// Get the watched file path
        std::string getWatchedPath() const { return _watchedPath; }

    private:
        int _iFd;
        int _wd;
        std::map<int, std::string> _watchedFiles;
        std::string _watchedPath;
        std::atomic<bool> _fileChanged{false};
};
