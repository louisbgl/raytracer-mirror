#include "RenderPanel.hpp"
#include "SFMLHelpers.hpp"
#include <iomanip>
#include <sstream>

namespace {
    const sf::Color COL_HEADER     = {16, 16, 24};
    const sf::Color COL_ACCENT     = {78, 158, 238};
    const sf::Color COL_DONE       = {68, 210, 110};
    const sf::Color COL_TEXT       = sf::Color::White;
    const sf::Color COL_SUBTEXT    = {170, 170, 185};
    const sf::Color COL_BAR_BG     = {46, 46, 62};
    const sf::Color COL_BAR_FG     = {58, 138, 218};
    const sf::Color COL_CANCEL     = {198, 58, 58};
    const sf::Color COL_BACK       = {38, 158, 78};
    const sf::Color COL_BORDER     = {52, 52, 70};
    const sf::Color COL_TOGGLE_ON  = {58, 138, 218};
    const sf::Color COL_TOGGLE_OFF = {60, 60, 80};
    const sf::Color COL_OVERLAY    = {0, 0, 0, 160};

    constexpr float CONTROLS_H   = 130.f;
    constexpr float HEADER_RATIO = 0.1f;
    constexpr float MIN_HEADER_H = 64.f;
}

// --- public ------------------------------------------------------------------

RenderPanel::RenderPanel(sf::Font& font) : _font(font) {}

void RenderPanel::setScene(const std::string& path) { _scenePath = path; }

void RenderPanel::update(const PixelBuffer& buf) {
    _rows  = buf.rowsComplete.load();
    _total = buf.totalRows.load();

    std::lock_guard<std::mutex> lock(buf.mutex);
    int w = buf.width, h = buf.height;
    if (w <= 0 || h <= 0 || static_cast<int>(buf.rgba.size()) != w * h * 4)
        return;

    if (_renderTex.getSize().x != static_cast<unsigned>(w) || _renderTex.getSize().y != static_cast<unsigned>(h)) {
        _renderTex.create(w, h);
        _renderSprite.setTexture(_renderTex, true);
    }
    _renderTex.update(reinterpret_cast<const sf::Uint8*>(buf.rgba.data()));
    _hasTexture = true;
}

void RenderPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type != sf::Event::MouseButtonPressed) return;
    if (wasClicked(event, window, _cancelRect)) _sigCancel = true;
    if (wasClicked(event, window, _backRect))   _sigBack   = true;
    if (wasClicked(event, window, _liveRect))   _liveMode  = !_liveMode;
}

void RenderPanel::drawRendering(sf::RenderWindow& window) {
    float ww        = static_cast<float>(window.getSize().x);
    float wh        = static_cast<float>(window.getSize().y);
    float cx        = ww / 2.f;
    float headerH   = std::max(MIN_HEADER_H, wh * HEADER_RATIO);
    float controlsH = CONTROLS_H;
    float stripY    = wh - controlsH;
    float imgAreaY  = headerH;
    float imgAreaH  = wh - headerH - controlsH;

    drawHeader(window, ww, headerH);
    drawRenderingTitle(window, ww, cx, headerH);
    drawLiveArea(window, ww, cx, imgAreaY, imgAreaH);
    drawProgressStrip(window, stripY, ww, controlsH);
    drawCancelButton(window, ww, stripY, controlsH);
    drawLiveToggle(window, ww, stripY, controlsH);
    drawReloadIndicator(window, ww, wh);
}

void RenderPanel::drawDone(sf::RenderWindow& window) {
    float ww        = static_cast<float>(window.getSize().x);
    float wh        = static_cast<float>(window.getSize().y);
    float cx        = ww / 2.f;
    float headerH   = std::max(MIN_HEADER_H, wh * HEADER_RATIO);
    float controlsH = CONTROLS_H;
    float stripY    = wh - controlsH;
    float imgAreaY  = headerH;
    float imgAreaH  = wh - headerH - controlsH;

    drawHeader(window, ww, headerH);
    drawDoneTitle(window, cx, headerH);
    drawImageFitted(window, 0.f, imgAreaY, ww, imgAreaH);
    drawDoneStrip(window, ww, cx, stripY, controlsH);
}

// --- helpers -----------------------------------------------------------------

void RenderPanel::drawHeader(sf::RenderWindow& window, float ww, float headerH) {
    sf::RectangleShape hdr({ww, headerH});
    hdr.setFillColor(COL_HEADER);
    window.draw(hdr);
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, headerH - 1.f);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);
}

void RenderPanel::drawImageFitted(sf::RenderWindow& window, float x, float y, float w, float h) {
    if (!_hasTexture) return;
    sf::Vector2u ts = _renderTex.getSize();
    if (ts.x == 0 || ts.y == 0) return;

    float scaleX = w / static_cast<float>(ts.x);
    float scaleY = h / static_cast<float>(ts.y);
    float scale  = std::min(scaleX, scaleY);
    float drawW  = ts.x * scale;
    float drawH  = ts.y * scale;
    float imgX   = x + (w - drawW) / 2.f;
    float imgY   = y + (h - drawH) / 2.f;

    _renderSprite.setScale(scale, scale);
    _renderSprite.setPosition(imgX, imgY);
    window.draw(_renderSprite);
}

void RenderPanel::drawButton(sf::RenderWindow& window, sf::FloatRect rect,
                             const std::string& label, sf::Color bg) {
    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition(rect.left, rect.top);
    shape.setFillColor(bg);
    window.draw(shape);

    unsigned sz = static_cast<unsigned>(std::clamp(rect.height * 0.42f, 13.f, 20.f));
    sf::Text text(label, _font, sz);
    text.setFillColor(COL_TEXT);
    sfh::centerXY(text, rect.left + rect.width / 2.f, rect.top + rect.height / 2.f);
    window.draw(text);
}

bool RenderPanel::wasClicked(const sf::Event& event, const sf::RenderWindow& window,
                             sf::FloatRect rect) const {
    if (event.type != sf::Event::MouseButtonPressed) return false;
    sf::Vector2f pos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
    return rect.contains(pos);
}

void RenderPanel::drawRenderingTitle(sf::RenderWindow& window, float ww, float cx, float headerH) {
    unsigned titleSz = static_cast<unsigned>(std::clamp(headerH * 0.38f, 18.f, 32.f));
    sf::Text title("RENDERING", _font, titleSz);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(COL_ACCENT);
    sfh::centerXY(title, cx, headerH / 2.f);
    window.draw(title);

    unsigned pathSz = static_cast<unsigned>(std::clamp(headerH * 0.24f, 12.f, 18.f));
    sf::Text sceneTxt(_scenePath, _font, pathSz);
    sceneTxt.setFillColor(COL_SUBTEXT);
    sfh::alignRight(sceneTxt, ww - std::max(48.f, ww * 0.05f), headerH / 2.f);
    window.draw(sceneTxt);
}

void RenderPanel::drawLiveArea(sf::RenderWindow& window, float ww, float cx, float imgY, float imgH) {
    if (_liveMode && _hasTexture) {
        drawImageFitted(window, 0.f, imgY, ww, imgH);
    } else if (!_liveMode) {
        unsigned sz = static_cast<unsigned>(std::clamp(imgH * 0.05f, 18.f, 36.f));
        sf::Text waitTxt("Render in progress. Live mode is off", _font, sz);
        waitTxt.setFillColor(COL_SUBTEXT);
        sfh::centerXY(waitTxt, cx, imgY + imgH / 2.f);
        window.draw(waitTxt);
    }
}

void RenderPanel::drawProgressStrip(sf::RenderWindow& window, float stripY, float ww, float stripH) {
    float cx = ww / 2.f;

    sf::RectangleShape bg({ww, stripH});
    bg.setPosition(0.f, stripY);
    bg.setFillColor(COL_OVERLAY);
    window.draw(bg);

    float barW = std::min(ww * 0.55f, 700.f);
    float barH = std::clamp(stripH * 0.18f, 12.f, 22.f);
    float barX = cx - barW / 2.f;
    float barY = stripY + stripH * 0.18f;

    sf::RectangleShape barBg({barW, barH});
    barBg.setPosition(barX, barY);
    barBg.setFillColor(COL_BAR_BG);
    barBg.setOutlineThickness(1.f);
    barBg.setOutlineColor(COL_BORDER);
    window.draw(barBg);

    float pct = (_total > 0) ? std::clamp(static_cast<float>(_rows) / static_cast<float>(_total), 0.f, 1.f) : 0.f;
    if (pct > 0.f) {
        sf::RectangleShape fill({barW * pct, barH});
        fill.setPosition(barX, barY);
        fill.setFillColor(COL_BAR_FG);
        window.draw(fill);
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (pct * 100.f) << "%  ("
        << _rows << " / " << _total << " rows)";
    unsigned textSz = static_cast<unsigned>(std::clamp(stripH * 0.16f, 12.f, 17.f));
    sf::Text info(oss.str(), _font, textSz);
    info.setFillColor(COL_SUBTEXT);
    sfh::centerXTop(info, cx, barY + barH + 6.f);
    window.draw(info);
}

void RenderPanel::drawCancelButton(sf::RenderWindow& window, float ww, float stripY, float controlsH) {
    float btnW = std::max(110.f, ww * 0.09f);
    float btnH = std::max(36.f, controlsH * 0.42f);
    _cancelRect = {ww - btnW - std::max(24.f, ww * 0.03f), stripY + controlsH * 0.52f, btnW, btnH};
    drawButton(window, _cancelRect, "Cancel", COL_CANCEL);
}

void RenderPanel::drawLiveToggle(sf::RenderWindow& window, float ww, float stripY, float controlsH) {
    float btnW = std::max(110.f, ww * 0.09f);
    float btnH = std::max(36.f, controlsH * 0.42f);
    _liveRect = {std::max(24.f, ww * 0.03f), stripY + controlsH * 0.52f, btnW, btnH};
    drawButton(window, _liveRect, _liveMode ? "Live ON" : "Live OFF",
               _liveMode ? COL_TOGGLE_ON : COL_TOGGLE_OFF);
}

void RenderPanel::drawDoneTitle(sf::RenderWindow& window, float cx, float headerH) {
    unsigned titleSz = static_cast<unsigned>(std::clamp(headerH * 0.38f, 18.f, 32.f));
    sf::Text title("RENDER COMPLETE", _font, titleSz);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(COL_DONE);
    sfh::centerXY(title, cx, headerH / 2.f);
    window.draw(title);
}

void RenderPanel::drawDoneStrip(sf::RenderWindow& window, float ww, float cx, float stripY, float controlsH) {
    sf::RectangleShape bg({ww, controlsH});
    bg.setPosition(0.f, stripY);
    bg.setFillColor(COL_OVERLAY);
    window.draw(bg);

    unsigned pathSz = static_cast<unsigned>(std::clamp(controlsH * 0.16f, 12.f, 17.f));
    sf::Text pathTxt(_scenePath, _font, pathSz);
    pathTxt.setFillColor(COL_SUBTEXT);
    sfh::centerXTop(pathTxt, cx, stripY + controlsH * 0.14f);
    window.draw(pathTxt);

    float btnW = std::max(160.f, ww * 0.13f);
    float btnH = std::max(38.f, controlsH * 0.42f);
    _backRect = {cx - btnW / 2.f, stripY + controlsH * 0.52f, btnW, btnH};
    drawButton(window, _backRect, "Back to Browser", COL_BACK);
}

void RenderPanel::drawReloadIndicator(sf::RenderWindow& window, float ww, float wh) {
    if (!_showReloadIndicator) return;

    float indicatorSize = std::clamp(wh * 0.025f, 12.f, 20.f);
    sf::CircleShape indicator(indicatorSize);
    indicator.setFillColor(sf::Color::Yellow);
    indicator.setPosition(ww - indicatorSize * 3.f, indicatorSize);
    window.draw(indicator);

    unsigned labelSz = static_cast<unsigned>(std::clamp(wh * 0.018f, 11.f, 16.f));
    sf::Text label("RELOADING", _font, labelSz);
    label.setFillColor(sf::Color::Yellow);
    sfh::alignRight(label, ww - indicatorSize * 5.5f, indicatorSize * 2.f);
    window.draw(label);
}
