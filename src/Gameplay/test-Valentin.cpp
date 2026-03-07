#include <ECS_Engine.h>
#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{

    //------------------------base game------------------------------
    ECS_CONSOLE; // create console

    ECS_RUN(App, hInstance, 1920, 1080);

    //------------------------Other test-------------------------------
   // RunTest_StateMachine();// test of the state machine in console
}