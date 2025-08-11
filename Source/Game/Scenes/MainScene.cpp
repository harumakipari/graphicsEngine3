#include "MainScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif
#include <random>

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Resource/GltfModel.h"
#include "Graphics/Resource/GltfModelStaticBatching.h"
#include "Engine/Utility/Win32Utils.h"
#include "Engine/Utility/Timer.h"
//#include "Camera.h"
#include "Physics/Collider.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsUtility.h"
#include "Components/Camera/CameraComponent.h"
#include "Game/Actors/Camera/Camera.h"
#include "Game/Actors/Camera/DefeatEnemyCamera.h"


#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Actors/Enemy/DefeatEnemy.h"
#include "Game/Actors/Stage/Building.h"
#include "Game/Actors/Stage/BossBuilding.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Stage/Objects/StageProp.h"
#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Player/TitlePlayer.h"
#include "Game/Managers/GameManager.h"
// FOG
#include "Graphics/Resource/PrecomputedNoiseTexture3D.h"
#include <DDSTextureLoader.h>

#include "Components/Transform/Transform.h"

#include "Core/ActorManager.h"
#include "Physics/CollisionSystem.h"


#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/GameUIFactory.h"
#include "Widgets/PauseUIFactory.h"
#include "Widgets/ResultUIFactory.h"
#include "Widgets/Image.h"
#include "Widgets/Timer.h"

#include "Game/Actors/Enemy/EmptyEnemy.h"
#include "Components/Audio/AudioSourceComponent.h"

bool MainScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    OutputDebugStringA((std::string("Scene::Initialize this=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("_current_scene.get()=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("actorManager_ ptr=") + std::to_string(reinterpret_cast<uintptr_t>(this->GetActorManager())) + "\n").c_str());

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

    //ProjectionMappingConstants
    bufferDesc.ByteWidth = sizeof(projectionMappingConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[3].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    bufferDesc.ByteWidth = sizeof(LightConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[4].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // FOG 
    framebuffers[0] = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, true);
    CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", pixelShaders[2].GetAddressOf());
    D3D11_TEXTURE2D_DESC texture2dDesc;
# if 0
    LoadTextureFromFile(device, L"./Data/Effect/Particles/noise.png", noise2d.GetAddressOf(), &texture2dDesc);
#else
    //LoadTextureFromFile(device, L"./Data/Effect/Particles/noise1.png", noise2d.GetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Effect/Textures/noise.png", noise2d.GetAddressOf(), &texture2dDesc);
#endif
    sceneConstants.time = 0;//開始時に０にしておく
#if 1
    //Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    //hr = DirectX::CreateDDSTextureFromFile(device, L"./Data/Effect/Particles/_noise_3d.dds", resource.GetAddressOf(), noise3d.GetAddressOf());
    //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#else
    ProcomputedNoiseTexture3d(device, 64, noise3d.GetAddressOf());
#endif
    //スカイマップ
    //skyMap = std::make_unique<decltype(skyMap)::element_type >(device, L"./Data/Environment/Sky/winter_evening_4k.DDS");
    //skyMap = std::make_unique<decltype(skyMap)::element_type >(device, L"./Data/Environment/Sky/cloud/skybox.dds");

    fullscreenQuadTransfer = std::make_unique<FullScreenQuad>(device);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, static_cast<uint32_t>(width), height, 3);

    // GBUFFER
    gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
    CreatePsFromCSO(device, "./Shader/DefefferdPS.cso", pixelShaders[1].ReleaseAndGetAddressOf());

    //ブルーム
    bloomer = std::make_unique<Bloom>(device, static_cast<uint32_t>(width), height);
    CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", pixelShaders[0].ReleaseAndGetAddressOf());

    //CascadedShadpwMaps
    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    // VOLUMETRIC_CLOUDSCAPES
    //volumetricCloudscapes = std::make_unique<decltype(volumetricCloudscapes)::element_type>(device, L"./Data/Environment/VolumetricCloudscapes/weather.bmp");
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width) / downsamplingFactor, height / downsamplingFactor);

    //gameWorld_ = std::make_unique<World>();

    //shaderToy
    //shaderToyTransfer = std::make_unique<FullScreenQuad>(device);

    //// LoadSceneに持っていく用
    //CreatePsFromCSO(device, "./Shader/ShaderToyPS.cso", shaderToyPS.GetAddressOf());
    CreatePsFromCSO(device, "./Shader/ShaderToySkyPS.cso", pixelShaders[3].GetAddressOf());
    CreatePsFromCSO(device, "./Shader/ShaderToyPS.cso", pixelShaders[4].GetAddressOf());

    //テクスチャをロード
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_charlie.dds", shaderResourceViews[0].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/diffuse_iem.dds", shaderResourceViews[1].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/specular_pmrem.dds", shaderResourceViews[2].ReleaseAndGetAddressOf(), &texture2dDesc);
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_ggx.dds", shaderResourceViews[3].ReleaseAndGetAddressOf(), &texture2dDesc);


    Physics::Instance().Initialize();

    // 最終の描画
    framebuffers[1] = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, true);

    bufferDesc.ByteWidth = sizeof(ShaderToyCB);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, shaderToyConstantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //アクターをセット
    SetUpActors();

    //パーティクルシステム
    //particles = std::make_unique<decltype(particles)::element_type>(device, 10000);
    //LoadTextureFromFile(device, L"./Data/Effect/Particles/LargeFlame01.tif", particleTexture.GetAddressOf(), NULL);
    //particles->particleSystemData.spriteSheetGrid = { 8,8 };
    ////LoadTextureFromFile(device, L"./Data/Effect/Particles/Ramp02.png", colorTemperChart.GetAddressOf(), NULL); // blue
    //LoadTextureFromFile(device, L"./Data/Effect/Particles/ramp01.png", colorTemperChart.GetAddressOf(), NULL); // red 

    effectSystem = std::make_unique<EffectSystem>();
    effectSystem->Initialize();
    //collisionMesh = std::make_unique<decltype(collisionMesh)::element_type>(device, "./Data/Models/Stage/ExampleStage.gltf");

    //sprite_batches[0] = std::make_unique<SpriteBatch>(device, L"./Data/Textures/Screens/GameScene/screenshot.jpg", 1);

    EventSystem::Initialize();//追加 UI
    
    //死亡フラグ初期化
    isBossDeath = false;
    isPlayerDeath = false;

    GameManager::Initialize();
#if 0
#endif
    // ScreenSpace-ProjectionMapping
    hr = CreatePsFromCSO(device, "./Shader/ScreenSpaceProjectionMappingPS.cso", screenSpaceProjectionMappingPixelShader.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    return true;
}

void MainScene::Start()
{
    GameUIFactory::Create(this);
    PauseUIFactory::Create(this);

    //ゲームBGM再生
    GameObject* soundObj = UIFactory::Create("GameBGM");
    AudioSource* gameBgm = soundObj->AddComponent<AudioSource>(L"./Data/Sound/BGM/game.wav");
    gameBgm->SetVolume(0.7f);
    gameBgm->SetLoopOption(27.2f, 102.4f);
    gameBgm->Play(XAUDIO2_LOOP_INFINITE);
    //WARNING効果音再生
    GameObject* warningObj = UIFactory::Create("WarningSE");
    AudioSource* warning = warningObj->AddComponent<AudioSource>(L"./Data/Sound/SE/warning.wav");
    warning->SetVolume(1.0f);
    warning->Play();
}

void MainScene::Update(ID3D11DeviceContext* immediateContext, float elapsedTime)
{
    auto camera = std::dynamic_pointer_cast<MainCamera>(GetActorManager()->GetActorByName("mainCameraActor"));

    ////elapsedTimeを止めてposeする
    //if (elapsed_time > 0.05f)
    //{
    //    elapsed_time = 0.0f;
    //}
    //camera->SetOldTarget(player->GetPosition());

    Physics::Instance().Update(elapsedTime);
    //gameWorld_->Tick(elapsed_time);
    //ActorManager::Update(elapsedTime);
    EventSystem::Update(elapsedTime);//追加
    objectManager.Update(elapsedTime);//追加
    GameManager::Update(elapsedTime);
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

    if (GetAsyncKeyState(VK_RETURN) & 0x0001 && GetAsyncKeyState(VK_MENU) & 0x8000)
    {
        Graphics::StylizeWindow(!Graphics::fullscreenMode);
    }

#ifdef _DEBUG
    if (InputSystem::GetInputState("F8", InputStateMask::Trigger))
    {
        CameraManager::ToggleCamera();
    }
#endif
    float screenWidth = Graphics::GetScreenWidth();
    float screenHeight = Graphics::GetScreenHeight();

    shaderToy.iTime += elapsedTime;
    shaderToy.iResolution.x = screenWidth;
    shaderToy.iResolution.y = screenHeight;


    //フェードハンドラ
    fadeHandler.Update(fadeValue, elapsedTime);
    float dummy;
    waitHandler.Update(dummy, elapsedTime);
    if (isBossDeath || isPlayerDeath)
    {
        ObjectManager::Find("Fade")->GetComponent<Image>()->color.a = fadeValue;
    }

    if (waitHandler.IsCompleted())
    {
        XMFLOAT4 colors[] = {
            {0,0,0,1},
            {0,1,0,1},
            {0,0,1,1},
            {1,0,0,1},
            {1,1,0,1},
        };
        effectSystem->ResultParticle(colors[GameManager::GetRank()]);
    }

    //死亡処理
    if (!isBossDeath && std::dynamic_pointer_cast<RiderEnemy>(enemies[0])->GetHP() <= 0)
    {
        //enemies[0]->SetPendingDestroy();

        //stop
        std::dynamic_pointer_cast<RiderEnemy>(GetActorManager()->GetActorByName("enemy"))->rushAudioComponent->Stop();

        GameObject* fadeCanvas = ObjectManager::Find("FadeCanvas");
        fadeCanvas->SetActive(true);

        //BGM小さくする
        ObjectManager::Find("GameBGM")->GetComponent<AudioSource>()->SetVolume(0.2f);

        //ゲームUI非表示
        ObjectManager::Find("TimerCanvas")->SetActive(false);
        ObjectManager::Find("BossIndicatorCanvas")->SetActive(false);
        ObjectManager::Find("BossCanvas")->SetActive(false);
        ObjectManager::Find("PlayerCanvas")->SetActive(false);
        ObjectManager::Find("MenuCanvas")->SetActive(false);

        fadeHandler.Clear();
        fadeHandler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.25f);
        fadeHandler.SetCompletedFunction([&]() {
            fadeHandler.SetEasing(EaseType::OutExp, 1.0f, 0.0f, 0.25f);
            GetActorManager()->ClearAll();
            stage = GetActorManager()->CreateAndRegisterActor<Stage>("stage");
            Transform transform(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,180.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            //Transform transform(DirectX::XMFLOAT3{ -1.2f,0.1f,-0.9f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            auto defeatEnemy = GetActorManager()->CreateAndRegisterActorWithTransform<DefeatEnemy>("defeatEnemy", transform);

            auto defeatCamera = GetActorManager()->CreateAndRegisterActor<DefeatEnemyCamera>("defeatEnemyCamera");
            auto mainCameraComponent = defeatCamera->GetComponent<CameraComponent>();
            //XMFLOAT3 target = defeatEnemy->GetPosition();
            //target.y = 2.0f;
            //defeatCamera->SetTarget(target);
            //CameraManager::SetGameCamera(mainCameraComponent);
            CameraManager::SetGameCamera(defeatCamera.get());

            //撃破SE再生
            Audio::PlayOneShot(L"./Data/Sound/SE/defeat.wav");

            waitHandler.SetWait(10.0f);
            waitHandler.SetCompletedFunction([&]() {
                ResultUIFactory::Create(this, true);
                }, true);

            }, true);

        isBossDeath = true;
    }
#ifdef _DEBUG
    //デバッグ用
    if (InputSystem::GetInputState("E", InputStateMask::Trigger))
    {
        //player->ApplyDirectHpDamage(100);
    }
#endif
    // プレイヤーが死亡したら
    if (!isPlayerDeath && player->GetHp() <= 0)
    {
        //BGM小さくする
        ObjectManager::Find("GameBGM")->GetComponent<AudioSource>()->SetVolume(0.2f);

        //ゲームUI非表示
        ObjectManager::Find("TimerCanvas")->SetActive(false);
        ObjectManager::Find("BossIndicatorCanvas")->SetActive(false);
        ObjectManager::Find("BossCanvas")->SetActive(false);
        ObjectManager::Find("PlayerCanvas")->SetActive(false);
        ObjectManager::Find("MenuCanvas")->SetActive(false);

        //stop
        std::dynamic_pointer_cast<RiderEnemy>(GetActorManager()->GetActorByName("enemy"))->rushAudioComponent->Stop();

        ObjectManager::Find("FadeCanvas")->SetActive(true);
        fadeHandler.Clear();
        fadeHandler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.25f);
        fadeHandler.SetCompletedFunction([&]() {
            fadeHandler.SetEasing(EaseType::OutExp, 1.0f, 0.0f, 0.25f);

            //プレイヤー死亡処理
            GetActorManager()->ClearAll();
            stage = GetActorManager()->CreateAndRegisterActor<Stage>("stage");

            auto resultCamera = GetActorManager()->CreateAndRegisterActor<TitleCamera>("resultCamera");
            auto mainCameraComponent = resultCamera->GetComponent<CameraComponent>();
            CameraManager::SetGameCamera(resultCamera.get());
            //CameraManager::SetGameCamera(mainCameraComponent);

            Transform playerTr(DirectX::XMFLOAT3{ -3.0f,0.4f,10.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            //Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.4f,0.0f }, DirectX::XMFLOAT3{ 0.0f,30.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,-1.0f });
            auto resultPlayer = GetActorManager()->CreateAndRegisterActorWithTransform<TitlePlayer>("resultActor", playerTr);
            resultPlayer->SetType(TitlePlayer::Type::ResultLose);
            //resultPlayer->PlayAnimation("Lose", false);
            resultPlayer->PlayAnimation("Idle", false);

            Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-15.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            auto defeatEnemy = GetActorManager()->CreateAndRegisterActorWithTransform<EmptyEnemy>("WinEnemy", enemyTr);
            defeatEnemy->PlayAnimation("Idle");

            waitHandler.SetWait(3.0f);
            waitHandler.SetCompletedFunction([&]() {
                ResultUIFactory::Create(this, false);
                }, true);

            }, true);
        isPlayerDeath = true;
    }

    effectSystem->Update(elapsedTime);


    auto cameraComp = CameraManager::GetCurrentCamera();
    if (cameraComp && enemies[0] && enemies[0]->rootComponent_)
    {
        ViewConstants data = cameraComp->GetViewConstants();

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&data.projection);
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&data.view);
        //DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&cameraComp->GetProjection());
        //DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&cameraComp->GetView());
        DirectX::XMFLOAT3 worldEnemyPos = enemies[0]->GetPosition();
        DirectX::XMVECTOR WorldEnemyPosVec = DirectX::XMLoadFloat3(&worldEnemyPos);

        // ワールド -> ビュー　-> プロジェクション　->NDC
        DirectX::XMVECTOR clipPos = DirectX::XMVector3Transform(WorldEnemyPosVec, V * P);

        // wで除算（NDC空間へ）
        XMFLOAT4 ndc;
        XMStoreFloat4(&ndc, clipPos);

        ndc.x /= ndc.w;
        ndc.y /= ndc.w;
        ndc.z /= ndc.w;
        // -1.0 ～ 1.0 の範囲に収まっていれば画面内
        if (ndc.x >= -1.0f && ndc.x <= 1.0f && ndc.y >= -1.0f && ndc.y <= 1.0f && ndc.z >= 0.0f && ndc.z <= 1.0f)
        {
            isFrameOutEnemy = false;
        }
        else
        {
            isFrameOutEnemy = true;
        }

        imGuiNDC = ndc;
    }

    // エネミー プレイヤー　スクリーン座標取得
    {
        if (cameraComp && enemies[0] && enemies[0]->rootComponent_ && player && player->rootComponent_)
        {
            ViewConstants data = cameraComp->GetViewConstants();

            DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&data.projection);
            DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&data.view);
            DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

            DirectX::XMFLOAT3 worldEnemyPos = enemies[0]->GetPosition();
            DirectX::XMFLOAT3 worldPlayerPos = player->GetPosition();
            // ワールド座標からスクリーン座標へ変換
            DirectX::XMVECTOR WorldEnemyPostion, ScreenEnemyPosition;
            WorldEnemyPostion = DirectX::XMLoadFloat3(&worldEnemyPos);
            ScreenEnemyPosition = DirectX::XMVector3Project(WorldEnemyPostion, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, P, V, World);
            DirectX::XMVECTOR WorldPlayerPostion, ScreenPlayerPosition;
            WorldPlayerPostion = DirectX::XMLoadFloat3(&worldPlayerPos);
            ScreenPlayerPosition = DirectX::XMVector3Project(WorldPlayerPostion, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, P, V, World);

            DirectX::XMStoreFloat2(&enemyScreenPosition, ScreenEnemyPosition);
            DirectX::XMStoreFloat2(&playerScreenPosition, ScreenPlayerPosition);
        }
    }

    // スクリーン同士の差分のベクトル
    float dx = enemyScreenPosition.x - playerScreenPosition.x;
    float dy = enemyScreenPosition.y - playerScreenPosition.y;

    // UIの角度
    {
        if (enemies[0] && enemies[0]->rootComponent_ && player && player->rootComponent_)
        {
            DirectX::XMFLOAT3 worldEnemyPos = enemies[0]->GetPosition();
            DirectX::XMFLOAT3 worldPlayerPos = player->GetPosition();
            worldEnemyPos.y = 0;
            worldPlayerPos.y = 0;
            XMVECTOR WorldEnemyPostion = DirectX::XMLoadFloat3(&worldEnemyPos);
            XMVECTOR WorldPlayerPostion = DirectX::XMLoadFloat3(&worldPlayerPos);

            XMVECTOR Dir = XMVector3Normalize(WorldEnemyPostion - WorldPlayerPostion);
            XMFLOAT3 dir;
            XMStoreFloat3(&dir, Dir);

            XMFLOAT3 forward = { 0,0,1 };
            float angleRad = atan2(forward.x * dir.z - forward.z * dir.x, forward.x * dir.x + forward.z * dir.z);
            angleDegree = -XMConvertToDegrees(angleRad);
        }
    }

    // スクリーンの中心を原点としたベクトル
    DirectX::XMFLOAT2 dirVec = { dx,dy };

    float halfW = screenWidth * 0.5f;
    float halfH = screenHeight * 0.5f;
    DirectX::XMFLOAT2 origin = playerScreenPosition;

    float maxLength = std::max<float>(screenWidth, screenHeight);

    float length = std::sqrt(dx * dx + dy * dy);
    //if (length == 0.0f) return; 
    dirVec.x /= length;
    dirVec.y /= length;

    intersection.x = origin.x + dirVec.x * maxLength;
    intersection.y = origin.y + dirVec.y * maxLength;

    if (intersection.x < 0)
    {
        intersection.x = 0;
    }
    if (intersection.x > screenWidth)
    {
        intersection.x = screenWidth;
    }
    if (intersection.y < 0)
    {
        intersection.y = 0;
    }
    if (intersection.y > screenHeight)
    {
        intersection.y = screenHeight;
    }

    if (enemies[0] && enemies[0]->rootComponent_ && camera)
    {
        if (enemies[0]->GetIsStartPerf())
        {// 敵の最初の演出中
            if (enemies[0]->GetIsEnemyFall())
            {// 敵の落ち始めで　
                //if (!isFrameOutEnemy)
                {// ボスが画面内に入ってきたら
                    camera->IsTargetBoss(true);
                }
            }
            else
            {// 敵が落ちる前　WARNINGの間
                camera->IsTargetBoss(false);
            }
        }
        else
        {// 敵の最初の演出が終わったら
            camera->OnFinishFirstPerf();
        }
    }


    //DirectX::XMFLOAT3 position = player->GetPosition();
    //DirectX::XMFLOAT3 forward = player->GetForward();
    //position.y += 2.0f;
    //position.z -= 3.0f;
    ////gameWorld_->FindActorByName("mainCameraActor")->GetComponentByName("springArm")->SetWorldLocationDirect(position);
    //ActorManager::GetActorByName("mainCameraActor")->GetComponentByName("springArm")->SetWorldLocationDirect(position);
    if (player && player->rootComponent_ && camera)
    {
        camera->SetTarget(player->GetPosition());
        // プレイヤーの被弾時のカメラシェイク
        if (player->hasDamageThisFrame)
        {
            camera->Shake(power, timer);
        }
    }

#if USE_IMGUI
    ImGui::Begin("CameraShake");
    ImGui::DragFloat("power", &power, 0.01f);
    ImGui::DragFloat("timer", &timer, 0.01f);
    ImGui::DragFloat2("intersection", &intersection.x, 0.01f);
    ImGui::DragFloat("angleDegree", &angleDegree, 0.01f);
    ImGui::DragFloat4("enemy NDC space", &imGuiNDC.x);
    ImGui::Checkbox("enemy is Out", &isFrameOutEnemy);
    ImGui::End();
#endif // USE_IMGUI


#if 0
#if 1
    //particles->particleSystemData.worldTransform = enemys[0]->GetNodeWorldMatrix(64/*tongue_03*/);
    //particles->particleSystemData.emissionSize.y = 1.0f;
    particles->particleSystemData.direction = { 1,0,0 };
    particles->particleSystemData.strength = 2.0f;
    particles->particleSystemData.lifespan = { 1.53f,1.53f };
    particles->particleSystemData.fadeDuration = { 0.0f,0.63f };
    particles->particleSystemData.emissionPosition.y = 1.5f;
    static bool run_once = true;
    if (run_once || GetAsyncKeyState('Z') & 0x8000)
    {
        particles->Initialize(immediateContext, 0);
        run_once = false;
    }

    if (integrateParticles)
    {
        immediateContext->CSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());
        particles->Integrate(immediateContext, elapsedTime);
    }
#else
    //static bool run_once = true;
    //if (run_once || GetAsyncKeyState('Z') & 0x8000)
    //{
    //    particles->Initialize(immediate_context, 0);
    //    run_once = false;
    //}
    //particles->particleSystemData.worldTransform = player->GetWorldMatrix();
    //particles->particleSystemData.nodeWorldTransform = player->GetJointTransform(player->nodeAttackIndex);
    ////particles->particleSystemData.direction = player->GetJointForwardVector(player->nodeAttackIndex);
    //particles->particleSystemData.emissionPosition.x =spherePosition.x;
    //particles->particleSystemData.emissionPosition.y =spherePosition.y;
    //particles->particleSystemData.emissionPosition.z =spherePosition.z;
    //if (integrateParticles)
    //{
    //    immediate_context->CSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());
    //    particles->Integrate(immediate_context, elapsed_time);
    //}



    if (particles != nullptr)
    {
        enemys[0]->UpdateParticle(immediateContext, elapsedTime, particles.get());
    }
    particles->particleSystemData.direction = { 1,0,0 };
    particles->particleSystemData.strength = 2.0f;
    particles->particleSystemData.lifespan = { 1.53f,1.53f };
    particles->particleSystemData.fadeDuration = { 0.0f,0.63f };

    //particles->particleSystemData.worldTransform = enemys[0]->GetWorldMatrix();
    particles->particleSystemData.worldTransform = enemys[0]->GetJointTransform(64);

    if (integrateParticles)
    {
        immediateContext->CSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());
        //particles->Integrate(immediate_context, elapsed_time);
    }
#endif

#endif

    //particles->particleSystemData.emissionPosition.x += 1.0f * elapsed_time;

#if 0
#ifdef USE_IMGUI

    static const std::unordered_map<int, std::string> items =
    {
        { 0, "Alpha" },
        { 1, "Add" },
    };
    std::string current_item = items.at(blendMode);
    if (ImGui::BeginCombo("blend mode", current_item.c_str()))
    {
        for (decltype(items)::const_reference item : items)
        {
            bool is_selected = (current_item == item.second);
            if (ImGui::Selectable(item.second.c_str(), is_selected))
            {
                current_item = item.second;
                blendMode = item.first;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Checkbox("integrate_particles", &integrateParticles);
    ImGui::DragFloat2("particles radius", &particles->particleSystemData.radius.x, 0.1f);
    ImGui::SliderFloat("gravity", &particles->particleSystemData.gravity, -1.0f, +1.0f, "%.4f");
    ImGui::SliderFloat("strength", &particles->particleSystemData.strength, -10.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("lifespan min", &particles->particleSystemData.lifespan.x, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("lifespan max", &particles->particleSystemData.lifespan.y, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("spawnDelay min", &particles->particleSystemData.spawnDelay.x, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("spawnDelay max", &particles->particleSystemData.spawnDelay.y, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("fade in duration", &particles->particleSystemData.fadeDuration.x, +0.0f, 10.0f, "%.4f");
    ImGui::SliderFloat("fade out duration", &particles->particleSystemData.fadeDuration.y, +0.0f, 10.0f, "%.4f");
    ImGui::SliderFloat("emissionPosition.x", &particles->particleSystemData.emissionPosition.x, -10.0f, +10.0f);
    ImGui::SliderFloat("emissionPosition.y", &particles->particleSystemData.emissionPosition.y, -10.0f, +10.0f);
    ImGui::SliderFloat("emissionPosition.z", &particles->particleSystemData.emissionPosition.z, -10.0f, +10.0f);
    ImGui::SliderFloat("emissionOffset min", &particles->particleSystemData.emissionOffset.x, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionOffset max", &particles->particleSystemData.emissionOffset.y, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionSize spawn", &particles->particleSystemData.emissionSize.x, +0.0f, +5.0f, "%.4f");
    ImGui::SliderFloat("emissionSize despawn", &particles->particleSystemData.emissionSize.y, +0.0f, +5.0f, "%.4f");
    ImGui::SliderFloat("emissionSpeed min", &particles->particleSystemData.emissionSpeed.x, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionSpeed max", &particles->particleSystemData.emissionSpeed.y, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionAngularSpeed min", &particles->particleSystemData.emissionAngularSpeed.x, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionAngularSpeed max", &particles->particleSystemData.emissionAngularSpeed.y, +0.0f, +10.0f, "%.4f");
    ImGui::SliderFloat("emissionConeAngle min", &particles->particleSystemData.emissionConeAngle.x, +0.0f, +3.141592653f, "%.4f");
    ImGui::SliderFloat("emissionConeAngle max", &particles->particleSystemData.emissionConeAngle.y, +0.0f, +3.141592653f, "%.4f");
    ImGui::SliderFloat("noiseScale", &particles->particleSystemData.noiseScale, +0.0f, +1.0f, "%.4f");


    //ImGui::Begin("Volumetric Cloudscapes");
    //// VOLUMETRIC_CLOUDSCAPES
    //ImGui::DragFloat4("cameraFocus", &cameraFocus.x, 0.5f);

    //ImGui::DragFloat("density_scale", &volumetricCloudscapes->constantData.densityScale, 0.001f, 0.0f, 1.0f);
    //ImGui::DragFloat("cloud_coverage_scale", &volumetricCloudscapes->constantData.cloudCoverageScale, 0.001f, 0.0f, 0.5f);
    //ImGui::DragFloat("rain_cloud_absorption_scale", &volumetricCloudscapes->constantData.rainCloudAbsorptionScale, 0.01f, 0.0f, 10.0f, "%.2f");
    //ImGui::DragFloat("cloud_type_scale", &volumetricCloudscapes->constantData.cloudTypeScale, 0.01f, 0.0f, 10.0f, "%.2f");

    //ImGui::DragFloat("low_frequency_perlin_worley_sampling_scale", &volumetricCloudscapes->constantData.lowFrequencyPerlinWorleySamplingScale, 0.000001f, 0.0f, 1.0f, "%.7f");
    //ImGui::DragFloat("high_frequency_worley_sampling_scale", &volumetricCloudscapes->constantData.highFrequencyWorleySamplingScale, 0.00001f, 0.0f, 1.0f, "%.5f");
    //ImGui::DragFloat("horizon_distance_scale", &volumetricCloudscapes->constantData.horizonDistanceScale, 0.0001f, 0.0f, 1.0f, "%.4f");

    //ImGui::SliderFloat2("wind_direction", &(volumetricCloudscapes->constantData.windDirection.x), -1.0f, +1.0f);
    //ImGui::SliderFloat("wind_speed", &volumetricCloudscapes->constantData.windSpeed, 0.0f, 20.0f);

    //ImGui::DragFloat("earth_radius", &volumetricCloudscapes->constantData.earthRadius, 1.0f);
    //ImGui::DragFloat("cloud_altitudes_min", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.x, 1.0f);
    //ImGui::DragFloat("cloud_altitudes_max", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.y, 1.0f);

    //ImGui::DragFloat("cloud_density_long_distance_scale", &volumetricCloudscapes->constantData.cloudDensityLongDistanceScale, 0.01f, 0.0f, 36.0f, "%.2f");
    //ImGui::Checkbox("enable_powdered_sugar_efffect", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.enablePowderedSugarEffect));

    //ImGui::SliderInt("ray_marching_steps", &volumetricCloudscapes->constantData.rayMarchingSteps, 1, 128);
    //ImGui::Checkbox("auto_ray_marching_steps", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.autoRayMarchingSteps));

    //ImGui::End();
#endif
#endif
#ifdef _DEBUG
    if (GetAsyncKeyState('P') & 1)
    {
        Scene::_transition("BootScene", {});
    }
#endif // !_DEBUG
#if 0

#endif
}
void MainScene::Render(ID3D11DeviceContext* immediateContext, float elapsedTime)
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

    // NOISE
    immediateContext->PSSetShaderResources(10, 1, noise3d.GetAddressOf());
    immediateContext->PSSetShaderResources(11, 1, noise2d.GetAddressOf());

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

    //sceneConstants.lightDirection = lightDirection;
    //sceneConstants.colorLight = colorLight;
    //sceneConstants.iblIntensity = iblIntensity;
    lightConstants.lightDirection = lightDirection;
    lightConstants.colorLight = colorLight;
    lightConstants.iblIntensity = iblIntensity;
    lightConstants.directionalLightEnable = static_cast<int>(directionalLightEnable);
    lightConstants.pointLightEnable = static_cast<int>(pointLightEnable);
    lightConstants.pointLightCount = pointLightCount;
    for (int i = 0; i < pointLightCount; i++)
    {
        lightConstants.pointsLight[i].position = pointLightPosition[i];
        lightConstants.pointsLight[i].color = pointLightColor[i];
        lightConstants.pointsLight[i].range = pointLightRange[i];
    }

    // SCREEN_SPACE_AMBIENT_OCCLUSION
    sceneConstants.enableSSAO = enableSSAO;
    sceneConstants.enableBloom = enableBloom;
    sceneConstants.enableFog = enableFog;
    sceneConstants.enableCascadedShadowMaps = enableCascadedShadowMaps;
    sceneConstants.enableSSR = enableSSR;
    // SCREEN_SPACE_REFLECTION
    sceneConstants.reflectionIntensity = refrectionIntensity;
    // FOG
    sceneConstants.time += elapsedTime;

    immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &sceneConstants, 0, 0);
    immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());



#if 1
    // PROJECTION_MAPPING
    using namespace DirectX;
    //static XMFLOAT3 projection_mapping_eye = { 0,10,0 };
    //static XMFLOAT3 projection_mapping_focus = { 0,0,0 };
    static float projection_mapping_rotation = 0;
    static float projection_mapping_fovy = 10.f;
    projection_mapping_rotation += elapsedTime * 180;

    for (int i = 0; i < MAX_PROJECTION_MAPPING; i++)
    {
        XMVECTOR Eye = XMLoadFloat3(&eyes[i]);
        XMVECTOR Target = XMLoadFloat3(&targets[i]);
        if (XMVector3Equal(Eye, Target))
        {
            Eye += (XMVectorSet(0, 1, 0, 0) * distance);
        }

        XMStoreFloat4x4(&projectionMappingConstants.projectionMappingTransforms[i],
            XMMatrixLookAtLH(Eye, Target,
                XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMMatrixRotationRollPitchYaw(0, XMConvertToRadians(projection_mapping_rotation), 0))) *
            XMMatrixPerspectiveFovLH(XMConvertToRadians(projection_mapping_fovy), 1.0f, 1.0f, 500.0f)
        );
        uint32_t* enableMappings = &projectionMappingConstants.enableMapping[0].x;
        if (!enableMappings[i]) continue;
        ComputeParticleSystem::EmitParticleData data;
        //	更新タイプ
        data.parameter.x = 1;
        data.parameter.y = 1.f;
        //data.parameter.w = (rand() % 5 + 1) * 0.1f;
        //	発生位置
        data.position.x = targets[i].x;
        data.position.y = 1.f;
        data.position.z = targets[i].z;
        //	初速力
        data.velocity.x = 0;
        data.velocity.y = 2.f;
        data.velocity.z = 0;
        //	加速力
        data.acceleration.x = 0.0f;
        data.acceleration.y = 3.0f;
        data.acceleration.z = 0.0f;
        //	大きさ
        data.scale.x = 0.05f;
        data.scale.y = 0.05f;
        data.scale.z = 0.3f;
        data.scale.w = 0.3f;
        //色
        data.color = { 1,0.5f,0,1 };
        //	ターゲット位置
        data.customData.x = targets[i].x;
        data.customData.y = 0.0f;
        data.customData.z = targets[i].z;
        data.customData.w = 1.0f;

        effectSystem->computeParticles[5]->Emit(data);
    }

    //フラグ初期化
    for (int i = 0; i < (MAX_PROJECTION_MAPPING / 4); i++) {
        projectionMappingConstants.enableMapping[i] = { 0,0,0,0 };
    }

    //プロジェクションマッピング更新処理
    int i = 0;
    for (const auto& actor : GetActorManager()->GetAllActors())
    {
        if (auto bomb = std::dynamic_pointer_cast<Bomb>(actor))
        {

            if (bomb->GetIsValid() && !bomb->IsGoingUp())
            {
                DirectX::XMFLOAT3 pos = bomb->GetPosition();
                DirectX::XMFLOAT3 projectionPos = bomb->GetProjectionPosition();

                XMVECTOR Eye = XMLoadFloat3(&projectionPos);
                XMVECTOR Target = Eye;
                if (XMVector3Equal(Eye, Target))
                {
                    Eye += (XMVectorSet(0, 1, 0, 0) * distance);
                }

                XMStoreFloat4x4(&projectionMappingConstants.projectionMappingTransforms[i],
                    XMMatrixLookAtLH(Eye, Target,
                        XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMMatrixRotationRollPitchYaw(0, XMConvertToRadians(projection_mapping_rotation), 0))) *
                    XMMatrixPerspectiveFovLH(XMConvertToRadians(projection_mapping_fovy), 1.0f, 1.0f, 500.0f)
                );

                ComputeParticleSystem::EmitParticleData data;
                //	更新タイプ
                data.parameter.x = 1;
                data.parameter.y = 1.f;
                //data.parameter.w = (rand() % 5 + 1) * 0.1f;
                //	発生位置
                data.position.x = projectionPos.x;
                data.position.y = 1.f;
                data.position.z = projectionPos.z;
                //	初速力
                data.velocity.x = 0;
                data.velocity.y = 2.f;
                data.velocity.z = 0;
                //	加速力
                data.acceleration.x = 0.0f;
                data.acceleration.y = 3.0f;
                data.acceleration.z = 0.0f;
                //	大きさ
                data.scale.x = 0.05f;
                data.scale.y = 0.05f;
                data.scale.z = 0.3f;
                data.scale.w = 0.3f;
                //色
                data.color = { 1,0.5f,0,1 };
                //	ターゲット位置
                data.customData.x = projectionPos.x;
                data.customData.y = 0.0f;
                data.customData.z = projectionPos.z;
                data.customData.w = 1.0f;

                effectSystem->computeParticles[5]->Emit(data);

                uint32_t* enableMappings = &projectionMappingConstants.enableMapping[0].x;
                enableMappings[i] = true;

                //最大数に到達したらブレイク
                if (++i == MAX_PROJECTION_MAPPING) {
                    break;
                }
            }
        }
    }
    immediateContext->PSSetShaderResources(15, 1, effectSystem->projectionTexture.GetAddressOf());
#endif // 1

    shaderConstants.maxDistance = maxDistance;
    shaderConstants.resolution = resolution;
    shaderConstants.steps = steps;
    shaderConstants.thickness = thickness;

    immediateContext->UpdateSubresource(constantBuffers[1].Get(), 0, 0, &shaderConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());

    immediateContext->UpdateSubresource(constantBuffers[2].Get(), 0, 0, &fogConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());    //3 は cascadedShadowMap に使用中

    immediateContext->UpdateSubresource(constantBuffers[3].Get(), 0, 0, &projectionMappingConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(5, 1, constantBuffers[3].GetAddressOf());

    immediateContext->UpdateSubresource(constantBuffers[4].Get(), 0, 0, &lightConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(11, 1, constantBuffers[4].GetAddressOf());    //3 は cascadedShadowMap に使用中

    //定数バッファをGPUに送信
    {
        immediateContext->UpdateSubresource(shaderToyConstantBuffer.Get(), 0, 0, &shaderToy, 0, 0);
        immediateContext->VSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf()); //register b7に送信
        immediateContext->PSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf());
    }
    // VOLUMETRIC_CLOUDSCAPES
    //framebuffers[1]->Clear(immediateContext);
    //framebuffers[1]->Activate(immediateContext);
    //RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    //RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
    //volumetricCloudscapes->Blit(immediateContext);
    //framebuffers[1]->Deactivate(immediateContext);
    if (!useDeferredRendering)
    {
        // MULTIPLE_RENDER_TARGETS
        multipleRenderTargets->Clear(immediateContext);
        multipleRenderTargets->Acticate(immediateContext);

        // SKY_MAP
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        //RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
        //skyMap->Blit(immediateContext, sceneConstants.viewProjection);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);


        // VOLUMETRIC_CLOUDSCAPES
        //RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        //RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
        //volumetricCloudscapes->Blit(immediateContext);
        //fullscreenQuadTransfer->Blit(immediateContext, framebuffers[1]/*volumetric cloudscapes*/->shaderResourceViews[0].GetAddressOf(), 0, 1);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //sprite_batches[0]->Begin(immediateContext);
        //sprite_batches[0]->Render(immediateContext, 0, 0, 1280, 720);
        //sprite_batches[0]->End(immediateContext);

        // Shader Toy
        {
            //shaderToyFrameBuffer->Clear(immediateContext);// 512 * 512
            //shaderToyFrameBuffer->Activate(immediateContext);

            ID3D11ShaderResourceView* shaderResourceViews[]
            {
                //shaderToyFrameBuffer->shaderResourceViews[0].Get(), //color Map
                nullptr
            };
            fullscreenQuadTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, pixelShaders[3].Get());
            //shaderToyTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, shaderToyPS.Get());
            //shaderToyFrameBuffer->Deactivate(immediateContext);
            //if (isBossDeath)
            {
                //if (auto enemy = std::dynamic_pointer_cast<DefeatEnemy>(ActorManager::GetActorByName("defetEnemy")))
                {
                    //if (enemy->isFinish)
                    {
                        //fullscreenQuadTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, pixelShaders[4].Get());
                    }
                }
            }
        }


        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        // MULTIPLE_RENDER_TARGETS
        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);


        //gameWorld_->Render(immediateContext);

        actorRender.RenderOpaque(immediateContext);
        actorRender.RenderMask(immediateContext);
        actorRender.RenderBlend(immediateContext);
        actorRender.RenderInstanced(immediateContext);
        actorRender.RenderBuilding(immediateContext);


        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);

        // デバック描画
#if _DEBUG
    //actorColliderManager.DebugRender(immediateContext);
    //PhysicsTest::Instance().DebugRender(immediateContext);
    //GameManager::DebugRender(immediateContext);
#endif
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
#if 1
        // PARTICLE
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
        //RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
        //if (blendMode == 0)
        //{
        //    RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        //}
        //else
        //{
        //    RenderState::BindBlendState(immediateContext, BLEND_STATE::ADD);
        //}
        immediateContext->GSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
        immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
        immediateContext->CSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());

        //瓦礫のDissolve
        immediateContext->PSSetShaderResources(11, 1, noise2d.GetAddressOf());
        for (const auto& actor : ShockWaveTargetRegistry::GetTargets())
        {
            if (auto build = std::dynamic_pointer_cast<Building>(actor))
            {
                if (auto& debri = build->convexComponent)
                {
                    if (debri->GetActive())
                    {
                        //effectSystem->computeParticles[9]->PixelEmitBegin(immediateContext, elapsedTime);

                        //RenderState::BindDepthStencilState(Graphics::GetDeviceContext(), DEPTH_STATE::ZT_OFF_ZW_OFF, 0);
                        //RenderState::BindRasterizerState(Graphics::GetDeviceContext(), RASTER_STATE::SOLID_CULL_NONE);

                        DirectX::XMFLOAT4X4 world;
                        DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
                        PipeLineStateDesc pipelineState;
                        pipelineState.pixelShader = effectSystem->dissolvePixelShader;

                        debri->GetMeshComponent()->SetPipeLineState(pipelineState);

                        for (auto& material : debri->GetMeshComponent()->model->materials)
                        {
                            material.replacedPixelShader = effectSystem->dissolvePixelShader;
                        }
                        //if (value > 1.f) value = 1.f;
                        debri->GetMeshComponent()->model->SetDisolveFactor(build->GetDissolveRate());

                        //描画
                        debri->GetMeshComponent()->model->Render(immediateContext, world, debri->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::All, pipelineState);

                        //effectSystem->computeParticles[9]->PixelEmitEnd(immediateContext);
                        //meshComponent->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Mask);
                    }
                }
            }
            else if (auto build = std::dynamic_pointer_cast<BossBuilding>(actor))
            {
                if (auto& debri = build->convexComponent)
                {
                    if (debri->GetActive())
                    {
                        //effectSystem->computeParticles[9]->PixelEmitBegin(immediateContext, elapsedTime);

                        //RenderState::BindDepthStencilState(Graphics::GetDeviceContext(), DEPTH_STATE::ZT_OFF_ZW_OFF, 0);
                        //RenderState::BindRasterizerState(Graphics::GetDeviceContext(), RASTER_STATE::SOLID_CULL_NONE);

                        DirectX::XMFLOAT4X4 world;
                        DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
                        PipeLineStateDesc pipelineState;
                        pipelineState.pixelShader = effectSystem->dissolvePixelShader;

                        debri->GetMeshComponent()->SetPipeLineState(pipelineState);

                        for (auto& material : debri->GetMeshComponent()->model->materials)
                        {
                            material.replacedPixelShader = effectSystem->dissolvePixelShader;
                        }
                        //if (value > 1.f) value = 1.f;
                        debri->GetMeshComponent()->model->SetDisolveFactor(build->GetDissolveRate());

                        //描画
                        debri->GetMeshComponent()->model->Render(immediateContext, world, debri->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::All, pipelineState);

                        //effectSystem->computeParticles[9]->PixelEmitEnd(immediateContext);
                        //meshComponent->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Mask);
                    }
                }
            }
        }
        //immediateContext->PSSetShaderResources(0, 1, particleTexture.GetAddressOf());
        //immediateContext->GSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());
        //actorRender.RenderParticle(immediateContext);
        effectSystem->Render(immediateContext);
        //particles->Render(immediateContext);


#endif
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
#if 0
            framebuffers[0]->Clear(immediateContext, 0, 0, 0, 0);
            framebuffers[0]->Activate(immediateContext);


            RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
            RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
            RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
            ID3D11ShaderResourceView* shader_resource_views[]
            {
                multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
                multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
                cascadedShadowMaps->depthMap().Get(),   //cascaededShadowMaps
            };
            fullscreenQuadTransfer->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), pixelShaders[2]/*VolumetricFogPS*/.Get());

            framebuffers[0]->Deactivate(immediateContext);
#endif
        }

        framebuffers[1]->Clear(immediateContext, 0, 0, 0, 0);
        framebuffers[1]->Activate(immediateContext);

        // ScreenSpace-ProjectionMapping
        {
            ID3D11ShaderResourceView* srvs[]
            {
                multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
                multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
            };

            //プロジェクションマッピングで出すテクスチャをセット
            immediateContext->PSSetShaderResources(15, 1, effectSystem->projectionTexture.GetAddressOf());
            immediateContext->PSSetShaderResources(16, 1, &multipleRenderTargets->renderTargetShaderResourceViews[0]);

            //シーンのSRVとDepth値をPixelShaderに送って描画
            fullscreenQuadTransfer->Blit(immediateContext, srvs, 20, _countof(srvs), screenSpaceProjectionMappingPixelShader.Get());
        }


        framebuffers[1]->Deactivate(immediateContext);

        //framebuffers[1]->Clear(immediateContext, 0, 0, 0, 0);
        //framebuffers[1]->Activate(immediateContext);

        // CASCADED_SHADOW_MAPS
        // Draw shadow to scene framebuffer
        // FINAL_PASS
        {
            //ブルーム
            RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
            RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
            RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
            //bloomer->make(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[0]);
            bloomer->make(immediateContext, framebuffers[1]->shaderResourceViews[0].Get());

            ID3D11ShaderResourceView* shader_resource_views[]
            {
                // MULTIPLE_RENDER_TARGETS
                //multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
                framebuffers[1]->shaderResourceViews[0].Get(),
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
    }

    // UI描画
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        objectManager.Draw(immediateContext);
    }

    //framebuffers[1]->Deactivate(immediateContext);

    //fullscreenQuadTransfer->Blit(immediateContext, framebuffers[1]->shaderResourceViews[0].GetAddressOf(),0,
#if 0

#endif
}

void MainScene::DrawGui()
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
                ImGui::Checkbox("useDeferredRendering", &useDeferredRendering);
                ImGui::Checkbox("directionalLightEnable", &directionalLightEnable);
                ImGui::SliderFloat3("Light Direction", &lightDirection.x, -1.0f, 1.0f);
                ImGui::SliderFloat3("Light Color", &colorLight.x, -1.0f, 1.0f);
                ImGui::SliderFloat("IBL Intensity", &iblIntensity, 0.0f, 10.0f);
                ImGui::SliderFloat("Light Intensity", &colorLight.w, 0.0f, 10.0f);
                ImGui::Checkbox("pointLightEnable", &pointLightEnable);
                ImGui::SliderInt("Point Light Count", &pointLightCount, 0, 8);
                for (int i = 0; i < pointLightCount; i++)
                {
                    std::string header = "PointLight[" + std::to_string(i) + "]";
                    if (ImGui::CollapsingHeader(header.c_str()))
                    {
                        ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), &pointLightPosition[i].x, 0.1f);
                        ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), &pointLightColor[i].x);
                        ImGui::SliderFloat(("Range##" + std::to_string(i)).c_str(), &pointLightRange[i], 0.0f, 10.0f);
                        ImGui::SliderFloat(("Intensity##" + std::to_string(i)).c_str(), &pointLightColor[i].w, 0.0f, 10.0f);
                    }
                }
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
        // ==== Itemタブ（追加） ====
        if (ImGui::BeginTabItem("Item"))
        {
            static bool isYoffsetFromTarget = true;
            ImGui::Checkbox("eye is yOffset from target", &isYoffsetFromTarget);

            uint32_t* enableMappings = &projectionMappingConstants.enableMapping[0].x;
            uint32_t* textureId = &projectionMappingConstants.textureId[0].x;
            for (int i = 0; i < MAX_PROJECTION_MAPPING; i++) {
                ImGui::PushID(i);

                if (ImGui::TreeNodeEx(("Projection" + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("enable", reinterpret_cast<bool*>(&enableMappings[i]));
                    ImGui::SliderInt("texture Id", reinterpret_cast<int*>(&textureId[i]), 0, 1);

                    ImGui::DragFloat3("target", &targets[i].x);

                    if (!isYoffsetFromTarget) {
                        ImGui::DragFloat3("eye", &eyes[i].x);
                    }
                    else {
                        eyes[i] = targets[i];
                    }
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }


            GameManager::DrawGUI();

            ImGui::EndTabItem();
        }

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
#ifdef USE_IMGUI
    //ImGui::Text("occupipedAABBs size:%d", static_cast<int>(SpawnValidator::GetAABBs().size()));
    //ImGui::Text("targets size:%d", static_cast<int>(SpawnValidator::GetTargets().size()));
    //ImGui::Text("ShockWaveTargetRegistry size:%d", static_cast<int>(ShockWaveTargetRegistry::GetTargets().size()));
    //ImGui::BulletText("Alt + Enter  : fullscreen");
    //ImGui::BulletText("F8           : debugCamera");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Video memory usage %d MB", Graphics::video_memory_usage());
#if 0
#endif

    ImGui::End();
#endif
#endif
}

bool MainScene::Uninitialize(ID3D11Device* device)
{
    //ActorManager::ClearAll();
    GameManager::Finalize();
    Physics::Instance().Finalize();
    PhysicsTest::Instance().Finalize();
    //SpawnValidator::Clear();    // 登録していた Box を破棄する
    return true;
}

bool MainScene::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
{
    framebufferDimensions.cx = static_cast<LONG>(width);
    framebufferDimensions.cy = height;

    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);

    framebuffers[0] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx / downsamplingFactor, framebufferDimensions.cy / downsamplingFactor);
    framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);

    //ブルーム
    bloomer = std::make_unique<Bloom>(device, framebufferDimensions.cx, framebufferDimensions.cy);

    return true;
}

//パーティクルシステムセットする
void MainScene::SetParticleSystem()
{

}

void MainScene::LoadModel()
{
    //char aaa[256];
    //DWORD time = timeGetTime();
    benchmark actor_timer;
    benchmark stage_timer;
    benchmark enemy_timer;

    //デバイスを取得
    ID3D11Device* device = Graphics::GetDevice();
    std::thread actorThread = std::thread([&]()
        {
            actor_timer.begin();

            models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Idle.gltf"); //Idle,　
            //models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\Aurora_Frozen_Health_Resuce_Mesh\\untitled.gltf"); //Idle,　
            //models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\CharacterReduceMesh\\Idle.gltf"); //Idle,　
            //models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\CharacterAnimationCopy\\Idle.gltf"); //Idle,  DDS無し
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jog_Fwd.glb");//Running,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Ability_E_InMotion.glb");//Attack,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Primary_Attack_Fast_A.glb");//Attack_First,
            //models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Start.glb");//Jump_Start,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Apex.glb");//Jump_Start,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Apex.glb"); //Jump_Apex,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Land.glb"); //Jump_Land,
            models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Recovery.glb"); //Jump_Recovering,
            //models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Jump_Pad.glb"); //Jump_Attack,
            //models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\HitReact_Front.glb"); //Hit_Damaged,
            //models["actor"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Emote_Slice.glb"); //Emote,
            actor_timer.end();
        });
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\chicken_gun_supermarket\\superMarket1.gltf");
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\ExampleStage.gltf");
    std::thread stageThread = std::thread([&]()
        {
            stage_timer.begin();
            models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\ExampleStage.gltf");
            //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\3d_model_of_trim_castle\\scene.gltf");
            //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\Stage0430\\stageMesh0430.gltf");
            //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\sika\\StageParagon.gltf");
            //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\sika1\\Shika_reduce_pollygon0213.gltf");
            //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\ParagonSample\\ParagonSample.gltf");
            stage_timer.end();
        });
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\stage_miplevel8\\MAP_Demo_test.gltf");
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\stage_miplevel8\\MAP_Demo.gltf");
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\stage_miplevel8_blender\\untitled.gltf");
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\SICKA\\MAP_Demo.gltf");//木の葉無し
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\SICKACOPY\\MAP_Demo.gltf");//木の葉無し  DDSしていない
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\SICKA_DYNASTY\\MAP_Demo2.gltf");//木の葉がおかしい
    //models["stage"] = std::make_shared<GltfModelStaticBatching>(device, "..\\glTF-Sample-Models-main\\original\\SICKA_DYNASTY\\MAP_Demo.gltf");
    //models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\character.glb");
    //models["actor"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\characterAnimation.gltf");

    //models["enemy"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\EnemyAnimation\\Idle_test.gltf"); //Idle,
    //models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyAnimation\\Jog_Fwd.glb"); //Jog_Fwd,
    //models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyAnimation\\Idle_test.gltf"); //Idle,
    enemy_timer.begin();
    models["enemy"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\EnemyTest\\Idle_Relaxed_B_HS.gltf"); //Idle,
    models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyTest\\Jog_Fwd.gltf"); //Jog_Fwd,
    //models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyTest\\Primary_Fast_Fire.glb"); //Attack_Fast,
    models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyTest\\R_Ability_Fire.glb"); //Attack_Fast,
    //models["enemy"]->AddAnimation("..\\glTF-Sample-Models-main\\original\\EnemyTest\\Jog_Fwd.glb"); //Jog_Fwd,
    enemy_timer.end();


    //models["boss"] = std::make_shared<GltfModel>(device, "..\\glTF-Sample-Models-main\\original\\Boss\\Idle.gltf"); //Idle,
    //models["enemy"] = std::make_shared<GltfModel>(device, ".\\resources\\Sitting Laughing.glb");
    //models["enemy"]->AddAnimation(".\\resources\\Taunt.glb");
    actorThread.join();
    stageThread.join();
    float elapsed_actor_time = actor_timer.end();
    float elapsed_stage_time = stage_timer.end();
    float elapsed_enemy_time = enemy_timer.end();

    char debugActorMessage[256];
    char debugStageMessage[256];
    char debugEnemyMessage[256];
    sprintf_s(debugActorMessage, sizeof(debugActorMessage), "Actor loading time = %.6f sec\n", elapsed_actor_time);
    sprintf_s(debugStageMessage, sizeof(debugStageMessage), "Stage loading time = %.6f sec\n", elapsed_stage_time);
    sprintf_s(debugEnemyMessage, sizeof(debugEnemyMessage), "Enemy loading time = %.6f sec\n", elapsed_enemy_time);
    //sprintf_s(aaa, sizeof(aaa), "time = %d\n", timeGetTime() - time);
    //time = timeGetTime();
    //OutputDebugStringA(aaa);
    OutputDebugStringA(debugActorMessage);
    OutputDebugStringA(debugStageMessage);
    OutputDebugStringA(debugEnemyMessage);
}

void MainScene::SetUpActors()
{
    //デバイスを取得
    ID3D11Device* device = Graphics::GetDevice();
    //デバイスコンテクストを取得
    ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();

    //gameWorld_->SpawnActor<Stage>("stage");
    auto actorManager = GetActorManager();
    stage = actorManager->CreateAndRegisterActor<Stage>("stage");
#if 0
    auto& stage_ = actors.emplace_back(stage);
    stage_->GetModelComponent().InitializeModel(models["stage"]);
    stage_->GetModelComponent().SetIsModelInMeter(true);
    stage_->isCastShadow = true;
    std::vector<GltfModelBase::Material>& materials = stage_->GetModelComponent().GetMaterial();
    for (GltfModelBase::Material& material : materials)
    {
        if (material.name == "Acid_24036")
        {
            CreatePsFromCSO(device, "./Shader/TestPS.cso", material.replacedPixelShader.GetAddressOf());
        }
    }
#endif
    Transform playerTr(DirectX::XMFLOAT3{ 0.7f,0.8f,-9.5f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    player = GetActorManager()->CreateAndRegisterActorWithTransform<Player>("actor", playerTr);
    //player = gameWorld_->SpawnActor<Player>("player");
    Transform enemyTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-180.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    enemies[0] = GetActorManager()->CreateAndRegisterActorWithTransform<RiderEnemy>("enemy", enemyTr);
    //enemies[0] = gameWorld_->SpawnActor<RiderEnemy>("enemy");
#if 0
    auto pickUpItem = ActorManager::CreateAndRegisterActor<PickUpItem>("pickUpItem");
    Transform bombTr(DirectX::XMFLOAT3{ 5.0f,10.0f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.1f,1.1f,1.1f });
    auto bomb = ActorManager::CreateAndRegisterActorWithTransform<Bomb>("bomb", bombTr);
#endif
    //Transform bombTr1(DirectX::XMFLOAT3{ -1.0f,10.0f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.1f,1.1f,1.1f });
    //auto bomb1 = ActorManager::CreateAndRegisterActorWithTransform<Bomb>("bomb", bombTr1);

    auto mainCameraActor = GetActorManager()->CreateAndRegisterActor<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();
    //auto mainCameraActor = gameWorld_->SpawnActor<Actor>("mainCameraActor");
    //auto springArmComponent = mainCameraActor->NewComponent<class SpringArmComponent>("springArm");
    ////springArmComponent->SetRelativeEulerRotationDirect({ 20.0f, 0.0f, 0.0f });
    //springArmComponent->SetRelativeEulerRotationDirect({ 30.0f, 0.0f, 0.0f });
    //springArmComponent->SetRelativeLocationDirect({ 1.5f,9.0f,-10.9f });
    //auto mainCameraComponent = mainCameraActor->NewComponent<class CameraComponent>("mainCamera", "springArm");
    //mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(45), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
    //CameraManager::SetGameCamera(mainCameraComponent);
    CameraManager::SetGameCamera(mainCameraActor.get());

    auto debugCameraActor = GetActorManager()->CreateAndRegisterActor<DebugCamera>("debugCam");
    //auto debugCameraActor = ActorManager::CreateAndRegisterActor<Actor>("debugCam");
    //auto debugCamera = debugCameraActor->NewSceneComponent<DebugCameraComponent>("debugCamera");
    CameraManager::SetDebugCamera(debugCameraActor);

    //Transform propTr(DirectX::XMFLOAT3{ 1.0f,0.0f,1.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto props = ActorManager::CreateAndRegisterActorWithTransform<VendingMachineProp>("vendingMachine", propTr);
    //Transform prop1Tr(DirectX::XMFLOAT3{ 1.0f,0.0f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto props1 = ActorManager::CreateAndRegisterActorWithTransform<TrafficLightProp>("trafficLightProp", prop1Tr);


#if 0
    Transform testTr(DirectX::XMFLOAT3{ 5.0f,-5.0f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto testCollision = ActorManager::CreateAndRegisterActorWithTransform<BossBuilding>("testCollision", testTr);
    auto testCollision = ActorManager::CreateAndRegisterActorWithTransform<Building>("testCollision", testTr);

    Transform test1Tr(DirectX::XMFLOAT3{ 2.0f,-5.0f,1.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto testCollision1 = ActorManager::CreateAndRegisterActorWithTransform<Building>("testCollision", test1Tr);
    //auto testCollision1 = ActorManager::CreateAndRegisterActorWithTransform<BossBuilding>("testCollision", test1Tr);

    Transform test2Tr(DirectX::XMFLOAT3{ 6.0f,-5.0f,1.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto testCollision2 = ActorManager::CreateAndRegisterActorWithTransform<Building>("testCollision",test2Tr);
    auto testCollision2 = ActorManager::CreateAndRegisterActorWithTransform<BossBuilding>("testCollision", test2Tr);
#endif // 0

#if 0
    //auto& boss = actors.emplace_back(std::make_shared<Actor>("boss"));
    //enemys[1] = std::make_shared<Boss>("boss");
    //auto& boss_ = actors.emplace_back(enemys[1]);
    //boss_->GetModelComponent().InitializeModel(models["boss"]);
    //boss_->GetModelComponent().SetIsModelInMeter(true);
    //boss_->GetModelComponent().SetIsAnimation(true);
    //boss_->isCastShadow = true;
    //std::vector<GltfModelBase::Material>& bossMaterials = boss_->GetModelComponent().GetMaterial();
    //for (GltfModelBase::Material& material : bossMaterials)
    //{//セルフシャドウを防ぐため
    //    CreatePsFromCSO(device, "./Shader/GltfModelCharacterPS.cso", material.replacedPixelShader.GetAddressOf());
    //}
#endif
}

