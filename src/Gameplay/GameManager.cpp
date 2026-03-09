#include "GameManager.h"
#include "ShootingScript.h"
#include "LifeScript.h"
#include "Rail.h"
#include "RailScript.h"
#include "SushiArmScript.h"
#include <ECS_Engine.h>

GameManager* GameManager::m_instance = nullptr;

GameManager* GameManager::Getinstance() {
    if (m_instance == nullptr) {
        m_instance = new GameManager();
    }
    return m_instance;
}

#pragma region init Method

void GameManager::Init() {
    ecs = ECS_ECS;

    InitEntities();

    InitCameras();

    InitLights();

    InitUis();

    m_replay.StartRecording(PlayerId);
}

void GameManager::InitEntities()
{
    m_rail = new Rail();
    m_rail->segmentLength = 20.f;
    m_rail->maxYawDelta = 0.42f;
    m_rail->maxPitchDelta = 0.22f;
    m_rail->maxRollDelta = 0.28f;

    ////-------Create Seagull so no lag------   
    //Entity* Seagull = ECS_ECS->CreateEntity();

 //   MeshComponent* meshSeagull = ecs->AddComponents<MeshComponent>(Seagull);
 //   meshSeagull->LoadMesh("../../res/seagull.m3d");
 //   Seagull->transform.SetWorldPosition({ 0.f, -100000.f, 0.f });

 //   AnimatorComponent* animatorSeagull = ECS_ECS->AddComponents<AnimatorComponent>(Seagull);
 //   animatorSeagull->LoadSkeleton("../../res/seagull.m3d");
 //   animatorSeagull->Play("ArmatureAction.001", true, 1.0f);

 //   MaterialComponent* matSeagull = ECS_ECS->AddComponents<MaterialComponent>(Seagull);
 //   matSeagull->LoadTexture("../../res/SeagullTexture.png");

    //ECS_ECS->DestroyEntity(Seagull);
    // --- Entity 1 (Joueur) ---

    four = ECS_ECS->CreateEntity();

    MeshComponent* mesh1 = ecs->AddComponents<MeshComponent>(four);
    mesh1->LoadMesh("../../res/Four.obj");

    four->SetType(Entity::TYPE::Player);
    four->transform.SetWorldScale({ 1.f, 1.f, 1.f });
    four->transform.SetWorldPosition({0, 1, 0});
    MaterialComponent* mat1 = ecs->AddComponents<MaterialComponent>(four);
    mat1->LoadTexture("../../res/KitchenMachinery_B_basecolor.png");

    RigidBodyComponent* rb1 = ecs->AddComponents<RigidBodyComponent>(four);
    rb1->m_motionType = MotionType::Kinematic;

    ColliderComponent* collider1 = ecs->AddComponents<ColliderComponent>(four);
    collider1->SetAsBox({ 0.5f, 0.5f, 0.5f });


    Sol = ECS_ECS->CreateEntity();

    MeshComponent* mesh2 = ecs->AddComponents<MeshComponent>(Sol);
    mesh2->LoadMesh("cube");

    
    Sol->transform.SetWorldScale({ 10.f, 0.02f, 10.f });

    MaterialComponent* mat2 = ecs->AddComponents<MaterialComponent>(Sol);
    mat2->LoadTexture("../../res/Sol.png");

    RigidBodyComponent* rb2 = ecs->AddComponents<RigidBodyComponent>(Sol);
    rb2->m_motionType = MotionType::Static;

    ColliderComponent* collider2 = ecs->AddComponents<ColliderComponent>(Sol);
    collider2->SetAsBox({ 10.f, 0.02f, 10.f });




	//------Sushi------
    Sushi = ECS_ECS->CreateEntity();
    PlayerId = Sushi->GetId();
    MeshComponent* meshSushi = ecs->AddComponents<MeshComponent>(Sushi);
    meshSushi->LoadMesh("../../res/sushiBody.obj");


    RigidBodyComponent* rb3 = ecs->AddComponents<RigidBodyComponent>(Sushi);
    rb2->m_motionType = MotionType::Static;

    ColliderComponent* collider3 = ecs->AddComponents<ColliderComponent>(Sushi);
    collider2->SetAsBox({ 1.f, 3.f, 1.f });

    MaterialComponent* matSushi = ecs->AddComponents<MaterialComponent>(Sushi);
    matSushi->LoadTexture("../../res/Sushi.png");
	//------Sushi RightArms------
	SushiRightArm = ECS_ECS->CreateEntity();
    MeshComponent* meshSushi2 = ecs->AddComponents<MeshComponent>(SushiRightArm);
    meshSushi2->LoadMesh("../../res/sushiDroit.obj");

    MaterialComponent* matSushi2 = ecs->AddComponents<MaterialComponent>(SushiRightArm);
	matSushi2->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

    ScriptComponent* rightArmScript = ecs->AddComponents<ScriptComponent>(SushiRightArm);
    rightArmScript->SetScript<SushiArmScript>();
    SushiArmScript* rightArm = static_cast<SushiArmScript*>(rightArmScript->m_instance);
    rightArm->bodyEntity = Sushi;
    rightArm->isLeftArm = false;

	//------Sushi LeftArms------
    SushiLeftArm = ECS_ECS->CreateEntity();
    MeshComponent* meshSushi3 = ecs->AddComponents<MeshComponent>(SushiLeftArm);
    meshSushi3->LoadMesh("../../res/sushiGauche.obj");

    MaterialComponent* matSushi3 = ecs->AddComponents<MaterialComponent>(SushiLeftArm);
    matSushi3->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

    ScriptComponent* leftArmScript = ecs->AddComponents<ScriptComponent>(SushiLeftArm);
    leftArmScript->SetScript<SushiArmScript>();
    SushiArmScript* leftArm = static_cast<SushiArmScript*>(leftArmScript->m_instance);
    leftArm->bodyEntity = Sushi;
    leftArm->isLeftArm = true;

    //-------Sushi pos------------
	XMFLOAT3 pos = Sushi->transform.GetWorldPosition();
    Sushi->transform.SetWorldPosition({ pos.x + 0.25f,pos.y + 10.f,pos.z });
	Sushi->transform.SetWorldScale({ 0.4f,0.4f,0.4f });
    Sushi->transform.SetWorldRotation({ 0.f, 110.f, 0.f });


    Sushi->AddChild(SushiRightArm);
    Sushi->AddChild(SushiLeftArm);
}

void GameManager::InitCameras()
{
    // --- Camera 1 ---
    Entity* cam1 = ECS_ECS->GetComponents<CameraComponent>()[0]->GetOwner();
    Sushi->AddChild(cam1);

    XMFLOAT3 playerPos = Sushi->transform.GetWorldPosition();
    cam1->transform.SetWorldPosition(playerPos);
    cam1->transform.SetLocalPosition({ 0, 4.0f, -7.f });
    cam1->transform.LookAt({ playerPos.x, playerPos.y + 1, playerPos.z });
    
    // --- Camera 2 ---
    Entity* cam2 = Camera::Camera(0.73f, 0.73f, 0.25f, 0.25f, 1);
    cam2->transform.SetWorldPosition({ 0.0f, 5.0f, -10.0f });
    cam2->transform.LookAt(Sushi->transform.GetWorldPosition());

    cameras.push_back(cam1);
    cameras.push_back(cam2);
}

void GameManager::InitLights()
{
    // --- image 1 ---

    Light::Directional({ 0.5f, -1.0f, -0.5f });
}

void GameManager::InitUis()
{
    // --- image 1 (Minimap) ---
    Ui::Image(0.715f, 0.715f, 0.269f, 0.275f, "../../res/Minimap.png");

    // --- text 1 (FPS) ---
    textFps = Ui::Text(0.02f, 0.02f, "fps : ");
}

#pragma endregion

void GameManager::Update(const GameTimer& gt)
{
   
    float x = radius * sinf(phi) * cosf(theta);
    float z = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);

    cameras[0]->transform.SetWorldPosition({ x, y, z });

    if (Sushi) {
        cameras[0]->transform.LookAt(Sushi->transform.GetWorldPosition());
    }
  
    if (!Sushi && !m_playerDead || Inputs::IsKeyDown(Keyboard::R))
    {
        m_playerDead = true;
        m_replay.StopRecording();
        m_replay.StartReplay();


        if (cameras[1])
        {
            CameraComponent* cam2 = cameras[1]->GetComponent<CameraComponent>();
            cam2->m_viewWidth = 1.f;
            cam2->m_viewHeight = 1.f;
            cam2->m_viewX = 0.f;
            cam2->m_viewY = 0.f;
            cam2->m_renderOrder = 0;
        }
    }

 
    if (Inputs::IsKeyDown(Keyboard::T))
    {
        ECS_APP->GetTimer().ToggleSlowTimeDown();
    }

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
    if (Sushi)
    {
        float rotSpeed = m_speed * dt;
        XMFLOAT3 deltaMove = { 0,0,0 };
 
        // Rotation Cube
        if (Inputs::IsKeyPressed(Keyboard::LEFT_ARROW))
            Sushi->transform.WorldRotate({ 0, -rotSpeed, 0 });
        if (Inputs::IsKeyPressed(Keyboard::RIGHT_ARROW))
            Sushi->transform.WorldRotate({ 0, rotSpeed, 0 });

        if (Inputs::IsKeyPressed(Keyboard::Z))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalForward() * gt.DeltaTime());
        if (Inputs::IsKeyPressed(Keyboard::S))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalBackward() * gt.DeltaTime());

        if (Inputs::IsKeyPressed(Keyboard::Q))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalLeft() * gt.DeltaTime());
        if (Inputs::IsKeyPressed(Keyboard::D))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalRight() * gt.DeltaTime());

    }

    // Shaders de debug
    if (Inputs::IsKeyDown(Keyboard::F1)) ECS_ENGINE->DrawSolidShader();
    if (Inputs::IsKeyDown(Keyboard::F2)) ECS_ENGINE->DrawWireframeShader();
    if (Inputs::IsKeyDown(Keyboard::F5)) ECS_ENGINE->DrawPostProcessShader(); 

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