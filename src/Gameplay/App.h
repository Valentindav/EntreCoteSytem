#pragma once
#include <ECS_Engine.h>

class GameManager;
class App {
public:
    App();
    ~App();

    void Init();
    void Update(const GameTimer& gt);

    GameManager* game = nullptr;
};