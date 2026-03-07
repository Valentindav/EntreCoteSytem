#include "App.h"

#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"
#include "Components/UiImageComponent.h"
#include "Systems/CameraSystem.h"
#include "Script.h"
#include "Entity.h"
#include "Inputs.h"
#include "_engine.h"
#include "Components/UiButtonComponent.h"

using namespace DirectX;

START_SCRIPT(SpinScript, Flag::Start | Flag::Update | Flag::Destroy)
public:
    float rotationSpeed = 1.0f;

protected:
    void Start() override
    {
        for (auto& ent : GetOwners())
            std::cout << "SpinScript Started on Entity ID: " << ent.second->GetId() << std::endl;
    }

    void Update(const GameTimer& gt) override
    {
        float rot = rotationSpeed * gt.DeltaTime();

        if (!GetOwners().empty())
        {
            for (auto& ent : GetOwners())
                ent.second->transform.LocalRotate({ rot, rot, 0.0f });
        }
    }

    void Destroy() override
    {
        std::cout << "Entity destroyed!" << std::endl;
    }

    END_SCRIPT

        App::App()
    {
    }

    App::~App()
    {
    }

    void App::CreateUI()
    {
        Window* win = ECS_ENGINE->GetWindow();

        ECS* ecs = ECS_ECS;

        Entity* entityUI1 = ecs->CreateEntity();
        Entity* entityUI2 = ecs->CreateEntity();

        const wchar_t* text = L"../../res/cat.dds";
        const wchar_t* text2 = L"../../res/Button.dds";

        UiImageComponent* uiImage1 = ecs->AddComponents<UiImageComponent>(entityUI1);
        UiButtonComponent* uiImage2 = ecs->AddComponents<UiButtonComponent>(entityUI2);


        uiImage1->InitImageComponent(0.f, 0.f, text, 0.1f, 0.1f);

        uiImage2->InitButtonComponent(0.5, 0.5, text, text2, 0.3, 0.2);
        uiImage2->AddListener([]() {
            MessageBoxA(nullptr, "Button Clicked!", "Info", MB_OK);
            });

        Entity* entityText1 = ecs->CreateEntity();

        UiTextComponent* Text1 = ecs->AddComponents<UiTextComponent>(entityText1);

        Text1->InitTextComponent(0.5f, 0.2f, "-WORLD");

        Entity* entityText2 = ecs->CreateEntity();

        UiTextComponent* Text2 = ecs->AddComponents<UiTextComponent>(entityText2);

        Text2->InitTextComponent(0.1, 0.2, "Hello");
    }

    void App::CreateParticles()
    {
        auto& renderer = ECS_ENGINE->GetRenderer();

        // L'emetteur 0 est cree automatiquement par Renderer::Initialize.
        // On le reconfigure ici (feu orange qui suit le cube).
        ParticleEmitterDesc fire;

        fire.spawnRadius = 0.0001;
        fire.name = "Fire";
        fire.gravity = 0.5f;   // monte
        fire.speed = 0.5f;
        fire.spread = 1.0001f;
        fire.minLife = 0.8f;
        fire.maxLife = 2.5f;
        fire.sizeStart = 0.08f;
        fire.sizeEnd = 0.02f;
        fire.colorStartR = 1.0f;  fire.colorStartG = 0.6f;  fire.colorStartB = 0.0f;  fire.colorStartA = 1.0f;
        fire.colorEndR = 1.0f;  fire.colorEndG = 0.0f;  fire.colorEndB = 0.0f;  fire.colorEndA = 0.0f;
        fire.maxParticles = 100000;
        fire.emitRate = 10000;

        m_fire = EmitterManager::GetInstance()->AddEmitter(fire, *renderer);

        ParticleEmitterDesc water;
        water.name = "Water";
        water.gravity = 9.8f;       // tombe vers le bas
        water.speed = 3.0f;
        water.spread = 0.4f;        // légèrement étalé
        water.minLife = 0.5f;
        water.maxLife = 1.5f;
        water.sizeStart = 0.04f;
        water.sizeEnd = 0.01f;
        water.colorStartR = 0.3f;  water.colorStartG = 0.7f;  water.colorStartB = 1.0f;  water.colorStartA = 0.9f;
        water.colorEndR = 0.1f;  water.colorEndG = 0.4f;  water.colorEndB = 0.9f;  water.colorEndA = 0.0f;
        water.maxParticles = 100000;
        water.emitRate = 10000;
        m_water = EmitterManager::GetInstance()->AddEmitter(water, *renderer);
    }

    void App::CreateCamera()
    {
        Window* win = ECS_ENGINE->GetWindow();

        ECS* ecs = ECS_ECS;
        ///-----------------CAMERA1------------------
        float x = radius * sinf(phi) * cosf(theta);
        float z = radius * sinf(phi) * sinf(theta);
        float y = radius * cosf(phi);

        camera1 = ecs->CreateEntity();
        int od = camera1->GetId();
        camera1->transform.SetWorldPosition({ x,y, z });

        CameraComponent* camComp = ecs->AddComponents<CameraComponent>(camera1);
       /* ///-----------------CAMERA2-----------------
        camera2 = ecs->CreateEntity();
        camera2->transform.SetWorldPosition({ 0.0f, 3.0f, -20.0f });
        entity1->AddChild(camera2);

        CameraComponent* camComp2 = ecs->AddComponents<CameraComponent>(camera2);*/

        CurrentCamera = camera1;
    }

    void App::CreateEnt()
    {
        Window* win = ECS_ENGINE->GetWindow();

        ECS* ecs = ECS_ECS;

        entity1 = ecs->CreateEntity();
        entity1->transform.SetWorldPosition({ 0.0f, 0.5f, 0.0f });

        MeshComponent* mesh1 = ecs->AddComponents<MeshComponent>(entity1);

        mesh1->LoadMesh("cube");
        mesh1->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // Rouge

        SpinScript* spinScript = new SpinScript();
        spinScript->AddOwner(entity1);
        spinScript->Init();

        entity2 = ecs->CreateEntity();
        entity2->transform.SetWorldPosition({ 2.0f, 1.0f, 0.0f });

        MeshComponent* mesh2 = ecs->AddComponents<MeshComponent>(entity2);
        mesh2->LoadMesh("sphere");
        mesh2->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });

        ground = ecs->CreateEntity();
        ground->transform.SetWorldPosition({ 0.0f, -1.0f, 0.0f });
        ground->transform.SetWorldScale({ 1.0f, 1.f, 1.0f });

        MeshComponent* meshGround = ecs->AddComponents<MeshComponent>(ground);
        meshGround->LoadMesh("cylinder");
        meshGround->SetColor({ 0.3f, 0.3f, 0.3f, 1.0f });
    }

    void App::Init()
    {
        //-----------------UI------------------
        CreateUI();
        CreateParticles();

        //------------------ENTITIES-----------
        CreateEnt();

        //-----------------CAMERA--------------
        CreateCamera();


        camera1->GetComponent<CameraComponent>()->m_isActive = true;
    }

    void App::Update(const GameTimer& gt)
    {
        if (ECS_ECS->GetEntities().empty()) return; 
        else if (entity1 == nullptr) {
            entity1 = ECS_ECS->GetEntities()[0];
        }

        if (entity1)
        {
            XMFLOAT3 pos = entity1->transform.GetWorldPosition();
            m_fire->SetEmitterOrigin( pos.x, pos.y , pos.z);
        }

        if (entity2)
        {
            XMFLOAT3 pos = entity2->transform.GetWorldPosition();
            m_water->SetEmitterOrigin( pos.x, pos.y , pos.z);
        }

        // P = eteindre les particules
        if (Inputs::IsKeyDown(Keyboard::P))
            m_fire->SetEmitterEnabled( false);
        if (Inputs::IsKeyDown(Keyboard::O))
            m_fire->SetEmitterEnabled( true);



        float dt = gt.DeltaTime();

        float x = radius * sinf(phi) * cosf(theta);
        float z = radius * sinf(phi) * sinf(theta);
        float y = radius * cosf(phi);

        camera1->transform.SetWorldPosition({ x,y,z });
        if (CurrentCamera && entity1 != CurrentCamera && entity1)
        {
            CameraComponent* comp = CurrentCamera->GetComponent<CameraComponent>();
            if (comp)
                comp->m_renderCamera->LookAt(
                    { CurrentCamera->transform.GetWorldPosition() },       // Position
                    { entity1->transform.GetWorldPosition() }, // Target
                    { 0.0f, 1.0f, 0.0f }  // Up
                );
        }

        CheckInput(gt);
    }

    void App::CheckInput(const GameTimer& gt)
    {
        float dt = gt.DeltaTime();

        if (entity1)
        {
            float rotSpeed = m_speed * dt;

            if (Inputs::IsKeyPressed(Keyboard::Q))
                CurrentCamera->transform.WorldRotate({ 0, -rotSpeed, 0 });

            if (Inputs::IsKeyPressed(Keyboard::D))
                CurrentCamera->transform.WorldRotate({ 0, rotSpeed, 0 });

            if (Inputs::IsKeyPressed(Keyboard::Z))
            {
                entity1->transform.WorldTranslate(entity1->transform.GetLocalForward()* gt.DeltaTime());
            }
            if (Inputs::IsKeyPressed(Keyboard::S))
            {
                entity1->transform.WorldTranslate(entity1->transform.GetLocalBackward() * gt.DeltaTime());
            }


            // Rotation Cube

            if (Inputs::IsKeyDown(Keyboard::M)) {
                if (CurrentCamera == camera1)
                {
                    camera2->GetComponent<CameraComponent>()->m_isActive = true;
                    camera1->GetComponent<CameraComponent>()->m_isActive = false;
                    CurrentCamera = camera2;
                }
                else {
                    camera1->GetComponent<CameraComponent>()->m_isActive = true;
                    camera2->GetComponent<CameraComponent>()->m_isActive = false;
                    CurrentCamera = camera1;
                }
            }

            if (Inputs::IsKeyPressed(Keyboard::LEFT_ARROW))
                entity1->transform.WorldRotate({ 0, rotSpeed, 0 });

            if (Inputs::IsKeyPressed(Keyboard::RIGHT_ARROW))
                entity1->transform.WorldRotate({ 0, -rotSpeed, 0 });

            if (Inputs::IsKeyPressed(Keyboard::UP_ARROW))
                entity1->transform.LocalRotate({ rotSpeed, 0, 0 });

            if (Inputs::IsKeyPressed(Keyboard::DOWN_ARROW))
                entity1->transform.LocalRotate({ -rotSpeed, 0, 0 });

            // Suppression Cube

            if (Inputs::IsKeyDown(Keyboard::SPACE))
                ECS_ECS->DestroyEntity(entity1->GetId());

            // Changement de mode de rendu 
            // TODO : pas cencé être dans l'app, mais dans ecsEngine. Plutôt directement dans EngineApp? Une class Debug?

            if (Inputs::IsKeyDown(Keyboard::F1))
            {
                ECS_ENGINE->DrawSolidShader();
            }
            if (Inputs::IsKeyDown(Keyboard::F2))
            {
                ECS_ENGINE->DrawWireframeShader();
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
    }