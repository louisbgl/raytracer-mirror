#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../DataTypes/Vec3.hpp"
#include "../Interfaces/IMaterial.hpp"
#include "../Interfaces/IShape.hpp"

class ObjParser {
public:
    ObjParser() = default;
    ~ObjParser() = default;

    std::vector<std::shared_ptr<IShape>> parse(const std::string& filename, std::shared_ptr<IMaterial> material);

private:
    void parseVertex(const std::string& line);
    void parseFace(const std::string& line, std::shared_ptr<IMaterial> material);

    std::vector<Vec3> vertices;
    std::vector<std::shared_ptr<IShape>> shapes;
};