#pragma once
#include <ECS_Engine.h>
#include "GameManager.h"
#include "GameTimer.h"
#include "PlatScript.h"

START_SCRIPT(CommandeScript)

private:
    bool commandFinished = false;

    // Définition d'une recette
    struct Recette {
        std::string nom;
        bool needBuns;
        bool needSalad;
        bool needFish;
    };

    Recette recetteCourante;

    std::vector<Recette> listeRecettes = {
        {"Burger Complet",  true,  true,  true},
        {"Fish Sandwich",   true,  false, true},
        {"Veggie Bun",      true,  true,  false},
        {"Fish Solo",       false, false, true},
    };

public:
    void OnStart()
    {
        int index = rand() % listeRecettes.size();
        recetteCourante = listeRecettes[index];

        std::cout << "[Commande] Recette : " << recetteCourante.nom << std::endl;
        std::cout << "  Buns  : " << recetteCourante.needBuns << std::endl;
        std::cout << "  Salad : " << recetteCourante.needSalad << std::endl;
        std::cout << "  Fish  : " << recetteCourante.needFish << std::endl;
    }

    void OnUpdate()
    {
        if (commandFinished)
        {
            std::cout << "[Commande] Validée ! Destruction de l'entité." << std::endl;
            ECS_ECS->DestroyEntity(owner);
        }
    }

    void OnCollisionEnter(Entity* other)
    {
        if(ECS_ECS->GetComponent<ScriptComponent>(other->GetId())&& dynamic_cast<PlatScript*>(ECS_ECS->GetComponent<ScriptComponent>(other->GetId())->m_instance))
        {
            PlatScript* plat = dynamic_cast<PlatScript*>(ECS_ECS->GetComponent<ScriptComponent>(other->GetId())->m_instance);
            if (plat == nullptr) return;

            bool platValide = plat->CheckIngredients(
                recetteCourante.needBuns,
                recetteCourante.needSalad,
                recetteCourante.needFish
            );

            if (platValide)
            {
                commandFinished = true;
            }
            else
            {
                std::cout << "[Commande] Plat incomplet !" << std::endl;
            }
        }
    }

    void OnDestroy() {}

    std::string GetNomRecette() { return recetteCourante.nom; }

    END_SCRIPT(CommandeScript)