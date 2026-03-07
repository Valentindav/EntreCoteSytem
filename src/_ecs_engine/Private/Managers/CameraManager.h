#pragma once

#include <vector>

class RenderCamera;

class CameraManager
{
public:
    CameraManager() = default;
    ~CameraManager() = default;

    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    void SetRenderCameras(const std::vector<RenderCamera*>& cameras);

    void Clear();

    const std::vector<RenderCamera*>& GetRenderCameras() const { return m_activeCameras; }
    size_t GetCameraCount() const { return m_activeCameras.size(); }
    bool IsEmpty() const { return m_activeCameras.empty(); }

private:
    std::vector<RenderCamera*> m_activeCameras;
};