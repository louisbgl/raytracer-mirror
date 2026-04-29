#pragma once

#include "../DataTypes/Vec3.hpp"
#include <vector>
#include <memory>
#include <string>

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
     * @brief Gets the color of a specific pixel in the image.
     * @param x The x-coordinate of the pixel (0 to width-1).
     * @param y The y-coordinate of the pixel (0 to height-1).
     * @return The color of the pixel, represented as a Vec3 (R, G, B).
     */
    Vec3 getPixel(int x, int y) const;

    /**
     * @brief Samples the image using UV coordinates.
     * @param u Horizontal coordinate (0.0 to 1.0).
     * @param v Vertical coordinate (0.0 to 1.0).
     * @return The sampled color as a Vec3.
     */
    Vec3 sample(double u, double v) const;

    /**
     * @brief Writes the image to a file in PPM format.
     * @param filename The name of the file to write to.
     * @param binary Whether to write in binary (P6) or text (P3) format.
     */
    void writePPM(const std::string& filename, bool binary = true) const;

    /**
     * @brief Reads an image from a PPM file.
     * @param filename The name of the file to read from.
     * @return The loaded image, or nullptr if loading failed.
     */
    static std::unique_ptr<Image> readPPM(const std::string& filename);

private:
    int _width, _height;
    std::vector<Vec3> _pixels;

    void _writePPMp3(const std::string& filename) const;
    void _writePPMp6(const std::string& filename) const;
    static void _readPPMp3(int width, int height, std::ifstream& file, std::unique_ptr<Image>& image);
    static void _readPPMp6(int width, int height, std::ifstream& file, std::unique_ptr<Image>& image);
};