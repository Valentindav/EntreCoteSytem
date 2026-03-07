#pragma once
#include "GameTimer.h"
#include <functional>
#include <Windows.h>

class Window;
class ECS;

class Application
{
public:

    static Application* GetInstance();

    virtual ~Application() = default;

    virtual void Init(HINSTANCE hInstance, int width, int height, bool fullscreen = false) = 0;
    virtual int Run() = 0;
    virtual void OnResize() = 0;

    std::function<void()> OnInit;
    std::function<void(const GameTimer&)> OnUpdate;
    std::function<void(const GameTimer&)> OnRender;

    virtual int GetClientWidth() const { return m_clientWidth; }
    virtual int GetClientHeight() const { return m_clientHeight; }
    virtual ECS* GetECS() const = 0;
    virtual GameTimer& GetTimer() = 0;

protected:

    int m_clientWidth;
    int m_clientHeight;

    static Application* s_instance;
};

Application* CreateEngineCore();