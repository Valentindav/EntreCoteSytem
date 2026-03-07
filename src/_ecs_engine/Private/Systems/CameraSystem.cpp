#include "CameraSystem.h"

#include "Public/ECS_Components/CameraComponent.h"
#include "Public/Entity.h"
#include "Private/Managers/CameraManager.h"
#include "Private/_engine.h"
#include "Private/Window.h"

#include <RenderCamera.h>
#include <algorithm>

using namespace DirectX;

void CameraSystem::AddComponent(Component* _comp)
{
    m_cameras.push_back(static_cast<CameraComponent*>(_comp));
}

void CameraSystem::Update()
{
    if (m_cameras.empty()) return;

    std::vector<CameraComponent*> activeCameras;
    for (auto& camComp : m_cameras) {
        if (camComp->m_isActive) {
            activeCameras.push_back(camComp);
        }
    }

    std::sort(activeCameras.begin(), activeCameras.end(), [](CameraComponent* a, CameraComponent* b) {
        return a->m_renderOrder < b->m_renderOrder;
        });

    float w = ECS_ENGINE->GetClientWidth();
    float h = ECS_ENGINE->GetClientHeight();

    std::vector<RenderCamera*> renderCameras;

    for (CameraComponent* cam : activeCameras) {
        UpdateCamera(cam, w, h);
        renderCameras.push_back(cam->m_renderCamera);
    }

    ECS_ENGINE->GetCameraManager()->SetRenderCameras(renderCameras);

    m_cameras.clear();
}

void CameraSystem::UpdateCamera(CameraComponent* _cam, float winW, float winH)
{
    RenderCamera* rc = _cam->m_renderCamera;

    float vx = _cam->m_viewX * winW;
    float vy = _cam->m_viewY * winH;
    float vw = _cam->m_viewWidth * winW;
    float vh = _cam->m_viewHeight * winH;
    float aspect = vw / vh;

    rc->UpdateProjection(0.25f * 3.14159f, aspect, 1.0f, 1000.0f);
    rc->SetViewport(vw, vh, vx, vy);
    rc->SetScissorRect((int)vx, (int)vy, (int)(vx + vw), (int)(vy + vh));

    TransformComponent& trans = _cam->GetOwner()->transform;
    XMMATRIX worldMatrix = XMLoadFloat4x4(&trans.GetWorldMatrix());
    XMVECTOR det = XMMatrixDeterminant(worldMatrix);
    XMMATRIX viewMatrix = XMMatrixInverse(&det, worldMatrix);

    rc->SetViewMatrix(viewMatrix);
    rc->SetPosition(trans.GetWorldPosition());
}