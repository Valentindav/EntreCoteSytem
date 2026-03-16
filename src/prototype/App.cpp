#include "App.h"

#include "PlayerMovementScript.h"
#include "PlayerCatchScript.h"

using namespace DirectX;

App::App() {}
App::~App() {}

#pragma region Init Methods

void App::Init()
{
    InitEntities();
    InitLights();
    InitUis();
}

void App::InitEntities()
{
    Inputs::LockMouse(true);
    Inputs::ShowCursor(false);

    ECS* ecs = ECS_ECS;

    // ==========================================
    // --- PLAYER ROOT (Physique & Position Monde)
    // ==========================================
    player = ecs->CreateEntity();
    XMFLOAT3 playerPos = { 0.0f, 6.0f, 0.0f };
    player->transform.SetWorldPosition(playerPos);

    RigidBodyComponent* rb1 = ecs->AddComponents<RigidBodyComponent>(player);
    rb1->m_motionType = MotionType::Kinematic;
    rb1->m_bounciness = 0;

    /*
    // ==========================================
    // --- PLAYER MESH (Visuel & Animation)
    // ==========================================
    Entity* playerMesh = ecs->CreateEntity();
    playerMesh->transform.SetLocalScale({ 0.02f, 0.02f, 0.02f });

    MeshComponent* mesh1 = ecs->AddComponents<MeshComponent>(playerMesh);
    mesh1->LoadMesh("../../res/soldier.m3d");

    AnimatorComponent* animator1 = ecs->AddComponents<AnimatorComponent>(playerMesh);
    animator1->LoadSkeleton("../../res/soldier.m3d");

    MaterialComponent* mat1 = ecs->AddComponents<MaterialComponent>(playerMesh);
    mat1->LoadTexture("default");
    mat1->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

    player->AddChild(playerMesh);
    */
    
    // ==========================================
    // --- PLAYER HITBOX (Collision pour bloquer sur les murs)
    // ==========================================
    Entity* playerHitbox = ecs->CreateEntity();
    playerHitbox->transform.SetLocalPosition({ 0.0f, 0.8f, 0.0f });
    playerHitbox->transform.SetLocalScale({ 0.4f, 1.475f, 0.4f });

    ColliderComponent* collider1 = ecs->AddComponents<ColliderComponent>(playerHitbox);
    collider1->SetAsBox({ 0.5f, 0.5f, 0.5f });

    player->AddChild(playerHitbox);

    // ==========================================
    // --- LIAISON DU PLAYER MOVEMENT SCRIPT ---
    // ==========================================
    ScriptComponent* moveScriptComp = ecs->AddComponents<ScriptComponent>(player);
    PlayerMovementScript* moveScript = moveScriptComp->SetScript<PlayerMovementScript>();
    cameraPlayer = ECS_ECS->GetComponents<CameraComponent>()[0]->GetOwner();

    moveScript->SetCamera(cameraPlayer);
    moveScript->SetHitbox(playerHitbox);
    //moveScript->SetMesh(playerMesh);

    // ==========================================
    // --- LIAISON DU PLAYER CATCH SCRIPT ---
    // ==========================================

    ScriptComponent* catchScriptComp = ecs->AddComponents<ScriptComponent>(player);
    PlayerCatchScript* catchScript = catchScriptComp->SetScript<PlayerCatchScript>();

    catchScript->SetCamera(cameraPlayer);
    catchScript->SetHitbox(playerHitbox);

    // ==========================================
    // --- ENVIRONNEMENT
    // ==========================================
    Entity* ground = Geometry::Cube();
    ground->transform.SetWorldPosition({ 0.0f, 0.0f, 0.0f });
    ground->transform.SetWorldScale({ 20.0f, 1.5f, 20.0f });

    MaterialComponent* matGround = ecs->AddComponents<MaterialComponent>(ground);
    matGround->SetColor({ 0.3f, 0.3f, 0.3f, 1.0f });

    RigidBodyComponent* rbGround = ecs->AddComponents<RigidBodyComponent>(ground);
    rbGround->m_motionType = MotionType::Static;

    ColliderComponent* colliderGround = ecs->AddComponents<ColliderComponent>(ground);
    colliderGround->SetAsBox({ 0.5f, 0.5f, 0.5f });

    // ==========================================
    // --- BOX
    // ==========================================

    Entity* food = Geometry::Cube();
    food->transform.SetWorldPosition({ 0.f, 1.0f, 3.0f });
    food->transform.SetWorldScale({ 1.f, 1.f, 1.f });

    MaterialComponent* matFood = ecs->AddComponents<MaterialComponent>(food);
    matFood->SetColor({ 1.f, 0.f, 0.f, 1.f });

    RigidBodyComponent* rbFood = ecs->AddComponents<RigidBodyComponent>(food);
    rbFood->m_motionType = MotionType::Dynamic;

    ColliderComponent* colliderFood = ecs->AddComponents<ColliderComponent>(food);
    colliderFood->SetAsBox({ 0.5f, 0.5f, 0.5f });

    TagComponent* tag = ecs->AddComponents<TagComponent>(food);
    tag->m_tag = "catchable";
}

void App::InitLights()
{
    Light::Directional({ 0.5f, -1.0f, -0.5f });
}

void App::InitUis()
{
    Ui::Image(0.5f, 0.5f, 0.05f, 0.05f , "../../res/cross_air.png");
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
}