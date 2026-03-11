#pragma once
#include <ECS_Engine.h>
#include "GameManager.h"
#include "GameTimer.h"
#include "CommandeScript.h"

START_SCRIPT(PnjScript)
private:
public:
    void OnStart() {}

    void OnUpdate()
    {
        owner->transform.WorldTranslate(
            owner->transform.GetWorldForward() * ECS_ENGINE->GetTimer().DeltaTime()
        );
    }

    void OnCollisionEnter(Entity* other)
    {
        if (owner && other->GetName() == "CommandeCollector")
        {
            ECS_ECS->DestroyEntity(owner);
        }
    }

    void OnDestroy()
    {
        XMFLOAT3 pos = owner->transform.GetWorldPosition();

        Entity* commandeEntity = ECS_ECS->CreateEntity();
        commandeEntity->transform.SetWorldPosition({ pos.x + 2.f, pos.y, pos.z });

        ScriptComponent* sc = ECS_ECS->AddComponents<ScriptComponent>(commandeEntity);
        sc->SetScript<CommandeScript>();

        MeshComponent* mesh = ECS_ECS->AddComponents<MeshComponent>(commandeEntity);
        mesh->LoadMesh("cube");
        commandeEntity->transform.SetWorldScale({ 0.4f, 0.4f, 0.4f });

        MaterialComponent* mat = ECS_ECS->AddComponents<MaterialComponent>(commandeEntity);
        mat->SetColor({ 1.f, 0.8f, 0.f, 1.f }); // jaune = commande en attente

        std::cout << "[PnjScript] Commande créée à la position du PNJ." << std::endl;
    }

    END_SCRIPT(PnjScript)