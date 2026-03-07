#pragma once
#include "System.h"
#include <vector>

class CameraComponent;
class RenderCamera;

class CameraSystem : public System
{
private:
    std::vector<CameraComponent*> m_cameras;
    CameraComponent* m_currentCamera = nullptr;

    float m_lastAspectRatio = 0.0f;
    int m_lastWidth = 0;
    int m_lastHeight = 0;

    void AddComponent(Component* _comp) override;
    virtual void Update() override;
    void UpdateCamera(CameraComponent* _cam, float winW, float winH);

public:
    CameraSystem() = default;
    friend class ECS;
};