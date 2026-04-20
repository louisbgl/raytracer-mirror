#pragma once

#include "../DataTypes/Vec3.hpp"
#include <vector>

class Image {
public:
    Image(int width, int height) : _width(width), _height(height), _pixels(width * height) {}
    ~Image() = default;

    int width() const { return _width; }
    int height() const { return _height; }

    /**
     * @brief Sets the color of a specific pixel in the image.
     * @param x The x-coordinate of the pixel (0 to width-1).
     * @param y The y-coordinate of the pixel (0 to height-1).
     * @param color The color to set for the pixel, represented as a Vec3 (R, G, B).
     */
    void setPixel(int x, int y, const Vec3& color);

    /**
     * @brief Writes the image to a file in PPM format.
     * @param filename The name of the file to write to.
     */
    void writePPM(const std::string& filename) const;

private:
    int _width, _height;
    std::vector<Vec3> _pixels;
};