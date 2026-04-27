#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "../DataTypes/Vec3.hpp"
#include "../Interfaces/IMaterial.hpp"
#include "../Interfaces/IBoundable.hpp"

class ObjParser {
public:
    ObjParser() = default;
    ~ObjParser() = default;

    std::vector<std::shared_ptr<IBoundable>> parse(const std::string& filename, std::shared_ptr<IMaterial> material);

private:
    struct FaceVertex {
        int vIdx  = -1;
        int vtIdx = -1;
        int vnIdx = -1;
    };

    void parseVertex(const std::string& line);
    void parseTexcoord(const std::string& line);
    void parseNormal(const std::string& line);
    void parseFace(const std::string& line, std::shared_ptr<IMaterial> material);
    FaceVertex parseFaceVertex(const std::string& token);

    std::vector<Vec3> _vertices;
    std::vector<std::array<double, 2>> _texcoords;
    std::vector<Vec3> _normals;
    std::vector<std::shared_ptr<IBoundable>> _shapes;
};
