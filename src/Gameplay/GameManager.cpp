#include "GameManager.h"
#include "ShootingScript.h"
#include "LifeScript.h"
#include "Rail.h"
#include "RailScript.h"
#include "SushiArmScript.h"
#include <ECS_Engine.h>
#include "PnjScript.h"

#include "CommandeScript.h"
#include "PlatScript.h"
#include "IngredientScript.h"
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
    four->transform.SetWorldPosition({0, 0.45, 0});
    MaterialComponent* mat1 = ecs->AddComponents<MaterialComponent>(four);
    mat1->LoadTexture("../../res/KitchenMachinery_B_basecolor.png");

    RigidBodyComponent* rb1 = ecs->AddComponents<RigidBodyComponent>(four);
    rb1->m_motionType = MotionType::Static;

    ColliderComponent* collider1 = ecs->AddComponents<ColliderComponent>(four);
    collider1->SetAsBox({ 0.5f, 0.5f, 0.5f });


    CommandeCollector = ECS_ECS->CreateEntity();

    MeshComponent* meshCommandeCollector = ecs->AddComponents<MeshComponent>(CommandeCollector);
    meshCommandeCollector->LoadMesh("../../res/commandeCollector.obj");

   CommandeCollector->SetType(Entity::TYPE::Player);
   CommandeCollector->transform.SetWorldScale({ 1.f, 1.f, 1.f });
   CommandeCollector->transform.SetWorldPosition({ 0, 0.45, 5 });
   MaterialComponent* matCommandeCollector = ecs->AddComponents<MaterialComponent>(CommandeCollector);
    matCommandeCollector->LoadTexture("../../res/KitchenMachinery_B_basecolor.png");

    RigidBodyComponent* rbCommandeCollector = ecs->AddComponents<RigidBodyComponent>(CommandeCollector);
    rbCommandeCollector->m_motionType = MotionType::Static;

    ColliderComponent* colliderCommandeCollector = ecs->AddComponents<ColliderComponent>(CommandeCollector);
    colliderCommandeCollector->SetAsBox({ 0.5f, 0.5f, 0.5f });

	CommandeCollector->SetName("CommandeCollector");

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

	Customer1 = ECS_ECS->CreateEntity();
    ScriptComponent* scriptCustomer1 = ecs->AddComponents<ScriptComponent>(Customer1);
    scriptCustomer1->SetScript<PnjScript>();


    MeshComponent* mesh1Custom= ecs->AddComponents<MeshComponent>(Customer1);
    mesh1Custom->LoadMesh("../../res/Customer.m3d");

    AnimatorComponent* animatorCustomer1 = ECS_ECS->AddComponents<AnimatorComponent>(Customer1);
    animatorCustomer1->LoadSkeleton("../../res/Customer.m3d");
    animatorCustomer1->Play("Armature|mixamo.com|Layer0.002", true, 1.0f);


    RigidBodyComponent* rbCustomer1 = ecs->AddComponents<RigidBodyComponent>(Customer1);
    rbCustomer1->m_motionType = MotionType::Static;

    ColliderComponent* colliderCustomer1 = ecs->AddComponents<ColliderComponent>(Customer1);
    colliderCustomer1->SetAsBox({ 1, 3, 1 });

    Customer1->transform.SetWorldScale({ 1.f, 1.f, 1.f });
    Customer1->transform.SetWorldPosition({ 0, -0.5, 10 });
    Customer1->transform.WorldRotate({ 0, 3.14, 0});
    MaterialComponent* matCustom = ecs->AddComponents<MaterialComponent>(Customer1);
    matCustom->LoadTexture("../../res/wojaknpc_dif_txt.png");

	//------Sushi------
    Sushi = ECS_ECS->CreateEntity();
    PlayerId = Sushi->GetId();
    MeshComponent* meshSushi = ecs->AddComponents<MeshComponent>(Sushi);
    meshSushi->LoadMesh("../../res/sushiBody.obj");


    RigidBodyComponent* rb3 = ecs->AddComponents<RigidBodyComponent>(Sushi);
    rb2->m_motionType = MotionType::Static;


    ColliderComponent* collider3 = ecs->AddComponents<ColliderComponent>(Sushi);
    collider3->SetAsBox({ 1.f,1.25f, 1.f });

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
    Entity* cam1 = ECS_ECS->GetComponents<CameraComponent>()[0]->GetOwner();
    Sushi->AddChild(cam1);
    cam1->transform.SetLocalPosition({ 0.f, 3.5f, -1.5f }); // hauteur des yeux
    cam1->transform.SetLocalRotation({ 0.f, 0.f, 0.f });   // regard droit devant

    Entity* cam2 = Camera::Camera(0.73f, 0.73f, 0.25f, 0.25f, 1);
    cam2->transform.SetWorldPosition({ 0.0f, 5.0f, -10.0f });
    cam2->transform.LookAt(Sushi->transform.GetWorldPosition());

    cameras.push_back(cam1);
    cameras.push_back(cam2);
    cam1->AddChild(cam2);

	Sushi->AddChild(cam1);
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

    Inputs::LockMouse(true);
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

        // Lecture souris en permanence
        DirectX::XMFLOAT2 mouseDelta = Inputs::GetMouseDelta();
        float dx = XMConvertToRadians(0.10f * mouseDelta.x); // réduis si trop rapide
        float dy = XMConvertToRadians(0.10f * mouseDelta.y);

        // Souris X ? tourne Sushi (yaw) ? la cam suit automatiquement car enfant
        Sushi->transform.WorldRotate({ 0.f, dx, 0.f });

        // Souris Y ? pitch local de la caméra uniquement
        phi += dy;
        if (phi < -XM_PIDIV2 + 0.1f) phi = -XM_PIDIV2 + 0.1f;
        if (phi > XM_PIDIV2 - 0.1f)  phi = XM_PIDIV2 - 0.1f;
        cameras[0]->transform.SetLocalRotation({ phi, 0.f, 0.f });

        // Touches fléchées
        if (Inputs::IsKeyPressed(Keyboard::LEFT_ARROW))
            Sushi->transform.WorldRotate({ 0, -rotSpeed, 0 });
        if (Inputs::IsKeyPressed(Keyboard::RIGHT_ARROW))
            Sushi->transform.WorldRotate({ 0, rotSpeed, 0 });

        // Déplacement
        if (Inputs::IsKeyPressed(Keyboard::Z))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalForward() * dt);
        if (Inputs::IsKeyPressed(Keyboard::S))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalBackward() * dt);
        if (Inputs::IsKeyPressed(Keyboard::Q))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalLeft() * dt);
        if (Inputs::IsKeyPressed(Keyboard::D))
            Sushi->transform.WorldTranslate(Sushi->transform.GetLocalRight() * dt);
    }

    // Mort / replay
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

    // Shaders de debug
    if (Inputs::IsKeyDown(Keyboard::F1)) ECS_ENGINE->DrawSolidShader();
    if (Inputs::IsKeyDown(Keyboard::F2)) ECS_ENGINE->DrawWireframeShader();
    if (Inputs::IsKeyDown(Keyboard::F5)) ECS_ENGINE->DrawPostProcessShader();

    if (Inputs::IsKeyDown(Keyboard::SPACE))
    {
		Sushi->transform.WorldTranslate({ 0.f, 5, 0.f });

    }



    // --- Spawn Plat (P) ---
    if (Inputs::IsKeyDown(Keyboard::P))
    {
        XMFLOAT3 spawnPos = Sushi->transform.GetWorldPosition();
        spawnPos.z += 1.5f; // spawn devant le joueur

        Entity* plat = ECS_ECS->CreateEntity();
        plat->transform.SetWorldPosition(spawnPos);
        plat->transform.SetWorldScale({ 0.5f, 0.1f, 0.5f });
        plat->SetName("Plat");

        MeshComponent* mPlat = ECS_ECS->AddComponents<MeshComponent>(plat);
        mPlat->LoadMesh("cube");

        MaterialComponent* matPlat = ECS_ECS->AddComponents<MaterialComponent>(plat);
        matPlat->SetColor({ 0.9f, 0.9f, 0.9f, 1.f }); // blanc = plat vide

        RigidBodyComponent* rbPlat = ECS_ECS->AddComponents<RigidBodyComponent>(plat);
        rbPlat->m_motionType = MotionType::Static;
        rbPlat->m_useGravity = true;

        ColliderComponent* colPlat = ECS_ECS->AddComponents<ColliderComponent>(plat);
        colPlat->SetAsBox({ 0.5f, 0.1f, 0.5f });

        ScriptComponent* scPlat = ECS_ECS->AddComponents<ScriptComponent>(plat);
        scPlat->SetScript<PlatScript>();

        std::cout << "[Spawn] Plat créé." << std::endl;
    }

    // --- Spawn Bun (1) ---
    if (Inputs::IsKeyDown(Keyboard::NUMPAD1))
    {
        XMFLOAT3 spawnPos = Sushi->transform.GetWorldPosition();
        spawnPos.y += 1.f;

        Entity* bun = ECS_ECS->CreateEntity();
        bun->transform.SetWorldPosition(spawnPos);
        bun->transform.SetWorldScale({ 0.3f, 0.3f, 0.3f });

        MeshComponent* mBun = ECS_ECS->AddComponents<MeshComponent>(bun);
        mBun->LoadMesh("sphere");

        MaterialComponent* matBun = ECS_ECS->AddComponents<MaterialComponent>(bun);
        matBun->SetColor({ 0.85f, 0.55f, 0.1f, 1.f }); // marron = bun

        RigidBodyComponent* rbBun = ECS_ECS->AddComponents<RigidBodyComponent>(bun);
        rbBun->m_motionType = MotionType::Dynamic;
        rbBun->m_useGravity = true;

        ColliderComponent* colBun = ECS_ECS->AddComponents<ColliderComponent>(bun);
        colBun->SetAsBox({ 0.3f, 0.15f, 0.3f });

        ScriptComponent* scBun = ECS_ECS->AddComponents<ScriptComponent>(bun);
        scBun->SetScript<IngredientScript>();
        IngredientScript* ingBun = static_cast<IngredientScript*>(scBun->m_instance);
        ingBun->isBuns = true;

        std::cout << "[Spawn] Bun créé." << std::endl;
    }

    // --- Spawn Salad (2) ---
    if (Inputs::IsKeyDown(Keyboard::NUMPAD2))
    {
        XMFLOAT3 spawnPos = Sushi->transform.GetWorldPosition();
        spawnPos.y += 1.f;

        Entity* salad = ECS_ECS->CreateEntity();
        salad->transform.SetWorldPosition(spawnPos);
        salad->transform.SetWorldScale({ 0.3f, 0.15f, 0.3f });

        MeshComponent* mSalad = ECS_ECS->AddComponents<MeshComponent>(salad);
        mSalad->LoadMesh("cube");

        MaterialComponent* matSalad = ECS_ECS->AddComponents<MaterialComponent>(salad);
        matSalad->SetColor({ 0.2f, 0.75f, 0.2f, 1.f }); // vert = salade

        RigidBodyComponent* rbSalad = ECS_ECS->AddComponents<RigidBodyComponent>(salad);
        rbSalad->m_motionType = MotionType::Dynamic;
        rbSalad->m_useGravity = true;

        ColliderComponent* colSalad = ECS_ECS->AddComponents<ColliderComponent>(salad);
        colSalad->SetAsBox({ 0.3f, 0.15f, 0.3f });

        ScriptComponent* scSalad = ECS_ECS->AddComponents<ScriptComponent>(salad);
        scSalad->SetScript<IngredientScript>();
        IngredientScript* ingSalad = static_cast<IngredientScript*>(scSalad->m_instance);
        ingSalad->isSalad = true;

        std::cout << "[Spawn] Salad créée." << std::endl;
    }

    // --- Spawn Fish (3) ---
    if (Inputs::IsKeyDown(Keyboard::NUMPAD3))
    {
        XMFLOAT3 spawnPos = Sushi->transform.GetWorldPosition();
        spawnPos.y += 1.f;

        Entity* fish = ECS_ECS->CreateEntity();
        fish->transform.SetWorldPosition(spawnPos);
        fish->transform.SetWorldScale({ 0.4f, 0.15f, 0.2f });

        MeshComponent* mFish = ECS_ECS->AddComponents<MeshComponent>(fish);
        mFish->LoadMesh("cube");

        MaterialComponent* matFish = ECS_ECS->AddComponents<MaterialComponent>(fish);
        matFish->SetColor({ 1.f, 0.3f, 0.3f, 1.f }); // rouge = fish

        RigidBodyComponent* rbFish = ECS_ECS->AddComponents<RigidBodyComponent>(fish);
        rbFish->m_motionType = MotionType::Dynamic;
        rbFish->m_useGravity = true;
        ColliderComponent* colFish = ECS_ECS->AddComponents<ColliderComponent>(fish);
        colFish->SetAsBox({ 0.4f, 0.15f, 0.2f });

        ScriptComponent* scFish = ECS_ECS->AddComponents<ScriptComponent>(fish);
        scFish->SetScript<IngredientScript>();
        IngredientScript* ingFish = static_cast<IngredientScript*>(scFish->m_instance);
        ingFish->isFish = true;

        std::cout << "[Spawn] Fish créé." << std::endl;
    }
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

    // Slow motion
    if (Inputs::IsKeyDown(Keyboard::T))
        ECS_APP->GetTimer().ToggleSlowTimeDown();
}