#include "HelpDisplay.hpp"
#include <iostream>
#include <fstream>

HelpDisplay::HelpDisplay(const std::string& filepath) : _filepath(filepath) {}

void HelpDisplay::display() const {
    std::ifstream file(_filepath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << "\n";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Usage: raytracer <input_file>" << std::endl;
    }
}
