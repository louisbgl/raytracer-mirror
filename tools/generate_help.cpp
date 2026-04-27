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

        out << "Usage: raytracer <input_file> [--log]\n\n";
        out << "Scene File Structure:\n\n";

        // Camera section
        out << "Camera:\n";
        out << "\tresolution = { width = <int>; height = <int>; };\n";
        out << "\tposition = { x = <float>; y = <float>; z = <float>; };\n";
        out << "\tlook_at = { x = <float>; y = <float>; z = <float>; };\n";
        out << "\tup = { x = <float>; y = <float>; z = <float>; };\n";
        out << "\tfieldOfView = <float>;\n\n";

        // Renderer section
        out << "Renderer (optional):\n";
        out << "\toutputFile = <string> (default: \"output.ppm\")\n";
        out << "\tantialiasing:\n";
        out << "\t\tenabled = <bool> (default: false)\n";
        out << "\t\tmethod = <string> (\"ssaa\") (default: \"ssaa\")\n";
        out << "\t\tsamples = <int> (default: 1)\n";
        out << "\tlighting:\n";
        out << "\t\tambientColor = { r = <int>; g = <int>; b = <int>; } (default: 25, 25, 38)\n";
        out << "\t\tambientMultiplier = <float> (default: 0.4)\n";
        out << "\t\tdiffuseMultiplier = <float> (default: 0.6)\n";
        out << "\tbackground:\n";
        out << "\t\tcolor = { r = <int>; g = <int>; b = <int>; } (default: 135, 206, 235)\n";
        out << "\t\timage = <string> (optional PPM file path)\n\n";

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
