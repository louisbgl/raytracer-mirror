
#include "./ToastManager.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <algorithm>

ToastManager::ToastManager(sf::Font& given_font)
    : _font(given_font)
{
}

sf::Color ToastManager::getColourType(ToastType type) const
{
    switch (type) {
        case ToastType::Success: return {46, 204, 113, 220}; // Green
        case ToastType::Error:   return {231, 76, 60, 220};  // Red
        case ToastType::Warning: return {241, 196, 15, 220}; // Yellow
        case ToastType::Info:
        default:                 return {52, 152, 219, 220}; // Blue
    }
}

void ToastManager::push(const ToastConfig& config)
{
    ToastConfig toastConfig = config;
    if (toastConfig.bgColour == sf::Color::Transparent) {
        toastConfig.bgColour = getColourType(toastConfig.type);
    }

    _toasts.push_back({
        toastConfig,
        toastConfig.duration,
        1.0F
    });
}

void ToastManager::update(float delta)
{
    for (auto it = _toasts.begin(); it != _toasts.end();) {
        it->remaining -= delta;

        if (it->remaining <= 0.0F) {
            it = _toasts.erase(it);
            continue;
        }

        if (it->remaining < 0.5F) {
            it->alpha = std::clamp(it->remaining / 0.5F, 0.0F, 1.0F);
        }

        ++it;
    }
}

void ToastManager::draw(sf::RenderWindow& win)
{
    sf::Vector2u winSize = win.getSize();
    float footerHeight = std::max(80.0F, static_cast<float>(winSize.y) * 0.13F);
    float launchHeight = std::max(42.0F, footerHeight * 0.5F);
    float currentY = static_cast<float>(winSize.y) - footerHeight + (footerHeight - launchHeight) / 2.0F - (_padding * 0.5F);

    for (int i = static_cast<int>(_toasts.size()) - 1; i >= 0; --i) {
        const auto& toast = _toasts[i];
        sf::Text text(toast.config.msg, _font, 16);
        sf::FloatRect textBounds = text.getLocalBounds();

        float boxWidth = textBounds.width + 40.0F;
        float boxHeight = _toastHeight;

        sf::RectangleShape bg({boxWidth, boxHeight});
        bg.setOrigin(boxWidth / 2.0F, boxHeight / 2.0F);
        sf::Vector2f centerPos(static_cast<float>(winSize.x) / 2.0F,
                               currentY - (boxHeight / 2.0F));
        bg.setPosition(centerPos);

        sf::Color bgColor = toast.config.bgColour;
        bgColor.a = static_cast<sf::Uint8>(toast.alpha * bgColor.a);
        bg.setFillColor(bgColor);

        sf::Color textColor = toast.config.textColour;
        textColor.a = static_cast<sf::Uint8>(toast.alpha * 255);
        text.setFillColor(textColor);

        text.setOrigin(textBounds.left + textBounds.width / 2.0F,
                       textBounds.top + textBounds.height / 2.0F);
        text.setPosition(centerPos);

        win.draw(bg);
        win.draw(text);

        currentY -= (boxHeight + _padding);
    }
}
