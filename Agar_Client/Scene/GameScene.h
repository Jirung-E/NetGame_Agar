#pragma once

#include "Scene.h"
#include "../Game/Map.h"
#include "../Game/Cell.h"
#include "../Game/Player.h"

#include <list>
#include <ctime>
#include <mutex>
#include "../../protocol.h"


class GameScene : public Scene {
public:
    enum CameraMode {
        Fixed, Dynamic
    };
    bool show_score;

private:
    Map map;

    uint8_t id;
    std::list<Cell> objects;
    Player player;
    mutable std::mutex objects_mutex;

    Button resume_button;
    Button quit_button;
    bool paused;
    TextBox game_over_message;
    bool game_over;

    CameraMode cam_mode;

    double play_time;
    clock_t start_time;
    clock_t end_time;

    bool press_spit;
    bool press_split;
    Point mouse_position;

    std::atomic_bool connected;

    //SOCKET sock;

public:
    GameScene();

public:
    void setUp();
    void connect();
    void disconnect();

    CS_ACTION_PACKET BuildActionPacket();
	void SendActionPacket();
    void RecvPacket();
    void ProcessPacket(char* buf);
    void ProcessPacket(PACKET_HEADER* packet);

    void update(const POINT& point);

    void togglePauseState();
    void pause();
    void resume();

private:
    void updatePlayer(const POINT& point);

protected:
    void draw(const HDC& hdc) const;
    RECT getViewArea() const;
    void drawScore(const HDC& hdc) const;
    void drawPauseScene(const HDC& hdc) const;
    void drawGameOverScene(const HDC& hdc) const;

public:
    void setCameraMode(const CameraMode& mode);

    ButtonID clickL(const POINT& point);
    ButtonID clickR(const POINT& point);
    //void mouseMove(const POINT& point);
    virtual void mouseMove(const POINT& point);
};