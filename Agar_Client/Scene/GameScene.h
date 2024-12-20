#pragma once

#include "Scene.h"
#include "../Game/Map.h"
#include "../Game/Cell.h"
#include "../Game/Player.h"

#include <list>
#include <ctime>
#include <mutex>
#include <unordered_map>

#include "../../protocol.h"


class GameScene : public Scene {
public:
    enum CameraMode {
        Fixed, Dynamic
    };
    bool show_score;

    std::string nickname;

private:
    Map map;

    uint8_t id;
    std::vector<Cell> objects;
    std::unordered_map<uint8_t, std::string> player_profiles;
    Player player;
    Point player_destination;
    mutable std::mutex objects_mutex;

    Button resume_button;
    Button quit_button;
    bool paused;
    TextBox game_over_message;
    Button restart_button;
    bool game_over;

    volatile CameraMode cam_mode;

    double play_time;
    clock_t start_time;
    clock_t end_time;

    bool press_spit;
    bool press_split;
    Point mouse_position;   // �Ⱦ���

    bool send_limit_flag;

    std::atomic_bool connected;

    //SOCKET sock;

public:
    GameScene();

public:
    void setUp();
    void connect(const std::string& addr);
    void disconnect();

    CS_ACTION_PACKET BuildActionPacket();
	void SendActionPacket();
	void SendExitPacket();
	void SendRespawnPacket();
    void SendLoginPacket();
    void RecvPacket();
    void ProcessPacket(PACKET_HEADER* packet);

    void update(const POINT& point);

    void togglePauseState();
    void pause();
    void resume();
    void restart();

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