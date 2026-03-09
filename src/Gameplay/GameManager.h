#pragma once
#include "Pch.h"
#include <random>
#include <ECS_Engine.h>
#include "Rail.h" 
#include "Replay.h"

class GameManager
{
public:
    static GameManager* Getinstance();
    void Init();
    void Update(const GameTimer& gt);

    std::vector<Entity*> GetCameras() { return cameras; }
	Entity* GetSushi() { return Sushi; }

    Entity* SushiRightArm = nullptr;
    Entity* SushiLeftArm = nullptr;
private:
    static GameManager* m_instance;

    ECS* ecs = nullptr;

    float theta = 1.5f * DirectX::XM_PI;
    float phi = DirectX::XM_PIDIV4;
    float radius = 5.0f;
    float m_speed = 5.0f;
    float m_jumpforce = 500.0f;
    float m_timerFps = 0.0f;

    float m_replayTheta = 0.f;
    float m_replayPhi = 1.2f;
    float m_replayRadius = 8.f;

    bool isRotating = false;
    bool switchCamera = true;

    int PlayerId = -1;
    Entity* four = nullptr;
    Entity* Sol = nullptr;

	Entity* Sushi = nullptr;

    Entity* textFps = nullptr;

    Entity* cam1 = nullptr;
    Entity* cam2 = nullptr;
    std::vector<Entity*> cameras;

    Rail* m_rail = nullptr;

    Replay m_replay;
    bool m_playerDead = false;


    void InitEntities();
    void InitCameras();
    void InitLights();
    void InitUis();

};
