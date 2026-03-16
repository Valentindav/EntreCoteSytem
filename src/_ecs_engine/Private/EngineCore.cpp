#include "EngineCore.h"

#include "Private/Systems.h"

#include "Public/Inputs.h"
#include "Public/ECS_GameObjects/Camera.h"
#include "Public/ECS_Components.h"

#include <GpuCommandDispatcher.h>
#include <RenderCamera.h>
#include <thread>
#include <chrono>

Application* Application::s_instance = nullptr;

Application* Application::GetInstance()
{
    return s_instance;
}

Application* CreateEngineCore() {
    return new EngineCore();
}

EngineCore* EngineCore::GetInternalInstance()
{
    return static_cast<EngineCore*>(s_instance);
}

EngineCore::EngineCore() { s_instance = this; }
EngineCore::~EngineCore() { s_instance = nullptr; }

void EngineCore::Init(HINSTANCE hInstance, int width, int height, bool fullscreen)
{
    if (!m_window.Initialize(hInstance, width, height, L"Engine App")) {
        MessageBox(0, L"Window Initialization Failed", L"Error", MB_OK);
        return;
    }

    m_clientWidth = width;
    m_clientHeight = height;

    m_window.OnResize = [this](int w, int h) { this->OnResize(); };

    m_renderer.Initialize(m_window.GetHwnd(), m_window.GetClientWidth(), m_window.GetClientHeight());

    m_resourceManager.Initialize(m_renderer.GetBufferConverter(), m_renderer.GetCommandDispatcher(), &m_graphicMutex);

    m_uiManager.Initialize(m_renderer.GetDevice());

    m_ecs.Init();

    Inputs::BindToWindow(&m_window);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
    }

    this->OnResize();

    m_menuManager.loadMenu("SplashScreen");
    m_menuManager.switchMenu("SplashScreen");

    m_loadingFinished = false;

    m_mainCam = Camera::Camera();

    std::thread loadingThread([this]() {
        if (this->OnInit) {
            this->OnInit();
            //std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        this->m_loadingFinished = true;
        });

    loadingThread.detach();
}

void EngineCore::UpdateSplashScreen(const GameTimer& gt)
{
    float winW = static_cast<float>(m_window.GetClientWidth());
    float winH = static_cast<float>(m_window.GetClientHeight());
    float deltaTime = gt.DeltaTime();

    // =======================================================
    // 1. MISE A JOUR DE LA CAMERA MANUELLE
    // =======================================================
    CameraComponent* camComp = m_mainCam->GetComponent<CameraComponent>();
    TransformComponent* camTrans = m_mainCam->GetComponent<TransformComponent>();

    if (camComp && camTrans && camComp->m_renderCamera)
    {
        RenderCamera* rc = camComp->m_renderCamera;

        float vx = camComp->m_viewX * winW;
        float vy = camComp->m_viewY * winH;
        float vw = camComp->m_viewWidth * winW;
        float vh = camComp->m_viewHeight * winH;
        float aspect = vw / vh;

        rc->UpdateProjection(0.25f * 3.14159f, aspect, 1.0f, 1000.0f);
        rc->SetViewport(vw, vh, vx, vy);
        rc->SetScissorRect((int)vx, (int)vy, (int)(vx + vw), (int)(vy + vh));

        DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(&camTrans->GetWorldMatrix());
        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(worldMatrix);
        DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixInverse(&det, worldMatrix);

        rc->SetViewMatrix(viewMatrix);
        rc->SetPosition(camTrans->GetWorldPosition());

        m_cameraManager.SetRenderCameras({ rc });
    }

    // =======================================================
    // 2. MISE A JOUR DE L'UI MANUELLE
    // =======================================================
    if (&m_menuManager && m_menuManager.getCurrentMenu())
    {
        std::vector<Entity*> uiEntities = m_menuManager.getCurrentMenu()->entityList;

        std::sort(uiEntities.begin(), uiEntities.end(), [this](const Entity* pA, const Entity* pB)
            {
                auto* tA = this->m_ecs.GetComponent<UiTransformComponent>(pA->GetId());
                auto* tB = this->m_ecs.GetComponent<UiTransformComponent>(pB->GetId());

                float depthA = tA ? tA->m_depth : 0.0f;
                float depthB = tB ? tB->m_depth : 0.0f;

                return depthA < depthB;
            });

        for (Entity* ent : uiEntities)
        {
            if (!ent) continue;

            auto* transform = m_ecs.GetComponent<UiTransformComponent>(ent->GetId());
            if (!transform) continue;

            auto* imageComp = m_ecs.GetComponent<UiImageComponent>(ent->GetId());
            auto* textComp = m_ecs.GetComponent<UiTextComponent>(ent->GetId());
            auto* animComp = m_ecs.GetComponent<UiAnimatorComponent>(ent->GetId());

            float absX = transform->m_x * winW;
            float absY = transform->m_y * winH;
            float absW = transform->m_width * winW;
            float absH = transform->m_height * winH;

            if (imageComp && imageComp->m_isVisible)
            {
                if (animComp && animComp->m_isPlaying)
                {
                    animComp->m_timer += deltaTime;
                    float frameDuration = 1.0f / animComp->m_framesPerSecond;

                    if (animComp->m_timer >= frameDuration) {
                        animComp->m_timer -= frameDuration;
                        animComp->m_currentFrame++;

                        if (animComp->m_currentFrame >= animComp->m_maxFrames) {
                            if (animComp->m_loop) animComp->m_currentFrame = 0;
                            else {
                                animComp->m_currentFrame = animComp->m_maxFrames - 1;
                                animComp->m_isPlaying = false;
                            }
                        }
                    }

                    int textureWidth = animComp->m_maxFrames * animComp->m_height;
                    int textureHeight = animComp->m_width;

                    int cols = textureWidth / animComp->m_height;

                    int currentColumn = animComp->m_currentFrame % cols;
                    int currentRow = animComp->m_currentFrame / cols;

                    imageComp->m_uv0X = (currentColumn * animComp->m_height) / (float)textureWidth;
                    imageComp->m_uv0Y = (currentRow * animComp->m_width) / (float)textureHeight;
                    imageComp->m_uv1X = ((currentColumn + 1) * animComp->m_height) / (float)textureWidth;
                    imageComp->m_uv1Y = ((currentRow + 1) * animComp->m_width) / (float)textureHeight;
                }

                m_uiManager.AddUIImage(absX, absY, absW, absH, imageComp->m_textureHandle,
                    imageComp->m_uv0X, imageComp->m_uv0Y,
                    imageComp->m_uv1X, imageComp->m_uv1Y);
            }

            if (textComp && textComp->m_isVisible && !textComp->m_text.empty())
            {
                m_uiManager.AddUIText(textComp->m_text, absX, absY, textComp->m_font, textComp->m_fontTextureHandle);
            }
        }
    }
}

int EngineCore::Run()
{
    bool splashScreenDismissed = false;

    m_timer.Reset();
    while (true)
    {
        if (!m_window.ProcessMessages()) return 0;

        if (!m_window.IsPaused())
        {
            m_timer.Tick();
            Inputs::UpdateDeltas();

            if (m_loadingFinished)
            {
                if (!splashScreenDismissed)
                {
                    m_menuManager.loadMenuToLoad();
                    m_menuManager. forceUnActiveMenu("SplashScreen");
                    splashScreenDismissed = true;
                }

                m_ecs.Update(m_timer);
                if (OnUpdate) OnUpdate(m_timer);
                DrawFrame();
            }
            else
            {
                UpdateSplashScreen(m_timer);
                DrawFrame();
            }

            Inputs::EndFrameStates();
        }
        else { Sleep(100); }
    }
    return 0;
}

void EngineCore::DrawFrame()
{
    std::lock_guard<std::mutex> lock(m_graphicMutex);

    m_renderer.BeginFrame();

    int camIndex = 0;

    std::vector<RenderCamera*> activeCameras = m_cameraManager.GetRenderCameras();

    m_renderer.SimulateParticles(
        &m_particleManager ? m_particleManager.GetParticles() : std::vector<ParticleRenderData>(),
        m_timer.DeltaTime()
    );

    for (RenderCamera* cam : activeCameras)
    {
        m_renderer.BeginCamera(cam->GetView(), cam->GetProj(), cam->GetPosition(), cam->GetViewport(), cam->GetScissorRect(), camIndex);

        m_renderer.RenderScene(
            m_batchManager.GetBatches(),
            m_lightManager.GetLights(),
            m_animationManager.GetFrameBoneTransforms(),
            m_particleManager.GetParticles(),
            m_filterShaders
        );

        camIndex++;

        if (m_loadingFinished == false) break;
    }

    if (OnRender) OnRender(m_timer);

    if (&m_uiManager && !m_uiManager.GetUiVerticiesCPU().empty()) {
        D3D12_VIEWPORT vp;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(m_window.GetClientWidth());
        vp.Height = static_cast<float>(m_window.GetClientHeight());
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;

        D3D12_RECT sr;
        sr.left = 0;
        sr.top = 0;
        sr.right = m_window.GetClientWidth();
        sr.bottom = m_window.GetClientHeight();

        m_renderer.RenderUI(
            m_uiManager.GetUiVerticiesCPU(),
            m_uiManager.GetUIVertexBuffer().get(),
            vp.Width,
            vp.Height,
            vp, sr
        );
        m_uiManager.ClearUI();
    }

    m_batchManager.Clear();
    m_lightManager.Clear();
    m_animationManager.Clear();
    m_particleManager.Clear();

    m_renderer.EndFrame();
}

void EngineCore::OnResize()
{
    if (&m_renderer) {
        m_clientWidth = m_window.GetClientWidth();
        m_clientHeight = m_window.GetClientHeight();

        m_renderer.OnResize(m_clientWidth, m_clientHeight);
    }
}

#pragma region ShaderFilter
void EngineCore::DrawSolidShader(bool _enable) {
    if (_enable) m_filterShaders |= RenderFlags::Solid;
    else         m_filterShaders &= ~RenderFlags::Solid;
}

void EngineCore::DrawSolidShader() {
    m_filterShaders ^= RenderFlags::Solid;
}

void EngineCore::DrawWireframeShader(bool _enable) {
    if (_enable) m_filterShaders |= RenderFlags::Wireframe;
    else         m_filterShaders &= ~RenderFlags::Wireframe;
}

void EngineCore::DrawWireframeShader() {
    m_filterShaders ^= RenderFlags::Wireframe;
}

void EngineCore::DrawPostProcessShader(bool _enable) {
    if (_enable) m_filterShaders |= RenderFlags::Wireframe;
    else         m_filterShaders &= ~RenderFlags::Wireframe;
}

void EngineCore::DrawPostProcessShader() {
    m_filterShaders ^= RenderFlags::Wireframe;
}
#pragma endregion