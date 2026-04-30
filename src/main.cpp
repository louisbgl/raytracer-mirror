#include "core/Core.hpp"
#include "core/PluginManager.hpp"
#include "utils/HelpDisplay.hpp"
#include "utils/clap/App.hpp"
#include "utils/clap/ClapExceptions.hpp"

#ifdef WITH_UI
#include "ui/UIApp.hpp"
#endif

enum class RunMode { ShowUsage, LaunchUI, RunCLI, Error };

struct Args {
    bool        usage    = false;
    bool        log      = false;
    bool        ui       = false;
    std::string scene    = {};
    bool        sceneSet = false;
};

static int parseArgs(int argc, char* argv[], Args& out) {
    clap::App app("raytracer", "CPU raytracer — render scenes from .cfg files");

    #ifdef WITH_UI
    auto& ui    = app.flag("--ui",    "Launch windowed UI mode");
    #endif
    auto& usage = app.flag("--usage", "Show detailed scene file format reference");
    auto& log   = app.flag("--log",   "Enable logging");
    auto& scene = app.positional<std::string>("scene", "Scene config file (.cfg)");

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

    out.usage    = static_cast<bool>(usage);
    out.log      = static_cast<bool>(log);
    out.sceneSet = scene.is_set();
    if (out.sceneSet) out.scene = scene.get();
    #ifdef WITH_UI
    out.ui = static_cast<bool>(ui);
    #endif
    return -1;
}

static RunMode determineMode(const Args& args) {
    if (args.usage)    return RunMode::ShowUsage;
    #ifdef WITH_UI
    if (args.ui)       return RunMode::LaunchUI;
    #endif
    if (args.sceneSet) return RunMode::RunCLI;
    return RunMode::Error;
}

static int runCLI(const Args& args) {
    PluginManager::instance().initialize();
    Core core(args.scene, args.log);
    return core.simulate() ? 0 : 84;
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

        case RunMode::Error:
        default:
            HelpDisplay{}.display();
            return 84;
    }
}
