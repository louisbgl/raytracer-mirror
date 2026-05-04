#include "SceneBrowser.hpp"
#include <algorithm>
#include <filesystem>

namespace {
    const sf::Color COL_HEADER   = {16, 16, 24};
    const sf::Color COL_LIST_BG  = {26, 26, 36};
    const sf::Color COL_FOOTER   = {18, 18, 28};
    const sf::Color COL_ITEM     = {34, 34, 46};
    const sf::Color COL_ITEM_ALT = {30, 30, 42};
    const sf::Color COL_HOVER    = {48, 78, 118};
    const sf::Color COL_SEL      = {38, 108, 178};
    const sf::Color COL_ACCENT   = {78, 158, 238};
    const sf::Color COL_BTN_ON   = {38, 158, 78};
    const sf::Color COL_BTN_OFF  = {66, 66, 80};
    const sf::Color COL_TEXT     = sf::Color::White;
    const sf::Color COL_SUBTEXT  = {170, 170, 185};
    const sf::Color COL_BORDER   = {52, 52, 70};
    const sf::Color COL_SEP      = {44, 44, 60};
}

SceneBrowser::SceneBrowser(sf::Font& font) : _font(font) { scan(); }

SceneBrowser::Layout SceneBrowser::computeLayout(const sf::RenderWindow& window) const {
    float ww = static_cast<float>(window.getSize().x);
    float wh = static_cast<float>(window.getSize().y);

    Layout lo;
    lo.margin   = std::max(48.f, ww * 0.05f);
    lo.headerH  = std::max(64.f, wh * 0.1f);
    lo.footerH  = std::max(80.f, wh * 0.13f);
    lo.itemH    = std::clamp(wh * 0.055f, 38.f, 58.f);
    lo.listLeft = lo.margin;
    lo.listTop  = lo.headerH;
    lo.listW    = ww - 2.f * lo.margin;
    lo.listH    = wh - lo.headerH - lo.footerH;
    lo.footerY  = lo.listTop + lo.listH;

    lo.launchW    = std::max(148.f, ww * 0.11f);
    lo.launchH    = std::max(42.f, lo.footerH * 0.5f);
    lo.launchLeft = ww - lo.margin - lo.launchW;
    lo.launchTop  = lo.footerY + (lo.footerH - lo.launchH) / 2.f;

    return lo;
}

void SceneBrowser::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    const auto lo = computeLayout(window);

    if (event.type == sf::Event::MouseWheelScrolled) {
        _scroll -= event.mouseWheelScroll.delta * lo.itemH;
        clampScroll(lo);
        return;
    }

    if (event.type == sf::Event::MouseMoved) {
        float mx = static_cast<float>(event.mouseMove.x);
        float my = static_cast<float>(event.mouseMove.y);
        _hoveredIdx = -1;
        for (size_t i = 0; i < _scenes.size(); ++i) {
            float y = lo.listTop + static_cast<float>(i) * lo.itemH - _scroll;
            if (y + lo.itemH < lo.listTop || y > lo.listTop + lo.listH) continue;
            if (my < lo.footerY && sf::FloatRect{lo.listLeft, y, lo.listW, lo.itemH}.contains(mx, my)) {
                _hoveredIdx = static_cast<int>(i);
                break;
            }
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);
        bool hitItem = false;
        for (size_t i = 0; i < _scenes.size(); ++i) {
            float y = lo.listTop + static_cast<float>(i) * lo.itemH - _scroll;
            if (y + lo.itemH < lo.listTop || y > lo.listTop + lo.listH) continue;
            if (my < lo.footerY && sf::FloatRect{lo.listLeft, y, lo.listW, lo.itemH}.contains(mx, my)) {
                _selectedIdx = static_cast<int>(i);
                _selected    = _scenes[_selectedIdx];
                hitItem      = true;
                break;
            }
        }
        if (!hitItem && _launchRect.contains(mx, my) && hasSelection())
            _sigLaunch = true;
    }
}

void SceneBrowser::draw(sf::RenderWindow& window) {
    const auto lo = computeLayout(window);
    drawHeader(window, lo);
    drawList(window, lo);
    drawFooter(window, lo);
}

void SceneBrowser::drawHeader(sf::RenderWindow& window, const Layout& lo) {
    float ww = static_cast<float>(window.getSize().x);

    sf::RectangleShape hdr({ww, lo.headerH});
    hdr.setFillColor(COL_HEADER);
    window.draw(hdr);

    // Bottom separator
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, lo.headerH - 1.f);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);

    unsigned titleSz = static_cast<unsigned>(std::clamp(lo.headerH * 0.38f, 18.f, 32.f));
    sf::Text title("SCENE BROWSER", _font, titleSz);
    title.setFillColor(COL_ACCENT);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left, tb.top + tb.height / 2.f);
    title.setPosition(lo.margin, lo.headerH / 2.f);
    window.draw(title);

    unsigned cntSz = static_cast<unsigned>(std::clamp(lo.headerH * 0.26f, 13.f, 20.f));
    sf::Text cnt(std::to_string(_scenes.size()) + " scene" + (_scenes.size() != 1 ? "s" : ""),
                 _font, cntSz);
    cnt.setFillColor(COL_SUBTEXT);
    sf::FloatRect cb = cnt.getLocalBounds();
    cnt.setOrigin(cb.left + cb.width, cb.top + cb.height / 2.f);
    cnt.setPosition(ww - lo.margin, lo.headerH / 2.f);
    window.draw(cnt);
}

void SceneBrowser::drawList(sf::RenderWindow& window, const Layout& lo) {
    sf::RectangleShape bg({lo.listW, lo.listH});
    bg.setPosition(lo.listLeft, lo.listTop);
    bg.setFillColor(COL_LIST_BG);
    window.draw(bg);

    if (_scenes.empty()) {
        unsigned emptySz = static_cast<unsigned>(std::max(15.f, lo.listH * 0.03f));
        sf::Text empty("No scenes found in ./scenes/", _font, emptySz);
        empty.setFillColor(COL_SUBTEXT);
        sf::FloatRect eb = empty.getLocalBounds();
        empty.setOrigin(eb.left + eb.width / 2.f, eb.top + eb.height / 2.f);
        empty.setPosition(lo.listLeft + lo.listW / 2.f, lo.listTop + lo.listH / 2.f);
        window.draw(empty);
        return;
    }

    for (size_t i = 0; i < _scenes.size(); ++i) {
        float y = lo.listTop + static_cast<float>(i) * lo.itemH - _scroll;
        if (y + lo.itemH < lo.listTop || y > lo.listTop + lo.listH) continue;
        drawItem(window, _scenes[i], static_cast<int>(i), y, lo);
    }
}

void SceneBrowser::drawItem(sf::RenderWindow& window, const std::string& path,
                             int idx, float y, const Layout& lo) {
    bool sel = (idx == _selectedIdx);
    bool hov = (idx == _hoveredIdx);

    sf::Color bg = sel ? COL_SEL
                 : hov ? COL_HOVER
                 : (idx % 2 == 0 ? COL_ITEM : COL_ITEM_ALT);

    float clampedY     = std::max(y, lo.listTop);
    float clampedBot   = std::min(y + lo.itemH, lo.listTop + lo.listH);
    float clampedH     = clampedBot - clampedY;
    if (clampedH <= 0.f) return;

    sf::RectangleShape row({lo.listW, clampedH});
    row.setPosition(lo.listLeft, clampedY);
    row.setFillColor(bg);
    window.draw(row);

    // selected bar
    if (sel) {
        sf::RectangleShape bar({4.f, clampedH});
        bar.setPosition(lo.listLeft, clampedY);
        bar.setFillColor(COL_ACCENT);
        window.draw(bar);
    }

    // separator line
    sf::RectangleShape sep({lo.listW, 1.f});
    sep.setPosition(lo.listLeft, clampedY + clampedH - 1.f);
    sep.setFillColor(COL_SEP);
    window.draw(sep);

    if (y + 8.f < lo.listTop || y + lo.itemH - 8.f > lo.listTop + lo.listH) return;

    std::string label = std::filesystem::path(path).stem().string();
    unsigned textSz = static_cast<unsigned>(std::clamp(lo.itemH * 0.42f, 14.f, 22.f));
    sf::Text text(label, _font, textSz);
    text.setFillColor(sel ? COL_TEXT : hov ? COL_TEXT : COL_SUBTEXT);
    sf::FloatRect tb = text.getLocalBounds();
    text.setOrigin(tb.left, tb.top + tb.height / 2.f);
    text.setPosition(lo.listLeft + (sel ? 20.f : 14.f), y + lo.itemH / 2.f);
    window.draw(text);
}

void SceneBrowser::drawFooter(sf::RenderWindow& window, const Layout& lo) {
    float ww = static_cast<float>(window.getSize().x);
    float wh = static_cast<float>(window.getSize().y);

    sf::RectangleShape footer({ww, lo.footerH});
    footer.setPosition(0.f, lo.footerY);
    footer.setFillColor(COL_FOOTER);
    window.draw(footer);

    // top separator
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, lo.footerY);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);

    unsigned selSz = static_cast<unsigned>(std::clamp(lo.footerH * 0.22f, 12.f, 18.f));
    std::string selLabel = hasSelection()
        ? std::filesystem::path(_selected).stem().string()
        : "No scene selected";
    sf::Text selTxt(selLabel, _font, selSz);
    selTxt.setFillColor(hasSelection() ? COL_TEXT : COL_SUBTEXT);
    sf::FloatRect slb = selTxt.getLocalBounds();
    selTxt.setOrigin(slb.left, slb.top + slb.height / 2.f);
    selTxt.setPosition(lo.margin, lo.footerY + lo.footerH / 2.f);
    window.draw(selTxt);

    // path display
    if (hasSelection()) {
        unsigned pathSz = static_cast<unsigned>(std::clamp(lo.footerH * 0.16f, 11.f, 15.f));
        sf::Text pathTxt(_selected, _font, pathSz);
        pathTxt.setFillColor(COL_SUBTEXT);
        sf::FloatRect plb = pathTxt.getLocalBounds();
        pathTxt.setOrigin(plb.left, plb.top);
        pathTxt.setPosition(lo.margin, lo.footerY + lo.footerH / 2.f + selSz * 0.7f);
        window.draw(pathTxt);
    }

    // launch button
    _launchRect = {lo.launchLeft, lo.launchTop, lo.launchW, lo.launchH};
    sf::Color btnBg = hasSelection() ? COL_BTN_ON : COL_BTN_OFF;
    sf::RectangleShape btn({lo.launchW, lo.launchH});
    btn.setPosition(lo.launchLeft, lo.launchTop);
    btn.setFillColor(btnBg);
    if (hasSelection()) {
        btn.setOutlineThickness(1.f);
        btn.setOutlineColor({58, 198, 98});
    }
    window.draw(btn);

    unsigned btnSz = static_cast<unsigned>(std::clamp(lo.launchH * 0.42f, 14.f, 20.f));
    sf::Text btnTxt(hasSelection() ? "Launch " : "Launch", _font, btnSz);
    btnTxt.setFillColor(COL_TEXT);
    sf::FloatRect bb = btnTxt.getLocalBounds();
    btnTxt.setOrigin(bb.left + bb.width / 2.f, bb.top + bb.height / 2.f);
    btnTxt.setPosition(lo.launchLeft + lo.launchW / 2.f, lo.launchTop + lo.launchH / 2.f);
    window.draw(btnTxt);

    (void)wh;
}

void SceneBrowser::clearSelection() {
    _selected.clear();
    _selectedIdx = -1;
}

void SceneBrowser::refresh() {
    _scenes.clear();
    _scroll = 0.f;
    scan();
}

void SceneBrowser::scan() {
    namespace fs = std::filesystem;
    if (!fs::exists(SCENES_DIR)) return;
    for (const auto& entry : fs::directory_iterator(SCENES_DIR))
        if (entry.is_regular_file() && entry.path().extension() == FILE_EXT)
            _scenes.push_back(entry.path().string());
    std::sort(_scenes.begin(), _scenes.end());
}

void SceneBrowser::clampScroll(const Layout& lo) {
    float maxScroll = std::max(0.f, static_cast<float>(_scenes.size()) * lo.itemH - lo.listH);
    _scroll = std::clamp(_scroll, 0.f, maxScroll);
}
