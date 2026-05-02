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
        out << "\toutput = <string> (default: \"output.ppm\") — supports .ppm, .png, .jpg\n";
        out << "\tantialiasing:\n";
        out << "\t\tenabled = <bool> (default: false)\n";
        out << "\t\tmethod = <string> (\"ssaa\" | \"adaptive\") (default: \"ssaa\")\n";
        out << "\t\tsamples = <int> — ssaa only (default: 1)\n";
        out << "\t\tthreshold = <float> — adaptive only (default: 0.1)\n";
        out << "\tlighting:\n";
        out << "\t\tambientColor = { r = <int>; g = <int>; b = <int>; } (default: 25, 25, 38)\n";
        out << "\t\tambientMultiplier = <float> (default: 0.4)\n";
        out << "\t\tdiffuseMultiplier = <float> (default: 0.6)\n";
        out << "\t\tambientOcclusion:\n";
        out << "\t\t\tenabled = <bool> (default: false)\n";
        out << "\t\t\tsamples = <int> (default: 16)\n";
        out << "\t\t\tradius = <float> (default: 5.0)\n";
        out << "\tbackground:\n";
        out << "\t\tcolor = { r = <int>; g = <int>; b = <int>; } (default: 135, 206, 235)\n";
        out << "\t\timage = <string> (optional — supports .ppm, .png, .jpg)\n";
        out << "\ttoneMapping:\n";
        out << "\t\tenabled = <bool> (default: true) — ACES filmic, luminance-preserving\n";
        out << "\t\tstrength = <float> [0.0-1.0] (default: 0.8) — blend between original and mapped\n\n";

        out << "Scene Composition:\n";
        out << "\tscenes = ( { path = <string>; position = {x,y,z}; [rotation = {x,y,z}]; [scale = {x,y,z}]; } );\n";
        out << "\tSubscenes merge materials, shapes, lights into root. renderer/camera ignored.\n";
        out << "\tTransforms compose down the hierarchy. Cycle detection included.\n\n";

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
