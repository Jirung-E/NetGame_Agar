#include "TextBox.h"


TextBox::TextBox(const tstring& text, const Point& position, double width, double height) : text { text },
position { position }, width { width }, height { height }, transparent_background { false }, transparent_border { false },
square { true }, bold { 0 }, italic { false },
background_color { White }, text_color { Black }, border_color { White }, border_width { 1 }, align { DT_CENTER } {

}


void TextBox::show(const HDC& hdc, const RECT& valid_area) const {
    drawBase(hdc, valid_area);
    drawText(hdc, valid_area);
}

RECT TextBox::absoluteArea(const RECT& valid_area) const {
    RECT area = valid_area;
    if(square) {
        area = rectToSquare(valid_area);
    }
    else {
        area = valid_area;
    }
    int w = area.right - area.left;
    int h = area.bottom - area.top;
    int x = area.left + w * position.x / 100;
    int y = area.top + h * position.y / 100;
    return RECT { x, y, int(x + w * width / 100), int(y + h * height / 100) };
}


void TextBox::drawBase(const HDC& hdc, const RECT& valid_area) const {
    HBRUSH bg_br;
    if(transparent_background) {
        bg_br = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    }
    else {
        bg_br = CreateSolidBrush(background_color);
    }
    int style = transparent_border ? PS_NULL : PS_SOLID;
    HPEN bg_pen = CreatePen(style, border_width, border_color);
    HBRUSH old_br = (HBRUSH)SelectObject(hdc, bg_br);
    HPEN old = (HPEN)SelectObject(hdc, bg_pen);

    RECT rect = absoluteArea(valid_area);
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hdc, old_br);
    SelectObject(hdc, old);
    DeleteObject(bg_pen);
    DeleteObject(bg_br);
}

void TextBox::drawText(const HDC& hdc, const RECT& valid_area) const {
    LOGFONT logfont;
    GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logfont);

    RECT rect = absoluteArea(valid_area);
    rect %= 90;
    logfont.lfHeight = (rect.bottom - rect.top) * 90 / 100;
    switch(bold) {
    case 1:
        logfont.lfWeight = FW_BOLD;
        break;
    case 2:
        logfont.lfWeight = FW_EXTRABOLD;
        break;
    case 3:
        logfont.lfWeight = FW_ULTRABOLD;
        break;
    case 4:
        logfont.lfWeight = FW_HEAVY;
        break;
    }
    logfont.lfItalic = italic;

    HFONT hNewFont = CreateFontIndirect(&logfont);
    HFONT old_font = (HFONT)SelectObject(hdc, hNewFont);

    int center_x = (rect.left + rect.right) / 2;
    int center_y = (rect.top + rect.bottom) / 2;
    SIZE size;
    GetTextExtentPoint32W(hdc, text.c_str(), text.length(), &size);

    COLORREF old_text_color = SetTextColor(hdc, text_color);
    COLORREF old_bk_color = SetBkColor(hdc, background_color);
    if(transparent_background) {
        SetBkMode(hdc, TRANSPARENT);
    }
    DrawText(hdc, text.c_str(), text.length(), &rect, align);

    SetTextColor(hdc, old_text_color);
    SetBkColor(hdc, old_bk_color);
    SelectObject(hdc, old_font);
    DeleteObject(hNewFont);
}