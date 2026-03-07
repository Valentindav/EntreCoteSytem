#include "RenderSystem.h"

#include "Public/ECS_Components/MeshComponent.h"
#include "Public/ECS_Components/MaterialComponent.h"
#include "Public/ECS_Components/TransformComponent.h"
#include "Public/ECS_Components/AnimatorComponent.h"
#include "Public/Entity.h"
#include "Public/ECS.h"

#include "Private/EngineCore.h"
#include "Private/Managers/BatchManager.h"

using namespace DirectX;

void RenderSystem::AddComponent(Component* _comp)
{
    m_meshes.push_back(static_cast<MeshComponent*>(_comp));
}

void RenderSystem::Update()
{
    if (m_meshes.empty()) return;

    BatchManager* batchManager = ECS_ENGINE->GetBatchManager();
    if (!batchManager) return;

    for (MeshComponent* meshComp : m_meshes)
    {
        if (!meshComp->IsVisible()) continue;

        Entity* ent = meshComp->GetOwner();
        if (!ent) continue;

        ent->transform.UpdateMatrix();

        MaterialComponent* matComp = ECS_ENGINE->GetECS()->GetComponent<MaterialComponent>(ent->GetId());
        AnimatorComponent* animComp = ECS_ENGINE->GetECS()->GetComponent<AnimatorComponent>(ent->GetId());

        InstanceData data;

        XMFLOAT4 meshColor = meshComp->GetColor();
        XMFLOAT4 matColor = matComp ? matComp->GetColor() : XMFLOAT4(1.f, 1.f, 1.f, 1.f);

        XMFLOAT4 finalColor = {
            meshColor.x * matColor.x,
            meshColor.y * matColor.y,
            meshColor.z * matColor.z,
            meshColor.w * matColor.w
        };

        data.Color = finalColor;

        XMFLOAT4X4 worldPos = ent->transform.GetWorldMatrix();
        data.World0 = XMFLOAT4(worldPos._11, worldPos._12, worldPos._13, worldPos._14);
        data.World1 = XMFLOAT4(worldPos._21, worldPos._22, worldPos._23, worldPos._24);
        data.World2 = XMFLOAT4(worldPos._31, worldPos._32, worldPos._33, worldPos._34);
        data.World3 = XMFLOAT4(worldPos._41, worldPos._42, worldPos._43, worldPos._44);

        if (matComp) {
            data.TexIndex = matComp->m_textureHandle;
            data.MatIndex = matComp->m_materialHandle;
        }
        else {
            data.TexIndex = 0;
            data.MatIndex = 0;
        }

        if (animComp) {
            data.BoneOffset = animComp->CurrentBoneOffset;
        }
        else {
            data.BoneOffset = 0xFFFFFFFF;
        }

        batchManager->AddInstance(meshComp->GetMeshHandle(), data);
    }

    m_meshes.clear();
}