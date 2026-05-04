#include "RenderPanel.hpp"
#include <iomanip>
#include <sstream>

namespace {
    const sf::Color COL_HEADER  = {16, 16, 24};
    const sf::Color COL_ACCENT  = {78, 158, 238};
    const sf::Color COL_DONE    = {68, 210, 110};
    const sf::Color COL_TEXT    = sf::Color::White;
    const sf::Color COL_SUBTEXT = {170, 170, 185};
    const sf::Color COL_BAR_BG  = {46, 46, 62};
    const sf::Color COL_BAR_FG  = {58, 138, 218};
    const sf::Color COL_CANCEL  = {198, 58, 58};
    const sf::Color COL_BACK    = {38, 158, 78};
    const sf::Color COL_BORDER  = {52, 52, 70};
}

RenderPanel::RenderPanel(sf::Font& font) : _font(font) {}

void RenderPanel::setScene(const std::string& path) { _scenePath = path; }

void RenderPanel::update(const PixelBuffer& buf) {
    _rows  = buf.rowsComplete.load();
    _total = buf.totalRows.load();
}

void RenderPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type != sf::Event::MouseButtonPressed) return;
    if (wasClicked(event, window, _cancelRect)) _sigCancel = true;
    if (wasClicked(event, window, _backRect))   _sigBack   = true;
}

void RenderPanel::drawHeader(sf::RenderWindow& window, float ww, float headerH) {
    sf::RectangleShape hdr({ww, headerH});
    hdr.setFillColor(COL_HEADER);
    window.draw(hdr);
    sf::RectangleShape sep({ww, 1.f});
    sep.setPosition(0.f, headerH - 1.f);
    sep.setFillColor(COL_BORDER);
    window.draw(sep);
}

void RenderPanel::drawRendering(sf::RenderWindow& window) {
    float ww = static_cast<float>(window.getSize().x);
    float wh = static_cast<float>(window.getSize().y);
    float cx = ww / 2.f;

    float headerH = std::max(64.f, wh * 0.1f);
    drawHeader(window, ww, headerH);

    unsigned titleSz = static_cast<unsigned>(std::clamp(headerH * 0.38f, 18.f, 32.f));
    sf::Text title("RENDERING", _font, titleSz);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(COL_ACCENT);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(cx, headerH / 2.f);
    window.draw(title);

    // path (right side)
    unsigned pathSz = static_cast<unsigned>(std::clamp(headerH * 0.24f, 12.f, 18.f));
    sf::Text sceneTxt(_scenePath, _font, pathSz);
    sceneTxt.setFillColor(COL_SUBTEXT);
    sf::FloatRect sb = sceneTxt.getLocalBounds();
    sceneTxt.setOrigin(sb.left + sb.width, sb.top + sb.height / 2.f);
    sceneTxt.setPosition(ww - std::max(48.f, ww * 0.05f), headerH / 2.f);
    window.draw(sceneTxt);

    // progress bar
    float contentY  = headerH + (wh - headerH) * 0.3f;
    float barW      = std::min(ww * 0.65f, 900.f);
    float barH      = std::clamp(wh * 0.022f, 16.f, 26.f);
    float barX      = cx - barW / 2.f;

    sf::RectangleShape barBg({barW, barH});
    barBg.setPosition(barX, contentY);
    barBg.setFillColor(COL_BAR_BG);
    barBg.setOutlineThickness(1.f);
    barBg.setOutlineColor(COL_BORDER);
    window.draw(barBg);

    float pct = (_total > 0) ? static_cast<float>(_rows) / static_cast<float>(_total) : 0.f;
    pct = std::clamp(pct, 0.f, 1.f);
    if (pct > 0.f) {
        sf::RectangleShape barFill({barW * pct, barH});
        barFill.setPosition(barX, contentY);
        barFill.setFillColor(COL_BAR_FG);
        window.draw(barFill);
    }

    // percentage
    unsigned pctSz = static_cast<unsigned>(std::clamp(wh * 0.038f, 22.f, 42.f));
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (pct * 100.f) << "%";
    sf::Text pctTxt(oss.str(), _font, pctSz);
    pctTxt.setStyle(sf::Text::Bold);
    pctTxt.setFillColor(COL_TEXT);
    sf::FloatRect pb = pctTxt.getLocalBounds();
    pctTxt.setOrigin(pb.left + pb.width / 2.f, pb.top);
    pctTxt.setPosition(cx, contentY + barH + std::max(12.f, wh * 0.015f));
    window.draw(pctTxt);

    // row count
    unsigned rowSz = static_cast<unsigned>(std::clamp(wh * 0.02f, 13.f, 18.f));
    sf::Text rowTxt(std::to_string(_rows) + " / " + std::to_string(_total) + " rows", _font, rowSz);
    rowTxt.setFillColor(COL_SUBTEXT);
    sf::FloatRect rb = rowTxt.getLocalBounds();
    rowTxt.setOrigin(rb.left + rb.width / 2.f, rb.top);
    rowTxt.setPosition(cx, contentY + barH + pctSz + std::max(18.f, wh * 0.022f));
    window.draw(rowTxt);

    // cancel button
    float btnW = std::max(130.f, ww * 0.1f);
    float btnH = std::max(42.f, wh * 0.057f);
    _cancelRect = {cx - btnW / 2.f, wh * 0.74f, btnW, btnH};
    drawButton(window, _cancelRect, "Cancel", COL_CANCEL);
}

void RenderPanel::drawDone(sf::RenderWindow& window) {
    float ww = static_cast<float>(window.getSize().x);
    float wh = static_cast<float>(window.getSize().y);
    float cx = ww / 2.f;
    float cy = wh / 2.f;

    // header
    float headerH = std::max(64.f, wh * 0.1f);
    drawHeader(window, ww, headerH);

    unsigned titleSz = static_cast<unsigned>(std::clamp(headerH * 0.38f, 18.f, 32.f));
    sf::Text hdrTxt("RENDER COMPLETE", _font, titleSz);
    hdrTxt.setStyle(sf::Text::Bold);
    hdrTxt.setFillColor(COL_DONE);
    sf::FloatRect ht = hdrTxt.getLocalBounds();
    hdrTxt.setOrigin(ht.left + ht.width / 2.f, ht.top + ht.height / 2.f);
    hdrTxt.setPosition(cx, headerH / 2.f);
    window.draw(hdrTxt);

    // Center message
    unsigned msgSz = static_cast<unsigned>(std::clamp(wh * 0.036f, 20.f, 36.f));
    sf::Text msg("Render finished successfully.", _font, msgSz);
    msg.setFillColor(COL_TEXT);
    sf::FloatRect mb = msg.getLocalBounds();
    msg.setOrigin(mb.left + mb.width / 2.f, mb.top + mb.height / 2.f);
    msg.setPosition(cx, cy - wh * 0.06f);
    window.draw(msg);

    unsigned pathSz = static_cast<unsigned>(std::clamp(wh * 0.02f, 13.f, 18.f));
    sf::Text pathTxt(_scenePath, _font, pathSz);
    pathTxt.setFillColor(COL_SUBTEXT);
    sf::FloatRect pb = pathTxt.getLocalBounds();
    pathTxt.setOrigin(pb.left + pb.width / 2.f, pb.top);
    pathTxt.setPosition(cx, cy - wh * 0.06f + msgSz + std::max(10.f, wh * 0.012f));
    window.draw(pathTxt);

    // Back button
    float btnW = std::max(160.f, ww * 0.13f);
    float btnH = std::max(42.f, wh * 0.057f);
    _backRect = {cx - btnW / 2.f, wh * 0.74f, btnW, btnH};
    drawButton(window, _backRect, "Back to Browser", COL_BACK);
}

void RenderPanel::drawButton(sf::RenderWindow& window, sf::FloatRect rect,
                             const std::string& label, sf::Color bg) {
    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition(rect.left, rect.top);
    shape.setFillColor(bg);
    window.draw(shape);

    unsigned sz = static_cast<unsigned>(std::clamp(rect.height * 0.42f, 14.f, 20.f));
    sf::Text text(label, _font, sz);
    text.setFillColor(COL_TEXT);
    sf::FloatRect tb = text.getLocalBounds();
    text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    text.setPosition(rect.left + rect.width / 2.f, rect.top + rect.height / 2.f);
    window.draw(text);
}

bool RenderPanel::wasClicked(const sf::Event& event, const sf::RenderWindow& window,
                             sf::FloatRect rect) const {
    if (event.type != sf::Event::MouseButtonPressed) return false;
    sf::Vector2f pos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
    return rect.contains(pos);
}
