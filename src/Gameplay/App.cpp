#include "App.h"
#include "GameManager.h"

using namespace DirectX;

App::App() {}
App::~App() {}

void App::Init()
{
    game = GameManager::Getinstance();

    game->Init();
}

void App::Update(const GameTimer& gt) {
    game->Update(gt);
}