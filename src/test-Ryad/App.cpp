#include "App.h"

#include "TestScript.h"

using namespace DirectX;

App::App() {}
App::~App() {}

#pragma region Init Methods

void App::Init()
{

    InitEntities();

    InitCameras();

    InitLights();

    InitUis();
}

void App::InitEntities()
{
    ECS* ecs = ECS_ECS;

    // --- Entity 1 ---

    entity1 = ECS_ECS->CreateEntity();

    entity1->transform.SetWorldPosition({ 0.0f, -2.0f, -2.0f });
    entity1->transform.LocalTranslate({ 0.0f, -0.225f, 0.0f });
    entity1->transform.LocalRotate({ 0, 180, 0 });
    entity1->transform.SetWorldScale({ 0.02f, 0.02f, 0.02f });

    MeshComponent* mesh1 = ecs->AddComponents<MeshComponent>(entity1);
    mesh1->LoadMesh("../../res/soldier.m3d");

    AnimatorComponent* animator1 = ecs->AddComponents<AnimatorComponent>(entity1);
    animator1->LoadSkeleton("../../res/soldier.m3d");

    MaterialComponent* mat1 = ecs->AddComponents<MaterialComponent>(entity1);
    mat1->LoadTexture("default");
    mat1->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

    RigidBodyComponent* rb1 = ecs->AddComponents<RigidBodyComponent>(entity1);
    rb1->m_motionType = MotionType::Dynamic;

    ColliderComponent* collider1 = ecs->AddComponents<ColliderComponent>(entity1);
    collider1->SetAsBox({ 0.5f, 0.5f, 0.5f });

    // --- Entity 2 ---

    entity2 = Geometry::Cube();

    entity2->transform.SetWorldPosition({ 0, 3, 0 });

    MaterialComponent* mat2 = ecs->AddComponents<MaterialComponent>(entity2);
    mat2->LoadTexture("../../res/Board.png");

    RigidBodyComponent* rb2 = ecs->AddComponents<RigidBodyComponent>(entity2);
    rb2->m_motionType = MotionType::Dynamic;

    ColliderComponent* collider2 = ecs->AddComponents<ColliderComponent>(entity2);
    collider2->SetAsBox({ 0.5f, 0.5f, 0.5f });

    ScriptComponent* script2 = ecs->AddComponents<ScriptComponent>(entity2);
    script2->SetScript<TestScript>();

    // --- Ground ---

	ground = Geometry::Cube();
    ground->transform.SetWorldPosition({ 0.0f, -3.0f, 0.0f });
    ground->transform.SetWorldScale({ 8.0f, 1.5f, 8.0f });

    MaterialComponent* matGround = ecs->AddComponents<MaterialComponent>(ground);
	matGround->SetColor({ 0.3f, 0.3f, 0.3f, 1.0f });

    RigidBodyComponent* rbGround = ecs->AddComponents<RigidBodyComponent>(ground);
    rbGround->m_motionType = MotionType::Static;

    ColliderComponent* colliderGround = ecs->AddComponents<ColliderComponent>(ground);
    colliderGround->SetAsBox({ 0.5f, 0.5f, 0.5f });

	// --- Particle Emitter 1 ---

	emitter1 = ECS_ECS->CreateEntity();

	emitter1->transform.SetWorldPosition({ 0.0f, -1.6f, -1.8f });
    
	ParticleEmitterComponent* emitterComp = ecs->AddComponents<ParticleEmitterComponent>(emitter1);

    emitterComp->spawnRadius = 0.00000001;
    emitterComp->gravity = 0.5f;
    emitterComp->speed = 0.5f;
    emitterComp->spread = 1.0001f;
    emitterComp->minLife = 0.8f;
    emitterComp->maxLife = 0.9f;
    emitterComp->sizeStart = 0.08f;
    emitterComp->sizeEnd = 0.02f;
    emitterComp->colorStartR = 0.0f;  emitterComp->colorStartG = 1.f;  emitterComp->colorStartB = 0.0f;  emitterComp->colorStartA = 1.0f;
    emitterComp->colorEndR = 0.0f;    emitterComp->colorEndG = 1.0f;    emitterComp->colorEndB = 0.5f;    emitterComp->colorEndA = 0.0f;
    emitterComp->maxParticles = 100000;
    emitterComp->emitRate = 10000;
}

void App::InitCameras()
{
    float x = radius * sinf(phi) * cosf(theta);
    float z = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);

    Entity* cam1 = ECS_ECS->GetComponents<CameraComponent>()[0]->GetOwner();
    cameras.push_back(cam1);

	// --- Camera 2 ---

    Entity* cam2 = Camera::Camera(0.73f, 0.73f, 0.25f, 0.25f, 1);
    cam2->transform.SetWorldPosition({ 0.0f, 5.0f, -10.0f });
    cam2->transform.LookAt(entity2->transform.GetWorldPosition());

    cameras.push_back(cam2);
}

void App::InitLights()
{
    // --- image 1 ---

    Light::Directional({ 0.5f, -1.0f, -0.5f });
}

void App::InitUis()
{
    // --- image 1 (Minimap) ---
    Ui::Image(0.850f, 0.850f, 0.269f, 0.275f, "../../res/Minimap.png");

    // --- text 1 (FPS) ---
	textFps = Ui::Text(0.02f, 0.02f, "fps : ");
}

#pragma endregion

void App::Update(const GameTimer& gt)
{
    float dt = gt.DeltaTime();

    m_timerFps += dt;
    if (m_timerFps >= 1.f)
    {
        m_timerFps = 0.0f;
        std::string text = "fps : " + std::to_string((int)(1.0f / dt));

        UiTextComponent* textComp = textFps->GetComponent<UiTextComponent>();
        if (textComp) {
            textComp->m_text = text;
        }
    }
    
    float x = radius * sinf(phi) * cosf(theta);
    float z = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);

    cameras[0]->transform.SetWorldPosition({ x, y, z });

    if (entity1) {
        cameras[0]->transform.LookAt(entity1->transform.GetWorldPosition());
    }
    else if (entity2) {
        cameras[0]->transform.LookAt(entity2->transform.GetWorldPosition());
    }
    cameras[1]->transform.LookAt(entity2->transform.GetWorldPosition());

    if (Inputs::IsKeyPressed(Keyboard::I))
        radius += dt * 20;
    if (Inputs::IsKeyPressed(Keyboard::O))
        radius -= dt * 20;

    // --- LOGIQUE INPUT ---

    if (entity1)
    {
        XMFLOAT3 deltaMove = { 0, 0, 0 };

        if (Inputs::IsKeyPressed(Keyboard::Q))
            deltaMove = deltaMove + cameras[0]->transform.GetWorldLeft();
        if (Inputs::IsKeyPressed(Keyboard::D))
            deltaMove = deltaMove + cameras[0]->transform.GetWorldRight();
        if (Inputs::IsKeyPressed(Keyboard::Z))
            deltaMove = deltaMove + cameras[0]->transform.GetWorldForward();
        if (Inputs::IsKeyPressed(Keyboard::S))
            deltaMove = deltaMove + cameras[0]->transform.GetWorldBackward();

        deltaMove.y = 0;

        float length = sqrt(deltaMove.x * deltaMove.x + deltaMove.z * deltaMove.z);

        if (length > 0.001f)
        {
            deltaMove.x /= length;
            deltaMove.z /= length;

            XMFLOAT3 currentPos = entity1->transform.GetWorldPosition();
            XMFLOAT3 lookTarget = {
                currentPos.x - deltaMove.x,
                currentPos.y,
                currentPos.z - deltaMove.z
            };
            entity1->transform.LookAt(lookTarget);

            float moveDist = m_speed * dt;
            XMFLOAT3 finalMove = { deltaMove.x * moveDist, 0, deltaMove.z * moveDist };

            entity1->transform.WorldTranslate(finalMove);

            if (isMoving == false)
            {
                entity1->GetComponent<AnimatorComponent>()->Play("Take1", true, 1.0f);
                isMoving = true;
            }
        }
        else
        {
            if (isMoving == true)
            {
                entity1->GetComponent<AnimatorComponent>()->Stop();
                isMoving = false;
            }
        }

        if (Inputs::IsKeyDown(Keyboard::SPACE))
            entity1->GetComponent<RigidBodyComponent>()->AddForce({ 0, m_jumpforce, 0 });

        if (Inputs::IsKeyDown(Keyboard::A)) {
            ECS_ECS->DestroyEntity(entity1);
            entity1 = nullptr;
        }
    }

    // Shaders de debug
    if (Inputs::IsKeyDown(Keyboard::F1)) ECS_ENGINE->DrawSolidShader();
    if (Inputs::IsKeyDown(Keyboard::F2)) ECS_ENGINE->DrawWireframeShader();
    if (Inputs::IsKeyDown(Keyboard::T)) Utils::SwitchMenu("SplashScreen");

    // Changement de caméra
    if (Inputs::IsKeyDown(Keyboard::M)) {
        if (switchCamera)
        {
            CameraComponent* cam1 = cameras[0]->GetComponent<CameraComponent>();
            cam1->m_viewWidth = 0.25f;
            cam1->m_viewHeight = 0.25f;
            cam1->m_viewX = 0.73f;
            cam1->m_viewY = 0.73f;
            cam1->m_renderOrder = 1;

            CameraComponent* cam2 = cameras[1]->GetComponent<CameraComponent>();
            cam2->m_viewWidth = 1.f;
            cam2->m_viewHeight = 1.f;
            cam2->m_viewX = 0.f;
            cam2->m_viewY = 0.f;
            cam2->m_renderOrder = 0;
        }
        else
        {
            CameraComponent* cam1 = cameras[0]->GetComponent<CameraComponent>();
            cam1->m_viewWidth = 1.f;
            cam1->m_viewHeight = 1.f;
            cam1->m_viewX = 0.f;
            cam1->m_viewY = 0.f;
            cam1->m_renderOrder = 0;

            CameraComponent* cam2 = cameras[1]->GetComponent<CameraComponent>();
            cam2->m_viewWidth = 0.25f;
            cam2->m_viewHeight = 0.25f;
            cam2->m_viewX = 0.73f;
            cam2->m_viewY = 0.73f; 
            cam2->m_renderOrder = 1;
        }
		switchCamera = !switchCamera;
    }

    // Gestion souris
    if (Inputs::IsMousePressed(Mouse::LEFT))
    {
        DirectX::XMFLOAT2 mouseDelta = Inputs::GetMouseDelta();
        float dx = XMConvertToRadians(0.25f * mouseDelta.x);
        float dy = XMConvertToRadians(0.25f * mouseDelta.y);

        theta += dx;
        phi += dy;

        if (phi < 0.1f) phi = 0.1f;
        if (phi > XM_PI - 0.1f) phi = XM_PI - 0.1f;
    }

    if (Inputs::IsMousePressed(Mouse::RIGHT))
    {
        DirectX::XMFLOAT2 mouseDelta = Inputs::GetMouseDelta();
        float dx = XMConvertToRadians(0.25f * mouseDelta.x);
        float dy = XMConvertToRadians(0.25f * mouseDelta.y);

        radius += dx - dy;
        if (radius < 1.0f) radius = 1.0f;
    }
}