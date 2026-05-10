
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <string>
#include <vector>

enum class ToastType {
    Info,
    Success,
    Warning,
    Error,
};

struct ToastConfig {
    std::string msg;
    sf::Color textColour = sf::Color::White;
    sf::Color bgColour = sf::Color::Transparent;
    float duration = 3.0f;
    ToastType type = ToastType::Info;
};

class ToastManager
{
    public:
        ToastManager(sf::Font& font);
        ~ToastManager() = default;

        void push(const ToastConfig& tconfig);
        void update(float delta);
        void draw(sf::RenderWindow& win);
    private:
        struct ActiveToast {
            ToastConfig config;
            float remaining;
            float alpha;
        };

        [[nodiscard]] sf::Color getColourType(ToastType type) const;
        std::vector<ActiveToast> _toasts;
        sf::Font& _font;
        const float _toastHeight = 40.0f;
        const float _padding = 10.0f;
};
