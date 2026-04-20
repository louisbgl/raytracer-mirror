#include "core/Core.hpp"
#include <iostream>

void help() {
    std::cout << "Usage: raytracer <input_file>\n" << std::endl;
    std::cout << "Currently supported:" << std::endl;
    std::cout << "Shapes:" << std::endl;
    std::cout << "\tSphere (position (x, y, z), radius, material)" << std::endl;
    std::cout << "Materials:" << std::endl;
    std::cout << "\tLambertian (name, color (r, g, b))" << std::endl;
    std::cout << "Lights:" << std::endl;
    std::cout << "\tPointLight (position (x, y, z), color (r, g, b), intensity)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        help();
        return 84;
    }

    if (std::string(argv[1]) == "--help") {
        help();
        return 0;
    }

    Core core(argv[1]);
    if (core.simulate()) return 0;
    else                 return 84;
}