#pragma once
#include <ECS_Engine.h>
#include "GameManager.h"
#include "GameTimer.h"

START_SCRIPT(PlatScript)

private:
    bool hasBuns = false;
    bool hasSalad = false;
    bool hasFish = false;

public:
    void OnStart() {}

    void OnUpdate() {}

    // Appelé par CommandeScript pour valider le plat
    bool CheckIngredients(bool needBuns, bool needSalad, bool needFish)
    {
        bool bOk = (!needBuns || hasBuns);
        bool sOk = (!needSalad || hasSalad);
        bool fOk = (!needFish || hasFish);

        std::cout << "[Plat] Buns: " << hasBuns << " (requis: " << needBuns << ")" << std::endl;
        std::cout << "[Plat] Salad: " << hasSalad << " (requis: " << needSalad << ")" << std::endl;
        std::cout << "[Plat] Fish: " << hasFish << " (requis: " << needFish << ")" << std::endl;

        return bOk && sOk && fOk;
    }

    // Appelés quand un ingrédient est déposé sur le plat
    void AddBuns() { hasBuns = true; std::cout << "[Plat] Buns ajouté." << std::endl; }
    void AddSalad() { hasSalad = true; std::cout << "[Plat] Salad ajoutée." << std::endl; }
    void AddFish() { hasFish = true; std::cout << "[Plat] Fish ajouté." << std::endl; }

    void OnDestroy() {}

    END_SCRIPT(PlatScript)
