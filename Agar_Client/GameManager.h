#pragma once

#include "Scene/GameScene.h"
#include "Scene/MainScene.h"


// 장면전환, 버튼이 눌렸는지 등 관리
class GameManager {
private:
    MainScene main_scene;
    GameScene game_scene;
    Scene* current_scene;
    POINT mouse_position;

    enum Timer {
        Display, UpdateGame, GenerateFeeds, GenerateEnemy, GenerateTrap
    };

public:
    GameManager();
    ~GameManager();

public:
    void show(const HDC& hdc);
    void syncSize(const HWND& hWnd);

    void keyboardInput(const HWND& hWnd, int keycode);

    void clickScene(const HWND& hWnd, const POINT& point, const Direction& dir);
    void update(const HWND& hWnd);

    void quit(const HWND& hWnd);

    void setTimers(const HWND& hWnd);
    void timer(const HWND& hWnd, int id);

    void interrupt();

private:
    void gameStart(const HWND& hWnd);
    void lockCursor(const HWND& hWnd);
    void fixCursor(const HWND& hWnd);
    void releaseCursor();

    ButtonID buttonClicked(const POINT& point);

    void syncMousePosition(const HWND& hWnd);
};