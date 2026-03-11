#pragma once
#include <ECS_Engine.h>
#include "GameManager.h"
#include "GameTimer.h"
#include "PlatScript.h"

START_SCRIPT(IngredientScript)

private:
    // --- Définir ici quel ingrédient c'est ---
 

    bool isPickedUp = false;
    bool isOnPlate = false;

public:
    bool isBuns = false;
    bool isSalad = false;
    bool isFish = false;
    void OnStart()
    {
        // Vérification : un seul bool doit être actif
        int count = (int)isBuns + (int)isSalad + (int)isFish;
        if (count != 1)
        {
            std::cout << "[Ingredient] ERREUR : exactement 1 bool doit être actif ! (count=" << count << ")" << std::endl;
        }
    }

    void OnUpdate() {}

    void OnCollisionEnter(Entity* other)
    {
        if (other->GetName() == "Sushi")
        {
            if (Inputs::IsMouseDown(Mouse::Button::LEFT))
            {
				other->AddChild(owner);
                owner->GetComponent<RigidBodyComponent>()->m_motionType = MotionType::Static;
				

            }else if (Inputs::IsMouseUp(Mouse::Button::LEFT)) {
                other->RemoveChild(owner);
                owner->GetComponent<RigidBodyComponent>()->m_motionType = MotionType::Dynamic;

            }
        }

        if (ECS_ECS->GetComponent<ScriptComponent>(other->GetId()) && dynamic_cast<PlatScript*>(ECS_ECS->GetComponent<ScriptComponent>(other->GetId())->m_instance))
        {


            if (isOnPlate) return; // déjà posé, on ignore

            PlatScript* plat = dynamic_cast<PlatScript*>(ECS_ECS->GetComponent<ScriptComponent>(other->GetId())->m_instance);
            if (plat == nullptr) return;

            // Communique avec le plat selon l'ingrédient actif
            if (isBuns)  plat->AddBuns();
            if (isSalad) plat->AddSalad();
            if (isFish)  plat->AddFish();

            // Se met en enfant du plat
            owner->SetParent( other);

            isOnPlate = true;

            std::cout << "[Ingredient] Posé sur le plat." << std::endl;
            
        }
    }

    void OnDestroy() {}

    // Setter pour pickup/drop si tu as un système de portage
    void SetPickedUp(bool value) { isPickedUp = value; }
    bool IsOnPlate() { return isOnPlate; }

    END_SCRIPT(IngredientScript)

