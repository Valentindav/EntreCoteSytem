#pragma once

#include "Public/ECS_Components.h"
#include "Public/ECS_GameObjects.h"
#include "Public/Inputs.h"
#include "Public/Script.h"
#include "Public/ECS.h"
#include "Public/Application.h"
#include "Public/Utils.h"

#include <Windows.h>
#include <GameTimer.h>
#include <iostream>
#include <ctime>

// --- MACROS PUBLIQUES ---
#define ECS_RUN(UserAppClass, hInstance, width, height) Run<UserAppClass>(hInstance, width, height)
#define ECS_CONSOLE                     CreateConsole()
#define ECS_APP                         (Application::GetInstance())
#define ECS_ECS                         (Application::GetInstance()->GetECS())

// --- CONSOLE HELPER ---
inline void CreateConsole() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    std::cout.clear(); std::cerr.clear(); std::cin.clear();
}

// --- RUN TEMPLATE ---
template <typename ClientApp>
void Run(HINSTANCE hInstance, int width, int height, bool fullscreen = false)
{
    srand(static_cast<unsigned int>(time(0)));

    Application* pEngine = CreateEngineCore();
    ClientApp* pApp = new ClientApp();

    pEngine->OnInit = [pApp]() { pApp->Init(); };
    pEngine->OnUpdate = [pApp](const GameTimer& gt) { pApp->Update(gt); };
    pEngine->OnRender = [pApp](const GameTimer& gt) {};

    pEngine->Init(hInstance, width, height, fullscreen);

    pEngine->Run();

    delete pApp;
    delete pEngine;
}