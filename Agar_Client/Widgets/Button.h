#pragma once

#include "TextBox.h"


enum ButtonID {
    None, StartButton, ResumeGame, QuitGame, 
};


class Button : public TextBox {
public:
    ButtonID id;

public:
    Button(const tstring& text = L"", const Point& position = { 0, 0 }, double width = 1, double height = 1);
};