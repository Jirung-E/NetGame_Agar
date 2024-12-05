#pragma once

#include "Scene.h"

#include "../Widgets/Button.h"
#include "../Widgets/EditBox.h"


class MainScene : public Scene {
private:
    Button start_button;
    mutable EditBox edit_box;

public:
    MainScene();

protected:
    void draw(const HDC& hdc) const;

public:
    ButtonID clickL(const POINT& point) const;
    ButtonID clickR(const POINT& point) const;

    bool keyboardInput(int keycode);
};