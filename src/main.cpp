#include "core/Core.hpp"
#include "core/PluginManager.hpp"
#include "utils/HelpDisplay.hpp"
#include "utils/clap/App.hpp"
#include "utils/clap/ClapExceptions.hpp"
#include "cluster/Coordinator.hpp"
#include "cluster/Worker.hpp"
#include <string>

#ifdef WITH_UI
#include "ui/UIApp.hpp"
#endif

enum class RunMode { ShowUsage, LaunchUI, RunCLI, RunCoordinator, RunWorker, Error };

struct Args {
    bool        usage       = false;
    bool        log         = false;
    bool        ui          = false;
    bool        coordinator = false;
    std::string scene       = {};
    bool        sceneSet    = false;
    std::string worker      = {};
    bool        workerSet   = false;
};

static int parseArgs(int argc, char* argv[], Args& out) {
    clap::App app("raytracer", "CPU raytracer — render scenes from .txt scene files");

    #ifdef WITH_UI
    auto& ui          = app.flag("--ui",          "Launch windowed UI mode");
    #endif
    auto& usage       = app.flag("--usage",       "Show detailed scene file format reference");
    auto& log         = app.flag("--log",         "Enable logging");
    auto& coordinator = app.flag("--coordinator", "Launch as cluster coordinator");
    auto& worker      = app.option<std::string>("--worker", "Launch as cluster worker (ip:port)");
    auto& scene       = app.positional<std::string>("scene", "Scene config file (.txt)");

    try {
        if (argc == 1) {
            char* help_argv[] = {argv[0], const_cast<char*>("--help")};
            app.parse(2, help_argv);
        }
        app.parse(argc, argv);
    } catch (const clap::HelpRequested&) {
        return 0;
    } catch (const clap::ClapException& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    out.usage       = static_cast<bool>(usage);
    out.log         = static_cast<bool>(log);
    out.coordinator = static_cast<bool>(coordinator);
    out.sceneSet    = scene.is_set();
    out.workerSet   = worker.is_set();
    if (out.sceneSet)  out.scene  = scene.get();
    if (out.workerSet) out.worker = worker.get();
    #ifdef WITH_UI
    out.ui = static_cast<bool>(ui);
    #endif
    return -1;
}

static RunMode determineMode(const Args& args) {
    if (args.usage)                          return RunMode::ShowUsage;
    if (args.workerSet)                      return RunMode::RunWorker;
    if (args.coordinator && args.sceneSet)   return RunMode::RunCoordinator;
    #ifdef WITH_UI
    if (args.ui)                             return RunMode::LaunchUI;
    #endif
    if (args.sceneSet)                       return RunMode::RunCLI;
    return RunMode::Error;
}

static int runCLI(const Args& args) {
    PluginManager::instance().initialize();
    Core core(args.scene, args.log);
    return core.simulate() ? 0 : 84;
}

static int runCoordinator(const Args& args) {
    PluginManager::instance().initialize();
    try {
        Coordinator coordinator(args.scene);
        coordinator.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Coordinator error: " << e.what() << "\n";
        return 84;
    }
}

static int runWorker(const Args& args) {
    PluginManager::instance().initialize();
    std::string addr = args.worker;
    size_t colon = addr.rfind(':');
    if (colon == std::string::npos) {
        std::cerr << "Error: --worker expects ip:port format\n";
        return 84;
    }
    std::string host = addr.substr(0, colon);
    int port = std::stoi(addr.substr(colon + 1));
    try {
        Worker worker(host, port);
        worker.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Worker error: " << e.what() << "\n";
        return 84;
    }
}

int main(int argc, char* argv[]) {
    Args args;
    if (int err = parseArgs(argc, argv, args); err != -1)
        return err;

    switch (determineMode(args)) {
        case RunMode::ShowUsage:
            HelpDisplay{}.display();
            return 0;

        case RunMode::LaunchUI:
            #ifdef WITH_UI
            return UIApp{}.run();
            #else
            return 84;
            #endif

        case RunMode::RunCLI:
            return runCLI(args);

        case RunMode::RunCoordinator:
            return runCoordinator(args);

        case RunMode::RunWorker:
            return runWorker(args);

        case RunMode::Error:
        default:
            HelpDisplay{}.display();
            return 84;
    }
}
