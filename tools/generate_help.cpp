#include "../src/core/PluginManager.hpp"
#include <iostream>
#include <fstream>

int main() {
    try {
        // Initialize plugin manager
        PluginManager::instance().initialize();

        // Open output file
        std::ofstream out("help.txt");
        if (!out.is_open()) {
            std::cerr << "Failed to open help.txt for writing" << std::endl;
            return 1;
        }

        out << "Usage: raytracer <input_file>\n\n";
        out << "Currently supported:\n\n";

        // Get shapes
        auto shapes = PluginManager::instance().getPluginsByCategory("shape");
        if (!shapes.empty()) {
            out << "Shapes:\n";
            for (const auto& meta : shapes) {
                out << "\t" << meta.helpText << "\n";
            }
            out << "\n";
        }

        // Get materials
        auto materials = PluginManager::instance().getPluginsByCategory("material");
        if (!materials.empty()) {
            out << "Materials:\n";
            for (const auto& meta : materials) {
                out << "\t" << meta.helpText << "\n";
            }
            out << "\n";
        }

        // Get lights
        auto lights = PluginManager::instance().getPluginsByCategory("light");
        if (!lights.empty()) {
            out << "Lights:\n";
            for (const auto& meta : lights) {
                out << "\t" << meta.helpText << "\n";
            }
            out << "\n";
        }

        out.close();

        std::cout << "✓ Generated help.txt with "
                  << (shapes.size() + materials.size() + lights.size())
                  << " plugins" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error generating help: " << e.what() << std::endl;
        return 1;
    }
}
