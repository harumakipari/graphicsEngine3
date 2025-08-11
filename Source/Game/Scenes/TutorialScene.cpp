#include "TutorialScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Utility/Win32Utils.h"
#include "Engine/Utility/Timer.h"
#include "Core/ActorManager.h"


#include "Physics/Physics.h"
#include "Physics/PhysicsUtility.h"
#include "Physics/CollisionSystem.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/GameUIFactory.h"
#include "Widgets/TutorialUIFactory.h"
#include "Widgets/PauseUIFactory.h"

#include "Game/Managers/TutorialSystem.h"
#include "Game/Actors/Enemy/TutorialEnemy.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Camera/Camera.h"
#include "Game/Actors/Stage/Building.h"

#include "Components/Transform/Transform.h"


bool TutorialScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    HRESULT hr;

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(sceneConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[0].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    bufferDesc.ByteWidth = sizeof(shaderConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[1].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    bufferDesc.ByteWidth = sizeof(fogConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[2].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    sceneConstants.time = 0;//開始時に０にしておく

    // FOG 
    framebuffers[0] = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, true);
    CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", pixelShaders[2].GetAddressOf());

    //スカイマップ
    //skyMap = std::make_unique<decltype(skyMap)::element_type >(device, L"./Data/Environment/Sky/winter_evening_4k.DDS");
    skyMap = std::make_unique<decltype(skyMap)::element_type >(device, L"./Data/Environment/Sky/cloud/skybox.dds");

    fullscreenQuadTransfer = std::make_unique<FullScreenQuad>(device);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, static_cast<uint32_t>(width), height, 3);

    //ブルーム
    bloomer = std::make_unique<Bloom>(device, static_cast<uint32_t>(width), height);
    CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", pixelShaders[0].ReleaseAndGetAddressOf());

    //CascadedShadpwMaps
    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    D3D11_TEXTURE2D_DESC texture2dDesc;
    //テクスチャをロード
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_charlie.dds", shaderResourceViews[0].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/diffuse_iem.dds", shaderResourceViews[1].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/specular_pmrem.dds", shaderResourceViews[2].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_ggx.dds", shaderResourceViews[3].ReleaseAndGetAddressOf(), &texture2dDesc);

    //splash = std::make_unique<Sprite>(device, L"./Data/Textures/Screens/TitleScene/994759-1.jpg");

    tutorialItemCounter = 0;

    Physics::Instance().Initialize();

    //// 最終の描画
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, true);

    //アクターをセット
    SetUpActors();

    effectSystem = std::make_unique<EffectSystem>();
    effectSystem->Initialize();

    // ビル生成音
    audio = std::make_unique<StandaloneAudioSource>(L"./Data/Sound/SE/bill_spawn.wav");
    audio->SetVolume(2.0f);

    //renderState = std::make_unique<decltype(renderState)::element_type>(device);
    EventSystem::Initialize();//追加 UI
    

    return true;
}

void TutorialScene::Start()
{
    
    GameUIFactory::Create(this, false);
    TutorialUIFactory::Create(this);
    PauseUIFactory::Create(this, true);

    //チュートリアルシステム初期化
    TutorialSystem::Initialize();
    startTimer = 2.0f;
    TutorialSystem::SetInitializeFunction(TutorialStep::Move, [&]()
        {
            //Moveテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialMove")->SetActive(true);
        });
    TutorialSystem::SetInitializeFunction(TutorialStep::Collect, [&]()
        {
            Transform transform{ DirectX::XMFLOAT3{2.0f,0.175f,-3.0f},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
            auto item = GetActorManager()->CreateAndRegisterActorWithTransform<PickUpItem>("tutorialItem", transform);

            //Collectテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialCollect")->SetActive(true);
        });

    TutorialSystem::SetInitializeFunction(TutorialStep::FirstAttack, [&]()
        {
            //FirstAttackテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialFirstAttack")->SetActive(true);
        });

    TutorialSystem::SetInitializeFunction(TutorialStep::ManyCollect, [&]()
        {
            const DirectX::XMFLOAT3 center{ 0.0f, 0.175f, 0.0f };
            const float minRadius = 4.0f;
            const float maxRadius = 8.0f;

            for (int i = 0; i < 20; i++)
            {
                float rand1 = static_cast<float>(rand()) / RAND_MAX;
                float rand2 = static_cast<float>(rand()) / RAND_MAX;

                float dist = sqrtf(rand1 * (maxRadius * maxRadius - minRadius * minRadius) + minRadius * minRadius);

                float angle = rand2 * 2.0f * DirectX::XM_PI;

                float x = center.x + dist * cosf(angle);
                float z = center.z + dist * sinf(angle);
                float y = center.y;

                Transform transform
                {
                    DirectX::XMFLOAT3{x, y, z},
                    DirectX::XMFLOAT4{0.0f, 0.0f, 0.0f, 1.0f},
                    DirectX::XMFLOAT3{1.0f, 1.0f, 1.0f}
                };
                auto item = GetActorManager()->CreateAndRegisterActorWithTransform<PickUpItem>("tutorialItem", transform);

            }

            //ManyCollectテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialManyCollect")->SetActive(true);
            objectManager.FindGameObject("TutorialNum3")->SetActive(true);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::ManyCollect, [&]()
        {
            player->SetState(Player::State::Idle);
        });
    TutorialSystem::SetInitializeFunction(TutorialStep::SecondAttack, [&]()
        {
            player->SetState(Player::State::Idle);
            //ビーム攻撃ミッションテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialSecondAttack")->SetActive(true);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::SecondAttack, [&]()
        {
            //Info1テキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialInfo1")->SetActive(true);
        });
    TutorialSystem::SetInitializeFunction(TutorialStep::CreateBuild, [&]()
        {
            if (!GetActorManager()->GetActorByName("tutorialBuilding"))
            {
                Transform transform{ DirectX::XMFLOAT3{-4.0f,-3.0f,1.0f},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
                auto build = GetActorManager()->CreateAndRegisterActorWithTransform<Building>("tutorialBuilding", transform);
                build->preSkeltalMeshComponent->SetIsVisible(true);
                audio->Play();
            }
            float x = MathHelper::RandomRange(3.0f, 6.0f);
            float z = MathHelper::RandomRange(-3.0f, 3.0f);

            // 名前を連番でユニークにする
            std::string itemName = "tutorialItem__" + std::to_string(tutorialItemCounter++);
            Transform itemTransform{ DirectX::XMFLOAT3{x,0.175f,-z},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
            //Transform itemTransform{ DirectX::XMFLOAT3{5.0f,0.175f,-3.0f},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
            auto item = GetActorManager()->CreateAndRegisterActorWithTransform<PickUpItem>(itemName, itemTransform);

            //照準ミッションテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialAim")->SetActive(true);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::CreateBuild, [&]()
        {
            //Info2テキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialInfo2")->SetActive(true);
        });
    TutorialSystem::SetInitializeFunction(TutorialStep::ManyCollect2, [&]()
        {
            const DirectX::XMFLOAT3 center{ 0.0f, 0.175f, 0.0f };
            const float minRadius = 4.0f;
            const float maxRadius = 8.0f;

            for (int i = 0; i < 5; i++)
            {
                float rand1 = static_cast<float>(rand()) / RAND_MAX;
                float rand2 = static_cast<float>(rand()) / RAND_MAX;

                float dist = sqrtf(rand1 * (maxRadius * maxRadius - minRadius * minRadius) + minRadius * minRadius);

                float angle = rand2 * -DirectX::XM_PI * (1.0f / 3.0f);

                float x = center.x + dist * cosf(angle);
                float z = center.z + dist * sinf(angle);
                float y = center.y;

                Transform transform{ DirectX::XMFLOAT3{x,y,z},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
                auto item = GetActorManager()->CreateAndRegisterActorWithTransform<PickUpItem>("tutorialItem", transform);
            }
            if (!GetActorManager()->GetActorByName("tutorialBuildingManyCollect2"))
            {
                Transform transform{ DirectX::XMFLOAT3{-6.5f,-3.0f,1.0f},DirectX::XMFLOAT4{0.0f,0.0f,0.0f,1.0f},DirectX::XMFLOAT3{1.0f,1.0f,1.0f} };
                auto build = GetActorManager()->CreateAndRegisterActorWithTransform<Building>("tutorialBuildingManyCollect2", transform);
                build->preSkeltalMeshComponent->SetIsVisible(true);
                audio->Play();
            }
            //破壊ミッションテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialBreakAttack")->SetActive(true);
            objectManager.FindGameObject("TutorialNum9")->SetActive(true);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::ManyCollect2, [&]()
        {
            player->SetState(Player::State::Idle);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::BreakBuilds, [&]()
        {
            //Info4テキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialInfo4")->SetActive(true);
        });
    TutorialSystem::SetInitializeFunction(TutorialStep::BossBuild, [&]()
        {
            //Info3テキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("TutorialInfo3")->SetActive(true);
        });
    TutorialSystem::SetCompletedFunction(TutorialStep::BossBuild, [&]()
        {
            //BattleStartテキストだけ表示
            for (GameObject* object : objectManager.FindGameObject("TutorialCanvas")->children)
            {
                object->SetActive(false);
            }
            objectManager.FindGameObject("BattleStart")->SetActive(true);
        });

    TutorialSystem::SetInitializeFunction(TutorialStep::Finish, [&]()
        {
            EasingHandler handler;
            handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.5f);
            handler.SetCompletedFunction([]() {
                const char* types[] = { "0", "1" };
                Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
                });
            GameObject* fadeCanvas = ObjectManager::Find("FadeCanvas");
            fadeCanvas->SetActive(true);
            fadeCanvas->GetComponent<EasingComponent>()->StartHandler(handler);
        });

    //チュートリアルBGM
    GameObject* tutorialBgm = UIFactory::Create("TutorialBGM");
    AudioSource* tutorialSource = tutorialBgm->AddComponent<AudioSource>(L"./Data/Sound/BGM/tutorial.wav");
    tutorialSource->SetVolume(0.2f);
    tutorialSource->Play(XAUDIO2_LOOP_INFINITE);
}

void TutorialScene::Update(ID3D11DeviceContext* immediate_context, float deltaTime)
{
    auto camera = std::dynamic_pointer_cast<MainCamera>(GetActorManager()->GetActorByName("mainCameraActor"));

    TutorialSystem::Update(deltaTime);
    Physics::Instance().Update(deltaTime);
    //ActorManager::Update(deltaTime);
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

    //最初のタイマー(仮)
    if (TutorialSystem::GetCurrentStep() == TutorialStep::Start)
    {
        startTimer -= deltaTime;
        if (startTimer <= 0.0f)
        {
            TutorialSystem::AchievedAction(TutorialStep::Start);
        }
    }

    effectSystem->Update(deltaTime);

    //if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    //{
    //    const char* types[] = { "0", "1" };
    //    Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
    //    //TODO:01	ここをどう変えたらいいかわからない
    //}

    //if (InputSystem::GetInputState("Q", InputStateMask::Trigger))
    //{
    //    const char* types[] = { "0", "1" };
    //    //TODO:01	ここをどう変えたらいいかわからない
    //    Scene::_transition("LoadingScene", { std::make_pair("preload", "BootScene"), std::make_pair("type", types[rand() % 2]) });
    //}

    if (TutorialSystem::GetCurrentStep() >= TutorialStep::MoveCamera)
    {
        float lerpCameraTime = 2.0f;
        DirectX::XMFLOAT3 pos = player->GetPosition();
        DirectX::XMFLOAT3 enemyPos = enemy->GetPosition();
        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&pos);
        DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&enemyPos);
        elapsedTime += deltaTime;
        float t = std::clamp((elapsedTime / lerpCameraTime), 0.0f, 1.0f);
        DirectX::XMVECTOR Target = DirectX::XMVectorLerp(Pos, EnemyPos, t);
        DirectX::XMStoreFloat3(&target, Target);
        camera->SetTarget(target);
        if (t >= 1.0f)
        {
            TutorialSystem::AchievedAction(TutorialStep::MoveCamera);
        }
        
        if (enemy->IsFinishBuildAnimation())
        {
            camera->SetTarget(player->GetPosition());
        }
    }
    else
    {
        camera->SetTarget(player->GetPosition());
    }

    if (TutorialSystem::GetCurrentStep() == TutorialStep::Start)
    {
        // 動けなくする
        player->SetState(Player::State::StartCharge);
    }
    else if(TutorialSystem::GetCurrentStep() == TutorialStep::Move)
    {
        player->SetState(Player::State::Idle);
    }

}

void TutorialScene::SetUpActors()
{
    stage = GetActorManager()->CreateAndRegisterActor<Stage>("stage");

    Transform playerTr(DirectX::XMFLOAT3{ 0.7f,0.8f,-9.5f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    player = GetActorManager()->CreateAndRegisterActorWithTransform<Player>("actor", playerTr);

    auto mainCameraActor = GetActorManager()->CreateAndRegisterActor<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();

    CameraManager::SetGameCamera(mainCameraActor.get());

    auto debugCameraActor = GetActorManager()->CreateAndRegisterActor<DebugCamera>("debugCam");
    //auto debugCameraActor = ActorManager::CreateAndRegisterActor<Actor>("debugCam");
    //auto debugCamera = debugCameraActor->NewSceneComponent<DebugCameraComponent>("debugCamera");
    CameraManager::SetDebugCamera(debugCameraActor);

    Transform enemyTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,-1.0f });
    enemy = GetActorManager()->CreateAndRegisterActorWithTransform<TutorialEnemy>("tutorialEnemy", enemyTr);
}

bool TutorialScene::Uninitialize(ID3D11Device* device)
{
    //ActorManager::ClearAll();
    Physics::Instance().Finalize();
    PhysicsTest::Instance().Finalize();
    //SpawnValidator::Clear();    // 登録していた Box を破棄する
    return true;
}

void TutorialScene::Render(ID3D11DeviceContext* immediateContext, float delta_time)
{
    //サンプラーステートを設定
    RenderState::BindSamplerStates(immediateContext);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    //IBL
    immediateContext->PSSetShaderResources(32, 1, shaderResourceViews[0].GetAddressOf());
    immediateContext->PSSetShaderResources(33, 1, shaderResourceViews[1].GetAddressOf());
    immediateContext->PSSetShaderResources(34, 1, shaderResourceViews[2].GetAddressOf());
    immediateContext->PSSetShaderResources(35, 1, shaderResourceViews[3].GetAddressOf());


    D3D11_VIEWPORT viewport;
    UINT num_viewports{ 1 };
    immediateContext->RSGetViewports(&num_viewports, &viewport);

    float aspect_ratio{ viewport.Width / viewport.Height };

    auto camera = CameraManager::GetCurrentCamera();
    if (camera)
    {
        ViewConstants data = camera->GetViewConstants();
        sceneConstants.cameraPosition = data.cameraPosition;
        sceneConstants.view = data.view;
        sceneConstants.projection = data.projection;

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&data.projection);
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&data.view);
        DirectX::XMStoreFloat4x4(&sceneConstants.viewProjection, V * P);


        // CASCADED_SHADOW_MAPS
        DirectX::XMStoreFloat4x4(&sceneConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        DirectX::XMStoreFloat4x4(&sceneConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        // COMPUTE_PARTICLE_SYSTEM
        DirectX::XMStoreFloat4x4(&sceneConstants.invView, DirectX::XMMatrixInverse(NULL, V));
    }

    sceneConstants.lightDirection = lightDirection;
    sceneConstants.colorLight = colorLight;
    sceneConstants.iblIntensity = iblIntensity;
    // SCREEN_SPACE_AMBIENT_OCCLUSION
    sceneConstants.enableSSAO = enableSSAO;
    sceneConstants.enableBloom = enableBloom;
    sceneConstants.enableFog = enableFog;
    sceneConstants.enableCascadedShadowMaps = enableCascadedShadowMaps;
    sceneConstants.enableSSR = enableSSR;
    // SCREEN_SPACE_REFLECTION
    sceneConstants.reflectionIntensity = refrectionIntensity;
    // FOG
    sceneConstants.time += delta_time;

    immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &sceneConstants, 0, 0);
    immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->GSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->CSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());

    shaderConstants.maxDistance = maxDistance;
    shaderConstants.resolution = resolution;
    shaderConstants.steps = steps;
    shaderConstants.thickness = thickness;

    immediateContext->UpdateSubresource(constantBuffers[1].Get(), 0, 0, &shaderConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());

    immediateContext->UpdateSubresource(constantBuffers[2].Get(), 0, 0, &fogConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());    //3 は cascadedShadowMap に使用中

    //immediateContext->UpdateSubresource(constantBuffers[3].Get(), 0, 0, &projectionMappingConstants, 0, 0);
    //immediateContext->PSSetConstantBuffers(5, 1, constantBuffers[3].GetAddressOf());

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets->Clear(immediateContext);
    multipleRenderTargets->Acticate(immediateContext);

    // SKY_MAP
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    //skyMap->Blit(immediateContext, sceneConstants.viewProjection);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    RenderState::BindSamplerStates(immediateContext);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    //splash->Render(immediateContext, 0, 0, viewport.Width, viewport.Height);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    // MULTIPLE_RENDER_TARGETS
    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    actorRender.RenderOpaque(immediateContext);
    actorRender.RenderMask(immediateContext);
    actorRender.RenderBlend(immediateContext);
    actorRender.RenderInstanced(immediateContext);
    actorRender.RenderBuilding(immediateContext);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);

    // デバック描画
#if _DEBUG
    actorColliderManager.DebugRender(immediateContext);
    PhysicsTest::Instance().DebugRender(immediateContext);
    //GameManager::DebugRender(immediateContext);
#endif
    // 瓦礫
    {

    }

    effectSystem->Render(immediateContext);

    multipleRenderTargets->Deactivate(immediateContext);


    DirectX::XMFLOAT4X4 cameraView;
    DirectX::XMFLOAT4X4 cameraProjection;

    if (camera)
    {
        ViewConstants data = camera->GetViewConstants();
        cameraView = data.view;
        cameraProjection = data.projection;
#if 0
        cameraView = camera->GetView();
        cameraProjection = camera->GetProjection();

#endif // 0
    }
    // CASCADED_SHADOW_MAPS
    // Make cascaded shadow maps
    cascadedShadowMaps->Clear(immediateContext);
    cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightDirection, criticalDepthValue, 3/*cbSlot*/);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    actorRender.CastShadowRender(immediateContext);
    //gameWorld_->CastShadowRender(immediateContext);
    cascadedShadowMaps->Deactive(immediateContext);

    // FOG
    {
        framebuffers[0]->Clear(immediateContext, 0, 0, 0, 0);
        framebuffers[0]->Activate(immediateContext);


        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        ID3D11ShaderResourceView* shader_resource_views[]
        {
            multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
            multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
            cascadedShadowMaps->depthMap().Get(),   //cascaededShadowMaps
        };
        fullscreenQuadTransfer->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), pixelShaders[2]/*VolumetricFogPS*/.Get());

        framebuffers[0]->Deactivate(immediateContext);
    }

    // CASCADED_SHADOW_MAPS
    // Draw shadow to scene framebuffer
    // FINAL_PASS
    {
        //ブルーム
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        bloomer->make(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[0]);
        //bloomer->make(immediateContext, framebuffers[1]->shaderResourceViews[0].Get());

        ID3D11ShaderResourceView* shader_resource_views[]
        {
            // MULTIPLE_RENDER_TARGETS
            multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
            //framebuffers[1]->shaderResourceViews[0].Get(),
            multipleRenderTargets->renderTargetShaderResourceViews[1],
            multipleRenderTargets->renderTargetShaderResourceViews[2],
            multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
            bloomer->shader_resource_view(),    //bloom
            framebuffers[0]->shaderResourceViews[0].Get(),  //fog
            cascadedShadowMaps->depthMap().Get(),   //cascaededShadowMaps
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuadTransfer->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), pixelShaders[0]/*final pass*/.Get());

    }

    // UI描画
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        objectManager.Draw(immediateContext);
    }
}


void TutorialScene::DrawGui()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.08f, 0.15f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.1f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.15f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.2f, 0.4f, 0.8f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.15f, 0.25f, 0.9f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.25f, 0.4f, 1.0f);
    // ビューポートサイズ取得
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float screen_width = viewport->WorkSize.x;
    const float screen_height = viewport->WorkSize.y;

    const float left_panel_width = 300.0f;
    const float right_panel_width = 400.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 300.0f,
        viewport->WorkPos.y + viewport->WorkSize.y - 100.0f));
    ImGui::SetNextWindowBgAlpha(0.3f); // 半透明


    // ==== 左側：アウトライナー ====
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(left_panel_width, screen_height));
    ImGui::Begin("Actor Outliner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    for (auto& actor : GetActorManager()->allActors_) {
        bool is_selected = (selectedActor_ == actor);
        if (ImGui::Selectable(actor->GetName().c_str(), is_selected)) {
            selectedActor_ = actor;
        }
    }

    // ==== UIアウトライナ(追加)　====
    EditorGUI::DrawMainMenu();

    if (ImGui::TreeNodeEx("UI Outliner", ImGuiTreeNodeFlags_DefaultOpen))
    {
        objectManager.DrawHierarchy();

        ImGui::TreePop();
    }


    ImGui::End();

    // ==== 右側：インスペクタ ====
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + screen_width - right_panel_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(right_panel_width, screen_height));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // タブUI開始
    if (ImGui::BeginTabBar("InspectorTabs"))
    {
        // ===== Actorタブ =====
        if (ImGui::BeginTabItem("Actor"))
        {
            if (selectedActor_) {
                selectedActor_->DrawImGuiInspector();
            }
            else {
                ImGui::Text("No actor selected.");
            }
            ImGui::EndTabItem();
        }

        // ===== PostEffectタブ =====
        if (ImGui::BeginTabItem("PostEffect"))
        {
            // -------------------------
            // SSAO
            // -------------------------
            if (ImGui::Checkbox("Enable SSAO", &enableSSAO))
            {
                // SSAO有効切り替え時の処理があればここで
            }
#if 0
            if (enableSSAO && ImGui::TreeNode("SSAO Settings")) {
                ImGui::SliderFloat("Radius", &ssaoRadius, 0.1f, 10.0f);
                ImGui::SliderInt("Sample Count", &ssaoSamples, 4, 64);
                ImGui::TreePop();
            }
#endif

            // ========== SSR ==========
            if (ImGui::Checkbox("Enable SSR", &enableSSR)) {}
            if (enableSSR && ImGui::TreeNode("SSR Settings"))
            {
                ImGui::SliderFloat("Reflection Intensity", &refrectionIntensity, 0.0f, 1.0f);
                ImGui::SliderFloat("Max Distance", &maxDistance, 0.0f, 30.0f);
                ImGui::SliderFloat("Resolution", &resolution, 0.0f, 1.0f);
                ImGui::SliderInt("Steps", &steps, 0, 20);
                ImGui::SliderFloat("Thickness", &thickness, 0.0f, 1.0f);
                ImGui::TreePop();
            }

            // -------------------------
            // Bloom
            // -------------------------
            if (ImGui::Checkbox("Enable Bloom", &enableBloom))
            {
            }
            if (enableBloom && ImGui::TreeNode("Bloom Settings"))
            {
                ImGui::SliderFloat("Threshold", &bloomer->bloom_extraction_threshold, 0.0f, 5.0f);
                ImGui::SliderFloat("Intensity", &bloomer->bloom_intensity, 0.0f, 5.0f);
                ImGui::TreePop();
            }

            // -------------------------
            // Fog
            // -------------------------
            if (ImGui::Checkbox("Enable Fog", &enableFog))
            {
            }
            if (enableFog && ImGui::TreeNode("Fog Settings"))
            {
                ImGui::ColorEdit3("Fog Color", fogConstants.fogColor);
                ImGui::SliderFloat("Intensity", &(fogConstants.fogColor[3]), 0.0f, 10.0f);
                ImGui::SliderFloat("Density", &fogConstants.fogDensity, 0.0f, 0.05f, "%.6f");
                ImGui::SliderFloat("Height Falloff", &fogConstants.fogHeightFalloff, 0.001f, 1.0f, "%.4f");
                ImGui::SliderFloat("Cutoff Distance", &fogConstants.fogCutoffDistance, 0.0f, 1000.0f);
                ImGui::SliderFloat("Ground Level", &fogConstants.groundLevel, -100.0f, 100.0f);
                ImGui::SliderFloat("Mie Scattering", &fogConstants.mieScatteringCoef, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat("Time Scale", &fogConstants.timeScale, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat("Noise Scale", &fogConstants.noiseScale, 0.0f, 0.5f, "%.4f");
                ImGui::TreePop();
            }

#if 0
            // ========== Volumetric Cloudscapes ==========
            if (ImGui::Checkbox("Enable Volumetric Clouds", &enableVolumetricClouds)) {}
            if (enableVolumetricClouds && ImGui::TreeNode("Cloud Settings")) {
                ImGui::DragFloat4("Camera Focus", &cameraFocus.x, 0.5f);

                ImGui::DragFloat("Density Scale", &volumetricCloudscapes->constantData.densityScale, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Cloud Coverage", &volumetricCloudscapes->constantData.cloudCoverageScale, 0.001f, 0.0f, 0.5f);
                ImGui::DragFloat("Rain Absorption", &volumetricCloudscapes->constantData.rainCloudAbsorptionScale, 0.01f, 0.0f, 10.0f, "%.2f");
                ImGui::DragFloat("Cloud Type", &volumetricCloudscapes->constantData.cloudTypeScale, 0.01f, 0.0f, 10.0f, "%.2f");

                ImGui::DragFloat("LowFreq Perlin", &volumetricCloudscapes->constantData.lowFrequencyPerlinWorleySamplingScale, 0.000001f, 0.0f, 1.0f, "%.7f");
                ImGui::DragFloat("HighFreq Worley", &volumetricCloudscapes->constantData.highFrequencyWorleySamplingScale, 0.00001f, 0.0f, 1.0f, "%.5f");
                ImGui::DragFloat("Horizon Distance", &volumetricCloudscapes->constantData.horizonDistanceScale, 0.0001f, 0.0f, 1.0f, "%.4f");

                ImGui::SliderFloat2("Wind Direction", &volumetricCloudscapes->constantData.windDirection.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Wind Speed", &volumetricCloudscapes->constantData.windSpeed, 0.0f, 20.0f);

                ImGui::DragFloat("Earth Radius", &volumetricCloudscapes->constantData.earthRadius, 1.0f);
                ImGui::DragFloat("Cloud Altitude Min", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.x, 1.0f);
                ImGui::DragFloat("Cloud Altitude Max", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.y, 1.0f);

                ImGui::DragFloat("Long Distance Density", &volumetricCloudscapes->constantData.cloudDensityLongDistanceScale, 0.01f, 0.0f, 36.0f, "%.2f");
                ImGui::Checkbox("Powdered Sugar Effect", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.enablePowderedSugarEffect));

                ImGui::SliderInt("Ray Marching Steps", &volumetricCloudscapes->constantData.rayMarchingSteps, 1, 128);
                ImGui::Checkbox("Auto Ray Marching", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.autoRayMarchingSteps));

                ImGui::TreePop();
            }
#endif
            ImGui::EndTabItem();
        }

        // ===== SceneSettingsタブ（任意）=====
        if (ImGui::BeginTabItem("Scene"))
        {
            // -------------------------
            // Light Settings
            // -------------------------
            if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderFloat3("Light Direction", &lightDirection.x, -1.0f, 1.0f);
                ImGui::SliderFloat3("Light Color", &colorLight.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Light Intensity", &iblIntensity, 0.0f, 10.0f);
                ImGui::SliderFloat("Light Threshold", &colorLight.w, 0.0f, 10.0f);
            }

            // -------------------------
            // CSM (シャドウ関連)
            // -------------------------
            if (ImGui::CollapsingHeader("Cascaded Shadow Maps"))
            {
                ImGui::SliderFloat("Critical Depth", &criticalDepthValue, 0.0f, 1000.0f);
                ImGui::SliderFloat("Split Scheme", &cascadedShadowMaps->splitSchemeWeight, 0.0f, 1.0f);
                ImGui::SliderFloat("Z Mult", &cascadedShadowMaps->zMult, 1.0f, 100.0f);
                ImGui::Checkbox("Fit To Cascade", &cascadedShadowMaps->fitToCascade);
                ImGui::SliderFloat("Shadow Color", &shaderConstants.shadowColor, 0.0f, 1.0f);
                ImGui::DragFloat("Depth Bias", &shaderConstants.shadowDepthBias, 0.00001f, 0.0f, 0.01f, "%.8f");
                ImGui::Checkbox("Colorize Layer", &shaderConstants.colorizeCascadedlayer);
            }
            ImGui::EndTabItem();
        }

        // ==== UIタブ（追加） ====
        if (ImGui::BeginTabItem("UI"))
        {
            objectManager.DrawProperty();

            ImGui::Separator();
            ImGui::Text("EventSystem");
            ImGui::Separator();

            EventSystem::DrawProperty();

            ImGui::EndTabItem();
        }
        // ==== EffectSystemタブ（追加） ====
        if (ImGui::BeginTabItem("Effects"))
        {
            effectSystem->DrawGUI();

            ImGui::EndTabItem();
        }
        // ==== TutorialSystemタブ（追加） ====
        if (ImGui::BeginTabItem("Tutorial"))
        {
            TutorialSystem::DrawGUI();
            ImGui::EndTabItem();
        }
        //// ==== Itemタブ（追加） ====
        //if (ImGui::BeginTabItem("Item"))
        //{
        //	static bool isYoffsetFromTarget = true;
        //	ImGui::Checkbox("eye is yOffset from target", &isYoffsetFromTarget);

        //	uint32_t* enableMappings = &projectionMappingConstants.enableMapping[0].x;
        //	uint32_t* textureId = &projectionMappingConstants.textureId[0].x;
        //	for (int i = 0; i < MAX_PROJECTION_MAPPING; i++) {
        //		ImGui::PushID(i);

        //		if (ImGui::TreeNodeEx(("Projection" + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        //		{
        //			ImGui::Checkbox("enable", reinterpret_cast<bool*>(&enableMappings[i]));
        //			ImGui::SliderInt("texture Id", reinterpret_cast<int*>(&textureId[i]), 0, 1);

        //			ImGui::DragFloat3("target", &targets[i].x);

        //			if (!isYoffsetFromTarget) {
        //				ImGui::DragFloat3("eye", &eyes[i].x);
        //			}
        //			else {
        //				eyes[i] = targets[i];
        //			}
        //			ImGui::TreePop();
        //		}

        //		ImGui::PopID();
        //	}


        //	GameManager::DrawGUI();

        //	ImGui::EndTabItem();
        //}

        ImGui::EndTabBar();
    }

    ImGui::End();

    ImVec2 padding(10.0f, 10.0f);
    ImVec2 window_pos = ImVec2(viewport->WorkPos.x + padding.x,
        viewport->WorkPos.y + viewport->WorkSize.y - 100.0f);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.25f); // 透明度（0.0f ～ 1.0f）

    ImGui::Begin("ShortcutInfo", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoDecoration);

    ImGui::Text(" ShortcutInfo:");
    ImGui::BulletText("Alt + Enter  : fullscreen");
    ImGui::BulletText("F8           : debugCamera");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#if 0
    ImGui::Text("Video memory usage %d MB", video_memory_usage());
#endif
    ImGui::End();
#endif
}

bool TutorialScene::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
{
    framebufferDimensions.cx = static_cast<LONG>(width);
    framebufferDimensions.cy = height;

    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);

    framebuffers[0] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx / downsamplingFactor, framebufferDimensions.cy / downsamplingFactor);
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);
    //ブルーム
    bloomer = std::make_unique<Bloom>(device, framebufferDimensions.cx, framebufferDimensions.cy);
    return true;
}
