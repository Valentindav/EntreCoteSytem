#include "Pch.h"
#include "App.h"

#include <iostream>
#include <Windows.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include "StateMachine.h"
#include "Entity.h"

int counter = 0;
bool firstA = true;

void Enter() {
    counter = 0; firstA = false; std::cout << "[StateA] onEnter\n";
}
void update() {
    std::cout << "[StateA] onUpdate (" << counter << ")\n"; ++counter;
}
void Exit() {
    std::cout << "[StateA] onExit\n";
}
bool Transition() { return counter > 3 && firstA; }

struct StateA {

    Entity* owner = nullptr;
    std::function<void()> onEnter;
    std::function<void()> onUpdate;
    std::function<void()> onExit;
    std::function<bool()> Transition;

    StateA() :
        onEnter([this]() { ::Enter(); }),
        onUpdate([this]() { ::update(); }),
        onExit([this]() { ::Exit(); }),
        Transition([this]() { return ::Transition(); }) {}
};

struct StateB {

    Entity* owner = nullptr;
    void onEnter() { counter = 0; std::cout << "[StateB] onEnter\n"; }
    void onUpdate() { std::cout << "[StateB] onUpdate (" << counter << ")\n"; ++counter; }
    void onExit() { std::cout << "[StateB] onExit\n"; }
    bool Transition() { return counter > 6; }
};

int RunTest_StateMachine()
{
    Entity* owner = new Entity();
    ECS_CONSOLE;

    State* StateC = new State();
    StateC->name = "carrote";
    StateC->onEnter = []() { counter = 0; std::cout << "[StateC] onEnter\n"; };
    StateC->onUpdate = []() { std::cout << "[StateC] onUpdate (" << counter << ")\n";  ++counter;  };
    StateC->onExit = []() { std::cout << "[StateC] onExit\n"; };
    StateC->Transition = []() { return counter >= 1; };

    StateMachine<Entity> sm(owner);
    sm.AddState<StateA>();
    sm.AddState<StateB>();
    sm.AddState(StateC);

    sm.ToState(typeid(StateB).name());

    for (int i = 0; i < 100; ++i) {
        sm.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "=== Test terminé ===\n";
    return 0;

}