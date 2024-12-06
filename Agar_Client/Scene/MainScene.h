#pragma once

#include "Scene.h"

#include "../Widgets/Button.h"
#include "../Widgets/EditBox.h"


class MainScene : public Scene {
private:
    Button start_button;
    mutable EditBox addr_edit;
    mutable EditBox name_edit;

public:
    MainScene();

protected:
    void draw(const HDC& hdc) const;

public:
    ButtonID clickL(const POINT& point) const;
    ButtonID clickR(const POINT& point) const;

    bool keyboardInput(int keycode);

    bool isValid() const;
    std::string getAddress() const;
    std::string getName() const;
};