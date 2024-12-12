#include "MainScene.h"

#include "../Util.h"


MainScene::MainScene(): 
    Scene { Main }, 
    start_button { L"Start", { 25, 40 }, 50, 20 },
    addr_edit { L"IPADDR:PORT", { 0, 80 }, 100, 10 },
    name_edit { L"Nickname", { 0, 65 }, 100, 10 }
{
    start_button.border_color = Red;
    start_button.border_width = 3;
    start_button.id = StartButton;

    addr_edit.border_color = Black;
    addr_edit.border_width = 2;
    addr_edit.focused_border_color = Blue;
    addr_edit.focused_border_width = 4;
    addr_edit.text_mag = 80;
    addr_edit.format.assign(L"^$|\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}");

    name_edit.border_color = Black;
    name_edit.border_width = 2;
    name_edit.focused_border_color = Blue;
    name_edit.focused_border_width = 4;
    name_edit.text_mag = 80;
    name_edit.format.assign(L".{2,16}");
    name_edit.validate();
}


void MainScene::draw(const HDC& hdc) const {
    drawBackground(hdc, White);

    addr_edit.show(hdc, valid_area);

    name_edit.show(hdc, valid_area);

    // Draw Start Button
    start_button.show(hdc, valid_area);

    // Draw Customize Button


    // Draw Quit Button
    
}


ButtonID MainScene::clickL(const POINT& point) const {
    addr_edit.is_focused = false;
    name_edit.is_focused = false;

    RECT r = start_button.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        return start_button.id;
    }

    r = addr_edit.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        addr_edit.is_focused = true;
        return None;
    }

    r = name_edit.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        name_edit.is_focused = true;
        return None;
    }

    return None;
}

ButtonID MainScene::clickR(const POINT& point) const {
    addr_edit.is_focused = false;
    name_edit.is_focused = false;

    RECT r = addr_edit.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        addr_edit.is_focused = true;
        return None;
    }

    r = name_edit.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        name_edit.is_focused = true;
        return None;
    }

    return None;
}


bool MainScene::keyboardInput(int keycode) {
    if(addr_edit.is_focused) {
        switch(keycode) {
            case VK_BACK:
                addr_edit.backspace();
                return true;
            case VK_RETURN:
                addr_edit.is_focused = false;
                return true;
            case VK_ESCAPE:
                addr_edit.is_focused = false;
                return true;
            default:
                addr_edit.add(keycode);
                return true;
        }
    }
    if(name_edit.is_focused) {
        switch(keycode) {
            case VK_BACK:
                name_edit.backspace();
                return true;
            case VK_RETURN:
                name_edit.is_focused = false;
                return true;
            case VK_ESCAPE:
                name_edit.is_focused = false;
                return true;
            default:
                name_edit.add(keycode);
                return true;
        }
    }

    return false;
}


bool MainScene::isValid() const {
    return addr_edit.isValid() && name_edit.isValid();
}

std::string MainScene::getAddress() const {
    tstring tstr = addr_edit.getText();
    return { tstr.begin(), tstr.end() };
}

std::string MainScene::getName() const {
    tstring tstr = name_edit.getText();
    return { tstr.begin(), tstr.end() };
}
