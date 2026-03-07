#pragma once
#include "Public/Application.h"
#include <Renderer.h>
#include <memory>
#include <Windows.h>
#include <atomic>
#include <mutex>

class ECS;
class Window;
class ResourceManager;
class BatchManager;
class CameraManager;
class LightManager;
class UiManager;
class AnimationManager;
class ParticleManager;
class MenuManager;
class Entity;

#define ECS_ENGINE (EngineCore::GetInternalInstance())

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

    ECS* GetECS() const override { return m_ecs.get(); }
    GameTimer& GetTimer() override { return m_timer; }

    std::unique_ptr<Renderer>& GetRenderer() { return m_renderer; }
    ResourceManager* GetResourceManager() const { return m_resourceManager.get(); }
    BatchManager* GetBatchManager() const { return m_batchManager.get(); }
    CameraManager* GetCameraManager() const { return m_cameraManager.get(); }
    LightManager* GetLightManager() const { return m_lightManager.get(); }
    UiManager* GetUiManager() const { return m_uiManager.get(); }
    AnimationManager* GetAnimationManager() const { return m_animationManager.get(); }
    ParticleManager* GetParticleManager() const { return m_particleManager.get(); }
    MenuManager* GetMenuManager() const { return m_menuManager.get(); }

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
    std::unique_ptr<Window> m_window = nullptr;
    std::unique_ptr<ECS> m_ecs = nullptr;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<BatchManager> m_batchManager;
    std::unique_ptr<CameraManager> m_cameraManager;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<UiManager> m_uiManager;
    std::unique_ptr<AnimationManager> m_animationManager;
    std::unique_ptr<ParticleManager> m_particleManager;
    std::unique_ptr<MenuManager> m_menuManager;

    uint8_t m_filterShaders = 0;

    Entity* m_mainCam = nullptr;

    void DrawFrame();
};