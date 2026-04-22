#pragma once

#include <string>

class HelpDisplay {
public:
    explicit HelpDisplay(const std::string& filepath = "help.txt");
    void display() const;

private:
    std::string _filepath;
};
