#pragma once

#include "Scene.h"

#include "../Widgets/Button.h"


class MainScene : public Scene {
private:
    Button start_button;

public:
    MainScene();

protected:
    void draw(const HDC& hdc) const;

public:
    ButtonID clickL(const POINT& point) const;
    ButtonID clickR(const POINT& point) const;
};