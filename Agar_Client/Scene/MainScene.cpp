#include "MainScene.h"

#include "../Util.h"


MainScene::MainScene(): 
    Scene { Main }, 
    start_button { L"Start", { 25, 40 }, 50, 20 },
    edit_box { L"IPADDR:PORT", { 0, 70 }, 100, 10 }
{
    start_button.border_color = Red;
    start_button.border_width = 3;
    start_button.id = StartButton;

    edit_box.border_color = Black;
    edit_box.border_width = 2;
    edit_box.focused_border_color = Blue;
    edit_box.focused_border_width = 4;
    edit_box.text_mag = 80;
    edit_box.format.assign(L"^$|\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}");
}


void MainScene::draw(const HDC& hdc) const {
    drawBackground(hdc, White);

    edit_box.show(hdc, valid_area);

    // Draw Start Button
    start_button.show(hdc, valid_area);

    // Draw Customize Button


    // Draw Quit Button
    
}


ButtonID MainScene::clickL(const POINT& point) const {
    edit_box.is_focused = false;

    RECT r = start_button.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        return start_button.id;
    }

    r = edit_box.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        edit_box.is_focused = true;
        return None;
    }

    return None;
}

ButtonID MainScene::clickR(const POINT& point) const {
    edit_box.is_focused = false;

    RECT r = edit_box.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        edit_box.is_focused = true;
        return None;
    }

    return None;
}


bool MainScene::keyboardInput(int keycode) {
    if(edit_box.is_focused) {
        switch(keycode) {
            case VK_BACK:
                edit_box.backspace();
                return true;
            case VK_RETURN:
                edit_box.is_focused = false;
                return true;
            case VK_ESCAPE:
                edit_box.is_focused = false;
                return true;
            default:
                edit_box.add(keycode);
                return true;
        }
    }

    return false;
}


bool MainScene::isValidAddress() const {
    return edit_box.isValid();
}

tstring MainScene::getAddress() const {
    return edit_box.getText();
}
