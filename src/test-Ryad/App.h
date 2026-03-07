#pragma once
#include "Pch.h"

#include <ECS_Engine.h>

class App {
public:
    App();
    ~App();

    void Init();
    void Update(const GameTimer& gt);

private:

    bool isMoving = false;

    float theta = 1.5f * DirectX::XM_PI;
    float phi = DirectX::XM_PIDIV4;
    float radius = 5.0f;
    float m_speed = 1.0f;
    float m_jumpforce = 500.0f;
    float m_timerFps = 0.0f;

    bool isRotating = false;
    bool switchCamera = true;

    Entity* entity1 = nullptr;
    Entity* entity2 = nullptr;
    Entity* ground = nullptr;
	Entity* emitter1 = nullptr;

    Entity* textFps = nullptr;

    std::vector<Entity*> cameras;

	void InitEntities();
    void InitCameras();
    void InitLights();
	void InitUis();
};