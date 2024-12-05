#include "EditBox.h"


EditBox::EditBox(const tstring& hint, const Point& position, double width, double height):
    TextBox { L"", position, width, height },
    is_focused { false },
    focused_background_color { White },
    focused_border_color { Black },
    focused_border_width { 1 },
    error_background_color { Red | White },
    error_border_color { Red },
    format { L".*" },
    hint { hint },
    is_error { false }
{

}


void EditBox::drawBase(const HDC& hdc, const RECT& valid_area) const {
    int border_width = this->border_width;
    COLORREF border_color = this->border_color;
    COLORREF background_color = this->background_color;
    if(is_focused) {
        border_width = focused_border_width;
        border_color = focused_border_color;
        background_color = focused_background_color;
    }
    if(is_error) {
        border_color = error_border_color;
        background_color = error_background_color;
    }

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

void EditBox::drawText(const HDC& hdc, const RECT& valid_area) const {
    COLORREF text_color = this->text_color;
    COLORREF background_color = this->background_color;
    tstring text = this->text;
    if(is_focused) {
        background_color = focused_background_color;
    }
    else if(is_error) {
        background_color = error_background_color;
    }
    if(text.empty() && !hint.empty()) {
        text = hint;
        text_color = RGB(128, 128, 128);
    }

    LOGFONT logfont;
    GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logfont);

    RECT rect = absoluteArea(valid_area);
    rect %= 90;
    logfont.lfHeight = (rect.bottom - rect.top) * 90 / 100;
    logfont.lfHeight = logfont.lfHeight * text_mag / 100;
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
    rect.top = center_y - logfont.lfHeight / 2;
    rect.bottom = center_y + logfont.lfHeight / 2;

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


void EditBox::add(TCHAR c) {
    text += c;
    validate();
}

void EditBox::backspace() {
    if(!text.empty()) {
        text.pop_back();
        validate();
    }
}

tstring EditBox::getText() const {
    return text;
}

void EditBox::validate() {
    std::wsmatch m;
    is_error = !std::regex_match(text, m, format);
}
