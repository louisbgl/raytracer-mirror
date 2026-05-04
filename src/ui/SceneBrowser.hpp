#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <vector>

class SceneBrowser {
public:
    explicit SceneBrowser(sf::Font& font);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    const std::string& selected() const { return _selected; }
    bool               hasSelection()   const { return !_selected.empty(); }
    void               clearSelection();
    void               refresh();

    bool wantsLaunch() const { return _sigLaunch; }
    void clearSignals()      { _sigLaunch = false; }

private:
    static constexpr std::string_view SCENES_DIR = "./scenes/";
    static constexpr std::string_view FILE_EXT   = ".txt";

    struct Layout {
        float margin;
        float headerH, footerH;
        float listLeft, listTop, listW, listH, itemH;
        float footerY;
        float launchW, launchH, launchLeft, launchTop;
    };

    Layout computeLayout(const sf::RenderWindow& window) const;

    void scan();
    void clampScroll(const Layout& lo);

    void drawHeader(sf::RenderWindow& window, const Layout& lo);
    void drawList(sf::RenderWindow& window, const Layout& lo);
    void drawItem(sf::RenderWindow& window, const std::string& path,
                  int idx, float y, const Layout& lo);
    void drawFooter(sf::RenderWindow& window, const Layout& lo);

    sf::Font&                _font;
    std::vector<std::string> _scenes;
    std::string              _selected;
    int                      _hoveredIdx  = -1;
    int                      _selectedIdx = -1;
    float                    _scroll      = 0.f;
    sf::FloatRect            _launchRect  = {};
    bool                     _sigLaunch   = false;
};
