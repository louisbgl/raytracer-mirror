#include "core/Core.hpp"
#include "utils/HelpDisplay.hpp"

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

    bool log = (argc == 3 && std::string(argv[2]) == "--log");
    if (argc == 3 && !log) {
        HelpDisplay help;
        help.display();
        return 84;
    }

    Core core(argv[1], log);
    if (core.simulate()) return 0;
    else                 return 84;
}