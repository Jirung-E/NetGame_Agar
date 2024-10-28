#include "MainScene.h"

#include "../Util.h"


MainScene::MainScene() : Scene { Main }, start_button { L"Start", { 25, 40 }, 50, 20 } {
    start_button.border_color = Red;
    start_button.border_width = 3;
    start_button.id = StartButton;
}


void MainScene::draw(const HDC& hdc) const {
    drawBackground(hdc, White);

    // Draw Start Button
    start_button.show(hdc, valid_area);

    // Draw Customize Button


    // Draw Quit Button
    
}


ButtonID MainScene::clickL(const POINT& point) const {
    RECT r = start_button.absoluteArea(valid_area);
    if(PtInRect(&r, point)) {
        return start_button.id;
    }
    return None;
}

ButtonID MainScene::clickR(const POINT& point) const {
    return ButtonID();
}