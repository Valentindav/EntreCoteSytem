#include <ECS_Engine.h>
#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{

#ifdef _DEBUG
    ECS_CONSOLE;
#endif // DEBUG

    ECS_RUN(App, hInstance, 1920, 1080, true);
}