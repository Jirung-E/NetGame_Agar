#pragma once

#include <Windows.h>

#include "../Widgets/Button.h"
#include "../Widgets/TextBox.h"


enum SceneID {
    Main, Game, PlayerCustomize, Game_Setting
};


class Scene {
protected:
    SceneID id;
    int horizontal_length;
    int vertical_length;
    RECT valid_area;

public:
    Scene(const SceneID& id);

protected:
    void drawBackground(const HDC& hdc, const COLORREF& color) const;
    virtual void draw(const HDC& hdc) const = 0;

public:
    virtual void show(const HDC& hdc) const;
    void syncSize(const HWND& hWnd);
    SceneID getID() const;
    virtual ButtonID clickL(const POINT& point) const;
    virtual ButtonID clickR(const POINT& point) const;
};