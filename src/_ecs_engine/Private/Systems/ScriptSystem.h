#pragma once
#include "System.h"
#include <vector>

class ScriptComponent;
class GameTimer;

class ScriptSystem : public System
{
public:
    virtual void AddComponent(Component* _comp) override;

    virtual void Update() override;

private:
    std::vector<ScriptComponent*> m_scripts;
};