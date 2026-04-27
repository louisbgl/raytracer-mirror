#include "ObjParser.hpp"
#include "../plugins/Shapes/Triangle.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<std::shared_ptr<IShape>> ObjParser::parse(const std::string& filename, std::shared_ptr<IMaterial> material) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            parseVertex(line);
        } else if (line.substr(0, 2) == "f ") {
            parseFace(line, material);
        }
    }

    return shapes;
}

void ObjParser::parseVertex(const std::string& line) {
    std::istringstream iss(line.substr(2));
    double x, y, z;
    iss >> x >> y >> z;
    vertices.emplace_back(x, y, z);
}

void ObjParser::parseFace(const std::string& line, std::shared_ptr<IMaterial> material) {
    std::istringstream iss(line.substr(2));
    std::vector<int> vertexIndices;
    std::string vertexStr;

    while (iss >> vertexStr) {
        size_t slashPos = vertexStr.find('/');
        int vertexIndex = std::stoi(vertexStr.substr(0, slashPos)) - 1; // OBJ is 1-indexed
        vertexIndices.push_back(vertexIndex);
    }

    for (size_t i = 1; i + 1 < vertexIndices.size(); i++) {
        shapes.push_back(std::make_shared<Triangle>(
            vertices[vertexIndices[0]],
            vertices[vertexIndices[i]],
            vertices[vertexIndices[i + 1]],
            material
        ));
    }
}