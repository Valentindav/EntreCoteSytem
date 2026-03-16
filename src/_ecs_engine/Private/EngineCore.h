#pragma once

#include "Public/Application.h"
#include "Private/Managers.h"
#include "Private/Window.h"

#include "../Public/ECS.h"

#include <Renderer.h>
#include <memory>
#include <Windows.h>
#include <atomic>
#include <mutex>

#define ECS_ENGINE (EngineCore::GetInternalInstance())

class Entity;

class EngineCore : public Application
{
public:
    EngineCore();
    ~EngineCore() override;

    static EngineCore* GetInternalInstance();

    void Init(HINSTANCE hInstance, int width, int height, bool fullscreen = false) override;
    int Run() override;
    void OnResize() override;

    void UpdateSplashScreen(const GameTimer& gt);

    GameTimer& GetTimer() override { return m_timer; }

    ECS* GetECS() const override { return &m_ecs; }

    Renderer* GetRenderer() const { return &m_renderer; }
    ResourceManager* GetResourceManager() const { return &m_resourceManager; }
    BatchManager* GetBatchManager() const { return &m_batchManager; }
    CameraManager* GetCameraManager() const { return &m_cameraManager; }
    LightManager* GetLightManager() const { return &m_lightManager; }
    UiManager* GetUiManager() const { return &m_uiManager; }
    AnimationManager* GetAnimationManager() const { return &m_animationManager; }
    ParticleManager* GetParticleManager() const { return &m_particleManager; }
    MenuManager* GetMenuManager() const { return &m_menuManager; }

    void DrawSolidShader(bool _enable);
    void DrawSolidShader();
    void DrawWireframeShader(bool _enable);
    void DrawWireframeShader();
    void DrawPostProcessShader(bool _enable);
    void DrawPostProcessShader();

    std::atomic<bool> m_loadingFinished{ false };
    std::mutex m_graphicMutex;

private:
    GameTimer m_timer;

    mutable Window m_window;
    mutable ECS m_ecs;

    mutable Renderer m_renderer;
    mutable ResourceManager m_resourceManager;
    mutable BatchManager m_batchManager;
    mutable CameraManager m_cameraManager;
    mutable LightManager m_lightManager;
    mutable UiManager m_uiManager;
    mutable AnimationManager m_animationManager;
    mutable ParticleManager m_particleManager;
    mutable MenuManager m_menuManager;

    uint8_t m_filterShaders = 0;

    Entity* m_mainCam = nullptr;

    void DrawFrame();
};