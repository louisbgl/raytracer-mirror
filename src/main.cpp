#include "core/Core.hpp"
#include "utils/HelpDisplay.hpp"
#include "core/PluginManager.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        HelpDisplay help;
        help.display();
        return 84;
    }

    if (std::string(argv[1]) == "--help") {
        HelpDisplay help;
        help.display();
        return 0;
    }

    bool logging = (argc == 3 && std::string(argv[2]) == "--log");
    if (argc == 3 && !logging) {
        HelpDisplay help;
        help.display();
        return 84;
    }

    PluginManager::instance().initialize();
    Core core(argv[1], logging);
    if (core.simulate()) return 0;
    else                 return 84;
}