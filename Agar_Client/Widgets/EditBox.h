#pragma once

#include "TextBox.h"

#include <regex>


class EditBox : public TextBox {
public:
    bool is_focused;
    COLORREF focused_background_color;
    COLORREF focused_border_color;
    int focused_border_width;

    COLORREF error_background_color;
    COLORREF error_border_color;

    std::wregex format;

    tstring hint;

private:
    using TextBox::text;

    bool is_error;

public:
    EditBox(const tstring& hint = L"", const Point& position = { 0, 0 }, double width = 1, double height = 1);

protected:
    virtual void drawBase(const HDC& hdc, const RECT& valid_area) const override;
    virtual void drawText(const HDC& hdc, const RECT& valid_area) const override;

public:
    void add(TCHAR c);
    void backspace();
    tstring getText() const;
    bool isValid() const;

    void validate();
};
