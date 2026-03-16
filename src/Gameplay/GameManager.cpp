#include "GameManager.h"
#include "ShootingScript.h"
#include "LifeScript.h"
#include "Rail.h"
#include "RailScript.h"
#include "SushiArmScript.h"
#include <ECS_Engine.h>
#include "Audio/Audio.h"

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

    InitAudio();

    m_replay.StartRecording(PlayerId);
}

void GameManager::InitEntities()
{
    m_rail = new Rail();
    m_rail->segmentLength = 20.f;
    m_rail->maxYawDelta = 0.42f;
    m_rail->maxPitchDelta = 0.22f;
    m_rail->maxRollDelta = 0.28f;

	//-------Create Seagull so no lag------   
	Entity* Seagull = ECS_ECS->CreateEntity();

    MeshComponent* meshSeagull = ecs->AddComponents<MeshComponent>(Seagull);
    meshSeagull->LoadMesh("../../res/seagull.m3d");
    Seagull->transform.SetWorldPosition({ 0.f, -100000.f, 0.f });

    AnimatorComponent* animatorSeagull = ECS_ECS->AddComponents<AnimatorComponent>(Seagull);
    animatorSeagull->LoadSkeleton("../../res/seagull.m3d");
    animatorSeagull->Play("ArmatureAction.001", true, 1.0f);

    MaterialComponent* matSeagull = ECS_ECS->AddComponents<MaterialComponent>(Seagull);
    matSeagull->LoadTexture("../../res/SeagullTexture.png");

	ECS_ECS->DestroyEntity(Seagull);
    // --- Entity 1 (Joueur) ---

	Player = ECS_ECS->CreateEntity();

	MeshComponent* mesh1 = ecs->AddComponents<MeshComponent>(Player);
	mesh1->LoadMesh("../../res/Boat.obj");

    PlayerId = Player->GetId();
    Player->SetType(Entity::TYPE::Player);
	Player->transform.SetWorldScale({ 0.5f, 0.5f, 0.5f });

    MaterialComponent* mat1 = ecs->AddComponents<MaterialComponent>(Player);
    mat1->LoadTexture("../../res/Boat.png");

    RigidBodyComponent* rb1 = ecs->AddComponents<RigidBodyComponent>(Player);
    rb1->m_motionType = MotionType::Kinematic;

    ColliderComponent* collider1 = ecs->AddComponents<ColliderComponent>(Player);
    collider1->SetAsBox({ 0.5f, 0.5f, 0.5f });

    ScriptComponent* script1 = ecs->AddComponents<ScriptComponent>(Player);
    script1->SetScript<ShootScript>();

    ScriptComponent* scriptLife = ecs->AddComponents<ScriptComponent>(Player);
    scriptLife->SetScript<LifeScript>();

    ScriptComponent* scriptPoint = ecs->AddComponents<ScriptComponent>(Player);
    scriptPoint->SetScript<PointScript>();

    ScriptComponent* followerComp = ecs->AddComponents<ScriptComponent>(Player);
    followerComp->SetScript<RailFollower>();
    RailFollower* rf = static_cast<RailFollower*>(followerComp->m_instance);
    rf->spline = m_rail;

    ScriptComponent* spawnerComp = ecs->AddComponents<ScriptComponent>(Player);
    spawnerComp->SetScript<EnemySpawner>();
    EnemySpawner* es = static_cast<EnemySpawner*>(spawnerComp->m_instance);
    es->spline = m_rail;
    es->railFollower = static_cast<RailFollower*>(followerComp->m_instance);

	//------Sushi------
    Sushi = ECS_ECS->CreateEntity();
    MeshComponent* meshSushi = ecs->AddComponents<MeshComponent>(Sushi);
    meshSushi->LoadMesh("../../res/sushiBody.obj");

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
    rightArm->bodyEntity = Player;
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
    leftArm->bodyEntity = Player;
    leftArm->isLeftArm = true;

    //-------Sushi pos------------
	XMFLOAT3 pos = Player->transform.GetWorldPosition();
    Sushi->transform.SetWorldPosition({ pos.x + 0.25f,pos.y + 1.f,pos.z });
	Sushi->transform.SetWorldScale({ 0.4f,0.4f,0.4f });
    Sushi->transform.SetWorldRotation({ 0.f, 110.f, 0.f });


    Player->AddChild(SushiRightArm);
    Player->AddChild(SushiLeftArm);
    Player->AddChild(Sushi);
}

void GameManager::InitCameras()
{
    // --- Camera 1 ---
    Entity* cam1 = ECS_ECS->GetComponents<CameraComponent>()[0]->GetOwner();
    Player->AddChild(cam1);

    XMFLOAT3 playerPos = Player->transform.GetWorldPosition();
    cam1->transform.SetWorldPosition(playerPos);
    cam1->transform.SetLocalPosition({ 0, 4.0f, -7.f });
    cam1->transform.LookAt({ playerPos.x, playerPos.y + 1, playerPos.z });
    
    // --- Camera 2 ---
    Entity* cam2 = Camera::Camera(0.73f, 0.73f, 0.25f, 0.25f, 1);
    cam2->transform.SetWorldPosition({ 0.0f, 5.0f, -10.0f });
    cam2->transform.LookAt(Player->transform.GetWorldPosition());

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

void GameManager::InitAudio()
{
    audio::LoadSound("music_gameplay", L"../../res/Audio/song.mp3", 0, false);

    audio::SetVolumeOfCategory(audio::Category::MUSIC, 80);

    audio::SetAudioCategory("music_gameplay", audio::Category::MUSIC);

    audio::Play("music_gameplay", true);
}

#pragma endregion

void GameManager::Update(const GameTimer& gt)
{
    if (PlayerId >= 0) {
        Player = ECS_ECS->GetEntity(PlayerId);
    }
    else {
        Player = nullptr;
    }


    if (!Player && !m_playerDead || Inputs::IsKeyDown(Keyboard::R))
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

    if (m_replay.state == Replay::State::Replaying)
    {
        bool stillPlaying = m_replay.ReplayFrame();

        DirectX::XMFLOAT3 center = m_replay.GetCurrentPlayerPos();

        if (Inputs::IsMousePressed(Mouse::LEFT))
        {
            DirectX::XMFLOAT2 mouseDelta = Inputs::GetMouseDelta();
            m_replayTheta += XMConvertToRadians(0.25f * mouseDelta.x);
            m_replayPhi += XMConvertToRadians(0.25f * mouseDelta.y);
            if (m_replayPhi < 0.1f) m_replayPhi = 0.1f;
            if (m_replayPhi > XM_PI - 0.1f) m_replayPhi = XM_PI - 0.1f;
        }

        float x = m_replayRadius * sinf(m_replayPhi) * sinf(m_replayTheta);
        float y = m_replayRadius * cosf(m_replayPhi);
        float z = m_replayRadius * sinf(m_replayPhi) * cosf(m_replayTheta);

        if (cameras[1])
        {
            cameras[1]->transform.SetWorldPosition({ center.x + x, center.y + y, center.z + z });
            cameras[1]->transform.LookAt(center);
        }

        if (!stillPlaying) m_playerDead = false;
        return;
    }

    if (m_replay.state == Replay::State::Recording)
        m_replay.RecordFrame();

    if (Inputs::IsKeyDown(Keyboard::T))
    {
        ECS_APP->GetTimer().ToggleSlowTimeDown();
    }

    float dt = gt.DeltaTime();
    audio::Update(dt);

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
    if (Player)
    {
        float rotSpeed = m_speed * dt;
        XMFLOAT3 deltaMove = { 0,0,0 };
 
        // Rotation Cube
        if (Inputs::IsKeyPressed(Keyboard::LEFT_ARROW))
            Player->transform.WorldRotate({ 0, -rotSpeed, 0 });
        if (Inputs::IsKeyPressed(Keyboard::RIGHT_ARROW))
            Player->transform.WorldRotate({ 0, rotSpeed, 0 });

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

}