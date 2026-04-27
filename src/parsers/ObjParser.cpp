#include "ObjParser.hpp"
#include "../plugins/Shapes/MeshTriangle.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<std::shared_ptr<IBoundable>> ObjParser::parse(const std::string& filename, std::shared_ptr<IMaterial> material) {
    _vertices.clear();
    _texcoords.clear();
    _normals.clear();
    _shapes.clear();

    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open OBJ file: " + filename);

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 3) == "vn ")      parseNormal(line);
        else if (line.substr(0, 3) == "vt ") parseTexcoord(line);
        else if (line.substr(0, 2) == "v ")  parseVertex(line);
        else if (line.substr(0, 2) == "f ")  parseFace(line, material);
    }

    return _shapes;
}

void ObjParser::parseVertex(const std::string& line) {
    std::istringstream iss(line.substr(2));
    double x, y, z;
    iss >> x >> y >> z;
    _vertices.emplace_back(x, y, z);
}

void ObjParser::parseTexcoord(const std::string& line) {
    std::istringstream iss(line.substr(3));
    double u, v;
    iss >> u >> v;
    _texcoords.push_back({u, v});
}

void ObjParser::parseNormal(const std::string& line) {
    std::istringstream iss(line.substr(3));
    double x, y, z;
    iss >> x >> y >> z;
    _normals.emplace_back(x, y, z);
}

ObjParser::FaceVertex ObjParser::parseFaceVertex(const std::string& token) {
    // Handles: "1", "1/2", "1//3", "1/2/3"
    FaceVertex fv;
    size_t slash1 = token.find('/');
    fv.vIdx = std::stoi(token.substr(0, slash1)) - 1;

    if (slash1 == std::string::npos)
        return fv;

    size_t slash2 = token.find('/', slash1 + 1);
    bool hasTexcoord = (slash2 != slash1 + 1); // false if "//" (no vt between slashes)
    bool hasNormal   = (slash2 != std::string::npos && slash2 + 1 < token.size());

    if (hasTexcoord)
        fv.vtIdx = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
    if (hasNormal)
        fv.vnIdx = std::stoi(token.substr(slash2 + 1)) - 1;

    return fv;
}

void ObjParser::parseFace(const std::string& line, std::shared_ptr<IMaterial> material) {
    std::istringstream iss(line.substr(2));
    std::vector<FaceVertex> fvs;
    std::string token;

    while (iss >> token)
        fvs.push_back(parseFaceVertex(token));

    // Fan triangulation: (0,1,2), (0,2,3), (0,3,4) ...
    for (size_t i = 1; i + 1 < fvs.size(); i++) {
        const FaceVertex& a = fvs[0];
        const FaceVertex& b = fvs[i];
        const FaceVertex& c = fvs[i + 1];

        // Collect per-vertex normals if all three vertices have one
        std::optional<std::array<Vec3, 3>> normals = std::nullopt;
        if (a.vnIdx >= 0 && b.vnIdx >= 0 && c.vnIdx >= 0) {
            normals = {{ _normals[a.vnIdx], _normals[b.vnIdx], _normals[c.vnIdx] }};
        }

        // Collect per-vertex UVs if all three vertices have one
        std::optional<std::array<UV, 3>> uvs = std::nullopt;
        if (a.vtIdx >= 0 && b.vtIdx >= 0 && c.vtIdx >= 0) {
            UV uvA = { _texcoords[a.vtIdx][0], _texcoords[a.vtIdx][1] };
            UV uvB = { _texcoords[b.vtIdx][0], _texcoords[b.vtIdx][1] };
            UV uvC = { _texcoords[c.vtIdx][0], _texcoords[c.vtIdx][1] };
            uvs = {{ uvA, uvB, uvC }};
        }

        _shapes.push_back(std::make_shared<MeshTriangle>(
            _vertices[a.vIdx], _vertices[b.vIdx], _vertices[c.vIdx],
            material, normals, uvs
        ));
    }
}
