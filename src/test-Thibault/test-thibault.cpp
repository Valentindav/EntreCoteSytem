#include "_engine.h"
#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{

    ECS_CONSOLE();

    ECS_RUN(App, hInstance, 1280, 720);

}