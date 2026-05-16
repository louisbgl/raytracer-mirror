#include "SceneBrowser.hpp"
#include "SFMLHelpers.hpp"
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

// --- public ------------------------------------------------------------------

SceneBrowser::SceneBrowser(sf::Font& font) : _font(font) { scan(); }

void SceneBrowser::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    const auto lo = computeLayout(window);
    if (event.type == sf::Event::MouseWheelScrolled) { handleScroll(event, lo);      return; }
    if (event.type == sf::Event::MouseMoved)          { handleMouseMove(event, lo);   return; }
    if (event.type == sf::Event::MouseButtonPressed)  { handleMouseClick(event, lo);  return; }
}

void SceneBrowser::draw(sf::RenderWindow& window, const PixelBuffer* previewBuf) {
    const auto lo = computeLayout(window);
    drawHeader(window, lo);
    drawList(window, lo);
    drawPreviewPane(window, lo, previewBuf);
    drawFooter(window, lo);
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

// --- helpers -----------------------------------------------------------------

SceneBrowser::Layout SceneBrowser::computeLayout(const sf::RenderWindow& window) const {
    float ww = static_cast<float>(window.getSize().x);
    float wh = static_cast<float>(window.getSize().y);

    Layout lo;
    lo.margin  = std::max(48.f, ww * 0.05f);
    lo.headerH = std::max(64.f, wh * 0.1f);
    lo.footerH = std::max(80.f, wh * 0.13f);
    lo.itemH   = std::clamp(wh * 0.055f, 38.f, 58.f);
    lo.listTop = lo.headerH;
    lo.listH   = wh - lo.headerH - lo.footerH;
    lo.footerY = lo.listTop + lo.listH;

    float contentW  = ww - 2.f * lo.margin;
    lo.listLeft    = lo.margin;
    lo.listW       = contentW * 0.38f;
    lo.dividerX    = lo.listLeft + lo.listW;
    lo.previewLeft = lo.dividerX + 1.f;
    lo.previewW    = ww - lo.margin - lo.previewLeft;

    lo.launchW    = std::max(148.f, ww * 0.11f);
    lo.launchH    = std::max(42.f, lo.footerH * 0.5f);
    lo.launchLeft = ww - lo.margin - lo.launchW;
    lo.launchTop  = lo.footerY + (lo.footerH - lo.launchH) / 2.f;

    return lo;
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

void SceneBrowser::handleScroll(const sf::Event& event, const Layout& lo) {
    _scroll -= event.mouseWheelScroll.delta * lo.itemH;
    clampScroll(lo);
}

void SceneBrowser::handleMouseMove(const sf::Event& event, const Layout& lo) {
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
}

void SceneBrowser::handleMouseClick(const sf::Event& event, const Layout& lo) {
    float mx = static_cast<float>(event.mouseButton.x);
    float my = static_cast<float>(event.mouseButton.y);
    for (size_t i = 0; i < _scenes.size(); ++i) {
        float y = lo.listTop + static_cast<float>(i) * lo.itemH - _scroll;
        if (y + lo.itemH < lo.listTop || y > lo.listTop + lo.listH) continue;
        if (my < lo.footerY && sf::FloatRect{lo.listLeft, y, lo.listW, lo.itemH}.contains(mx, my)) {
            int newIdx = static_cast<int>(i);
            if (newIdx != _selectedIdx) {
                _selectedIdx = newIdx;
                _selected    = _scenes[_selectedIdx];
                _sigSelectionChanged = true;
            }
            return;
        }
    }
    if (_launchRect.contains(mx, my) && hasSelection())
        _sigLaunch = true;
}

void SceneBrowser::drawHeader(sf::RenderWindow& window, const Layout& lo) {
    float ww = static_cast<float>(window.getSize().x);

    sf::RectangleShape hdr({ww, lo.headerH});
    hdr.setFillColor(COL_HEADER);
    window.draw(hdr);
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, lo.headerH - 1.f);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);

    unsigned titleSz = static_cast<unsigned>(std::clamp(lo.headerH * 0.38f, 18.f, 32.f));
    sf::Text title("SCENE BROWSER", _font, titleSz);
    title.setFillColor(COL_ACCENT);
    title.setStyle(sf::Text::Bold);
    sfh::alignLeft(title, lo.margin, lo.headerH / 2.f);
    window.draw(title);

    unsigned cntSz = static_cast<unsigned>(std::clamp(lo.headerH * 0.26f, 13.f, 20.f));
    sf::Text cnt(std::to_string(_scenes.size()) + " scene" + (_scenes.size() != 1 ? "s" : ""),
                 _font, cntSz);
    cnt.setFillColor(COL_SUBTEXT);
    sfh::alignRight(cnt, ww - lo.margin, lo.headerH / 2.f);
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
        sfh::centerXY(empty, lo.listLeft + lo.listW / 2.f, lo.listTop + lo.listH / 2.f);
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

    sf::Color bg = sel ? COL_SEL : hov ? COL_HOVER : (idx % 2 == 0 ? COL_ITEM : COL_ITEM_ALT);

    float clampedY   = std::max(y, lo.listTop);
    float clampedBot = std::min(y + lo.itemH, lo.listTop + lo.listH);
    float clampedH   = clampedBot - clampedY;
    if (clampedH <= 0.f) return;

    sf::RectangleShape row({lo.listW, clampedH});
    row.setPosition(lo.listLeft, clampedY);
    row.setFillColor(bg);
    window.draw(row);

    if (sel) {
        sf::RectangleShape bar({4.f, clampedH});
        bar.setPosition(lo.listLeft, clampedY);
        bar.setFillColor(COL_ACCENT);
        window.draw(bar);
    }

    sf::RectangleShape sep({lo.listW, 1.f});
    sep.setPosition(lo.listLeft, clampedY + clampedH - 1.f);
    sep.setFillColor(COL_SEP);
    window.draw(sep);

    if (y + 8.f < lo.listTop || y + lo.itemH - 8.f > lo.listTop + lo.listH) return;

    std::string label = std::filesystem::path(path).stem().string();
    unsigned textSz = static_cast<unsigned>(std::clamp(lo.itemH * 0.42f, 14.f, 22.f));
    sf::Text text(label, _font, textSz);
    text.setFillColor(sel ? COL_TEXT : hov ? COL_TEXT : COL_SUBTEXT);
    sfh::alignLeft(text, lo.listLeft + (sel ? 20.f : 14.f), y + lo.itemH / 2.f);
    window.draw(text);
}

void SceneBrowser::drawPreviewPane(sf::RenderWindow& window, const Layout& lo, const PixelBuffer* previewBuf) {
    drawPreviewBackground(window, lo);
    drawPreviewContent(window, lo, previewBuf);
}

void SceneBrowser::drawPreviewBackground(sf::RenderWindow& window, const Layout& lo) {
    sf::RectangleShape divider({1.f, lo.listH});
    divider.setPosition(lo.dividerX, lo.listTop);
    divider.setFillColor(COL_BORDER);
    window.draw(divider);

    sf::RectangleShape bg({lo.previewW, lo.listH});
    bg.setPosition(lo.previewLeft, lo.listTop);
    bg.setFillColor({18, 18, 28});
    window.draw(bg);
}

void SceneBrowser::drawPreviewContent(sf::RenderWindow& window, const Layout& lo, const PixelBuffer* previewBuf) {
    float cx = lo.previewLeft + lo.previewW / 2.f;
    float cy = lo.listTop + lo.listH / 2.f;

    if (!hasSelection()) {
        unsigned sz = static_cast<unsigned>(std::clamp(lo.listH * 0.045f, 14.f, 20.f));
        sf::Text hint("Select a scene to preview", _font, sz);
        hint.setFillColor(COL_SUBTEXT);
        sfh::centerXY(hint, cx, cy);
        window.draw(hint);
        return;
    }

    float namePad = std::max(16.f, lo.listH * 0.04f);
    unsigned nameSz = static_cast<unsigned>(std::clamp(lo.listH * 0.05f, 15.f, 22.f));
    sf::Text name(std::filesystem::path(_selected).stem().string(), _font, nameSz);
    name.setFillColor(COL_ACCENT);
    name.setStyle(sf::Text::Bold);
    sfh::centerXTop(name, cx, lo.listTop + namePad);
    window.draw(name);

    float frameMargin = std::max(24.f, lo.previewW * 0.08f);
    float frameTop    = lo.listTop + namePad + nameSz + namePad;
    float frameBot    = lo.listTop + lo.listH - frameMargin;
    float frameW      = lo.previewW - 2.f * frameMargin;
    float frameH      = frameBot - frameTop;
    if (frameH <= 20.f) return;

    float frameX  = lo.previewLeft + frameMargin;
    bool hasPixels = false;

    if (previewBuf && previewBuf->totalRows.load() > 0) {
        std::lock_guard<std::mutex> lock(previewBuf->mutex);
        int pw = previewBuf->width;
        int ph = previewBuf->height;
        if (pw > 0 && ph > 0 && static_cast<int>(previewBuf->rgba.size()) == pw * ph * 4) {
            if (_previewTex.getSize().x != static_cast<unsigned>(pw) ||
                _previewTex.getSize().y != static_cast<unsigned>(ph))
                _previewTex.create(pw, ph);
            _previewTex.update(reinterpret_cast<const sf::Uint8*>(previewBuf->rgba.data()));

            float scale = std::min(frameW / static_cast<float>(pw), frameH / static_cast<float>(ph));
            float imgW  = pw * scale;
            float imgH  = ph * scale;
            sf::Sprite sprite(_previewTex);
            sprite.setScale(scale, scale);
            sprite.setPosition(frameX + (frameW - imgW) / 2.f, frameTop + (frameH - imgH) / 2.f);
            window.draw(sprite);
            hasPixels = true;
        }
    }

    if (!hasPixels) {
        sf::RectangleShape frame({frameW, frameH});
        frame.setPosition(frameX, frameTop);
        frame.setFillColor({26, 28, 40});
        frame.setOutlineThickness(1.f);
        frame.setOutlineColor(COL_BORDER);
        window.draw(frame);

        unsigned hintSz = static_cast<unsigned>(std::clamp(frameH * 0.07f, 12.f, 16.f));
        sf::Text hint("rendering preview...", _font, hintSz);
        hint.setFillColor({60, 60, 80});
        sfh::centerXY(hint, frameX + frameW / 2.f, frameTop + frameH / 2.f);
        window.draw(hint);
    }
}

void SceneBrowser::drawFooter(sf::RenderWindow& window, const Layout& lo) {
    float ww = static_cast<float>(window.getSize().x);

    sf::RectangleShape footer({ww, lo.footerH});
    footer.setPosition(0.f, lo.footerY);
    footer.setFillColor(COL_FOOTER);
    window.draw(footer);
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, lo.footerY);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);

    drawFooterSelectionInfo(window, lo);
    drawLaunchButton(window, lo);
}

void SceneBrowser::drawFooterSelectionInfo(sf::RenderWindow& window, const Layout& lo) {
    unsigned selSz = static_cast<unsigned>(std::clamp(lo.footerH * 0.22f, 12.f, 18.f));
    std::string selLabel = hasSelection()
        ? std::filesystem::path(_selected).stem().string()
        : "No scene selected";
    sf::Text selTxt(selLabel, _font, selSz);
    selTxt.setFillColor(hasSelection() ? COL_TEXT : COL_SUBTEXT);
    sfh::alignLeft(selTxt, lo.margin, lo.footerY + lo.footerH / 2.f);
    window.draw(selTxt);

    if (hasSelection()) {
        unsigned pathSz = static_cast<unsigned>(std::clamp(lo.footerH * 0.16f, 11.f, 15.f));
        sf::Text pathTxt(_selected, _font, pathSz);
        pathTxt.setFillColor(COL_SUBTEXT);
        sfh::alignLeftTop(pathTxt, lo.margin, lo.footerY + lo.footerH / 2.f + selSz * 0.7f);
        window.draw(pathTxt);
    }
}

void SceneBrowser::drawLaunchButton(sf::RenderWindow& window, const Layout& lo) {
    _launchRect = {lo.launchLeft, lo.launchTop, lo.launchW, lo.launchH};

    sf::RectangleShape btn({lo.launchW, lo.launchH});
    btn.setPosition(lo.launchLeft, lo.launchTop);
    btn.setFillColor(hasSelection() ? COL_BTN_ON : COL_BTN_OFF);
    if (hasSelection()) {
        btn.setOutlineThickness(1.f);
        btn.setOutlineColor({58, 198, 98});
    }
    window.draw(btn);

    unsigned btnSz = static_cast<unsigned>(std::clamp(lo.launchH * 0.42f, 14.f, 20.f));
    sf::Text btnTxt(hasSelection() ? "Launch " : "Launch", _font, btnSz);
    btnTxt.setFillColor(COL_TEXT);
    sfh::centerXY(btnTxt, lo.launchLeft + lo.launchW / 2.f, lo.launchTop + lo.launchH / 2.f);
    window.draw(btnTxt);
}
