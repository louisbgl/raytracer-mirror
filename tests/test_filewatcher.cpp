#include <criterion/criterion.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "../src/watcher/FileWatcher.hpp"

Test(FileWatcher, construction)
{
    FileWatcher watcher;
    cr_assert_not_null(&watcher);
}

Test(FileWatcher, add_target_path)
{
    std::string test_file = "/tmp/test_filewatcher_file.txt";
    std::ofstream f(test_file);
    f << "test";
    f.close();

    FileWatcher watcher;
    cr_assert_no_throw(watcher.addTargetPath(test_file));

    std::filesystem::remove(test_file);
}

Test(FileWatcher, add_invalid_path)
{
    FileWatcher watcher;
    cr_assert_throw(watcher.addTargetPath("/nonexistent/path/to/file.txt"), std::runtime_error);
}

Test(FileWatcher, process_events_on_file_change)
{
    std::string test_file = "/tmp/test_filewatcher_change.txt";
    std::ofstream f(test_file);
    f << "initial content";
    f.close();

    FileWatcher watcher;
    watcher.addTargetPath(test_file);

    // Give the watcher a moment to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Modify the file
    std::ofstream f2(test_file, std::ios::app);
    f2 << "\nmodified content";
    f2.close();

    // Give the filesystem time to generate the event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check for changes (should detect the modification)
    cr_assert(watcher.watchFileEvents());

    // Cleanup
    std::filesystem::remove(test_file);
}
