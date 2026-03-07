#pragma once
#include "Pch.h"
#include <random>
#include <DirectXMath.h>
#include "EngineApp.h"
#include "GameTimer.h"
#include "ParticleEmitter.h"
class App {
public:
    App();
    ~App();

    void Init();
    void Update(const GameTimer& gt);

    void CheckInput(const GameTimer& gt);

    void CreateUI();

    void CreateParticles();

    void CreateCamera();

    void CreateEnt();

    float theta = 1.5f * DirectX::XM_PI;
    float phi = DirectX::XM_PIDIV4;
    float radius = 5.0f;
    float m_speed = 5.0f;

    bool isRotating = false;

private:

    ParticleEmitter* m_fire;
    ParticleEmitter* m_water;


    int m_smokeEmitterId;
    int m_rainEmitterId;
    Entity* camera1 = nullptr;
    Entity* camera2 = nullptr;
    Entity* CurrentCamera = nullptr;

    Entity* entity2 = nullptr;
    Entity* entity2 = nullptr;
    Entity* ground = nullptr;
};