#include "CameraManager.h"
#include "RenderCamera.h"

void CameraManager::SetRenderCameras(const std::vector<RenderCamera*>& cameras)
{
    m_activeCameras = cameras;
}

void CameraManager::Clear()
{
    m_activeCameras.clear();
}