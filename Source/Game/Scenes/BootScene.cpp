#include "BootScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/TitleUIFactory.h"

bool BootScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
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

    //Glitch
    bufferDesc.ByteWidth = sizeof(spriteConstants);
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

    // GBUFFER
    gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
    CreatePsFromCSO(device, "./Shader/DefefferdPS.cso", pixelShaders[1].ReleaseAndGetAddressOf());

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
    LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_sheen_e.dds", shaderResourceViews[3].ReleaseAndGetAddressOf(), &texture2dDesc);

    //splash = std::make_unique<Sprite>(device, L"./Data/Textures/Screens/TitleScene/994759-1.jpg");

    //アクターをセット
    SetUpActors();

    EventSystem::Initialize();//追加 UI

    return true;
}

void BootScene::Start()
{
    TitleUIFactory::Create(this);

    auto titleButtonObj = objectManager.FindGameObject("TitleButton");
    auto titleButton = titleButtonObj->GetComponent<Button>();
    titleButton->AddOnClickEvent([&]()
        {
            //titlePlayer->PlayAnimation("Rotation", false);
            titlePlayer->OnPushStart();
            mainCameraActor->OnClick();
        });

    auto backButtonObj = objectManager.FindGameObject("BackToTitle");
    auto backButton = backButtonObj->GetComponent<Button>();
    backButton->AddOnClickEvent([&]()
        {
            titlePlayer->OnPushBackToTitle();
            title->OnPushBackToTitle();
        });

    //タイトルBGM
    UIFactory::SetObjectManager(&objectManager);
    AudioSource* titleBgm = UIFactory::Create("TitleBGM")->AddComponent<AudioSource>(L"./Data/Sound/BGM/title.wav");
    titleBgm->SetVolume(0.5f);
    titleBgm->Play(XAUDIO2_LOOP_INFINITE);
}

void BootScene::Update(ID3D11DeviceContext* immediate_context, float deltaTime)
{
    //ActorManager::Update(deltaTime);
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加

    mainCameraActor->SetTarget(cameraTarget);
    if (InputSystem::GetInputState("F8", InputStateMask::Trigger))
    {
        CameraManager::ToggleCamera();
    }
#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
    }
#endif // !_DEBUG

    //if (InputSystem::GetInputState("Enter", InputStateMask::Trigger))
    //{
    //    const char* types[] = { "0", "1" };
    //    Scene::_transition("LoadingScene", { std::make_pair("preload", "TutorialScene"), std::make_pair("type", types[rand() % 2]) });
    //}

#ifdef USE_IMGUI
    ImGui::Begin("Title Scene");
    ImGui::DragFloat3("cameraTarget", &cameraTarget.x, 0.2f);
    ImGui::End();
#endif
}

void BootScene::SetUpActors()
{
    Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //titlePlayer = ActorManager::CreateAndRegisterActorWithTransform<TitlePlayer>("actor", playerTr);
    titlePlayer = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitlePlayer>("actor", playerTr);

    mainCameraActor = this->GetActorManager()->CreateAndRegisterActor<TitleCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();

    Transform titleTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    title = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleStage>("title", titleTr);

    //CameraManager::SetGameCamera(mainCameraComponent);
    CameraManager::SetGameCamera(mainCameraActor.get());

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActor<DebugCamera>("debugCam");
    //auto debugCameraActor = ActorManager::CreateAndRegisterActor<Actor>("debugCam");
    //auto debugCamera = debugCameraActor->NewSceneComponent<DebugCameraComponent>("debugCamera");

    //Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-35.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-15.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto enemy = ActorManager::CreateAndRegisterActorWithTransform<EmptyEnemy>("enemy", enemyTr);
    CameraManager::SetDebugCamera(debugCameraActor);
}

bool BootScene::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
{
    framebufferDimensions.cx = static_cast<LONG>(width);
    framebufferDimensions.cy = height;

    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    //MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);

    framebuffers[0] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx / downsamplingFactor, framebufferDimensions.cy / downsamplingFactor);
    //framebuffers[1] = std::make_unique<FrameBuffer>(device, framebufferDimensions.cx, framebufferDimensions.cy, true);

    //ブルーム
    bloomer = std::make_unique<Bloom>(device, framebufferDimensions.cx, framebufferDimensions.cy);

    return true;
}

bool BootScene::Uninitialize(ID3D11Device* device)
{
    //ActorManager::ClearAll();
    return true;
}

void BootScene::Render(ID3D11DeviceContext* immediateContext, float delta_time)
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
        sceneConstants.view =data.view;
        sceneConstants.projection = data.projection;

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&data.projection);
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&data.view);
        DirectX::XMStoreFloat4x4(&sceneConstants.viewProjection, V * P);

        // CASCADED_SHADOW_MAPS
        DirectX::XMStoreFloat4x4(&sceneConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        DirectX::XMStoreFloat4x4(&sceneConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        // COMPUTE_PARTICLE_SYSTEM
        DirectX::XMStoreFloat4x4(&sceneConstants.invView, DirectX::XMMatrixInverse(NULL, V));
#if 0
        //DirectX::XMFLOAT3 cameraPosition = camera->GetWorldPosition();
        DirectX::XMFLOAT3 cameraPosition = camera->GetComponentWorldTransform().GetTranslation();
        sceneConstants.cameraPosition = { cameraPosition.x,cameraPosition.y,cameraPosition.z,1.0f };
        sceneConstants.view = camera->GetView();
        sceneConstants.projection = camera->GetProjection();
        //sceneConstants.viewProjection = camera->GetViewProjection();

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&camera->GetProjection());
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&camera->GetView());
        DirectX::XMStoreFloat4x4(&sceneConstants.viewProjection, V * P);

        // CASCADED_SHADOW_MAPS
        DirectX::XMStoreFloat4x4(&sceneConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        DirectX::XMStoreFloat4x4(&sceneConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        // COMPUTE_PARTICLE_SYSTEM
        DirectX::XMStoreFloat4x4(&sceneConstants.invView, DirectX::XMMatrixInverse(NULL, V));

#endif // 0
        actorRender.UpdateViewConstants(immediateContext, data);
        sceneRender.UpdateViewConstants(immediateContext, data);
    }
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
    //sceneConstants.lightDirection = lightDirection;
    //sceneConstants.colorLight = colorLight;
    //sceneConstants.iblIntensity = iblIntensity;
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

    immediateContext->UpdateSubresource(constantBuffers[4].Get(), 0, 0, &lightConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(11, 1, constantBuffers[4].GetAddressOf());    //3 は cascadedShadowMap に使用中

    spriteConstants.elapsedTime += delta_time;
    spriteConstants.enableGlitch = 0;
    immediateContext->UpdateSubresource(constantBuffers[3].Get(), 0, 0, &spriteConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(10, 1, constantBuffers[3].GetAddressOf());

    //titlePlayer->SwitchPS(useDeferredRendering);
    //title->SwitchPS(useDeferredRendering);
    if (!useDeferredRendering)
    {
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
        //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_BACK);

        // MULTIPLE_RENDER_TARGETS
        //RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        sceneRender.currentRenderPath = RenderPath::Forward;
        sceneRender.RenderOpaque(immediateContext);
        sceneRender.RenderMask(immediateContext);
        sceneRender.RenderBlend(immediateContext);
        //actorRender.RenderOpaque(immediateContext);
        //actorRender.RenderMask(immediateContext);
        //actorRender.RenderBlend(immediateContext);

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
        //actorRender.CastShadowRender(immediateContext);
        //gameWorld_->CastShadowRender(immediateContext);
        cascadedShadowMaps->Deactive(immediateContext);

#if 0
        // FOG
        {
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
        }

#endif // 0

        // CASCADED_SHADOW_MAPS
        // Draw shadow to scene framebuffer
        // FINAL_PASS
        {
            bloomer->bloom_intensity = bloomIntensity;
            bloomer->bloom_extraction_threshold = bloomThreshold;
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
    }
    else
    {
        gBufferRenderTarget->Clear(immediateContext);
        gBufferRenderTarget->Acticate(immediateContext);

        // SKY_MAP
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        skyMap->Blit(immediateContext, sceneConstants.viewProjection);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        // MULTIPLE_RENDER_TARGETS
        //RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

        sceneRender.currentRenderPath = RenderPath::Defferd;
        sceneRender.RenderOpaque(immediateContext);
        sceneRender.RenderMask(immediateContext);
        sceneRender.RenderBlend(immediateContext);

        //actorRender.RenderOpaque(immediateContext);
        //actorRender.RenderMask(immediateContext);
        //actorRender.RenderBlend(immediateContext);

        gBufferRenderTarget->Deactivate(immediateContext);

        // MULTIPLE_RENDER_TARGETS
#if 1
        multipleRenderTargets->Clear(immediateContext);
        multipleRenderTargets->Acticate(immediateContext);
#endif
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
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);


        // ここでライティングの処理
        ID3D11ShaderResourceView* shaderResourceViews[]
        {
            // MULTIPLE_RENDER_TARGETS
            gBufferRenderTarget->renderTargetShaderResourceViews[0],  // normalMap
            gBufferRenderTarget->renderTargetShaderResourceViews[1],   // msrMap
            gBufferRenderTarget->renderTargetShaderResourceViews[2],   // colorMap
            gBufferRenderTarget->renderTargetShaderResourceViews[3],   // positionMap
            gBufferRenderTarget->renderTargetShaderResourceViews[4],   // emissiveMap
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuadTransfer->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), pixelShaders[1]/*DefefferdPS*/.Get());
        //actorRender.RenderBlend(immediateContext);




#if 1
        multipleRenderTargets->Deactivate(immediateContext);
#endif

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
        //actorRender.CastShadowRender(immediateContext);
        //gameWorld_->CastShadowRender(immediateContext);
        cascadedShadowMaps->Deactive(immediateContext);

#if 0
        // FOG
        {
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
        }

#endif // 0

#if 1
        // CASCADED_SHADOW_MAPS
        // Draw shadow to scene framebuffer
        // FINAL_PASS
        {
            //bloomer->bloom_intensity = bloomIntensity;
            //bloomer->bloom_extraction_threshold = bloomThreshold;
            //ブルーム
            RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
            RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
            RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
            //bloomer->make(immediateContext, gBufferRenderTarget->renderTargetShaderResourceViews[0]);
            bloomer->make(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[0]);
            //bloomer->make(immediateContext, framebuffers[1]->shaderResourceViews[0].Get());

            ID3D11ShaderResourceView* shader_resource_views[]
            {
                // MULTIPLE_RENDER_TARGETS
                multipleRenderTargets->renderTargetShaderResourceViews[0],  //colorMap
                //gBufferRenderTarget->renderTargetShaderResourceViews[0],  //colorMap
                //framebuffers[1]->shaderResourceViews[0].Get(),
    #if 0
                multipleRenderTargets->renderTargetShaderResourceViews[1],  // positionMap
                multipleRenderTargets->renderTargetShaderResourceViews[2],  // normalMap
    #else
                gBufferRenderTarget->renderTargetShaderResourceViews[3],   // positionMap
                gBufferRenderTarget->renderTargetShaderResourceViews[0],   // normalMap
    #endif // 0
                //multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
                gBufferRenderTarget->depthStencilShaderResourceView,      //depthMap
                bloomer->shader_resource_view(),    //bloom
                framebuffers[0]->shaderResourceViews[0].Get(),  //fog
                cascadedShadowMaps->depthMap().Get(),   //cascaededShadowMaps
            };
            // メインフレームバッファとブルームエフェクトを組み合わせて描画
            fullscreenQuadTransfer->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), pixelShaders[0]/*final pass*/.Get());

        }
    }
#endif // 0

    // UI描画
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        objectManager.Draw(immediateContext);
    }
}

void BootScene::DrawGui()
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

    for (auto& actor : this->GetActorManager()->GetAllActors()) {
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

                //ImGui::DragFloat4("Point Light Position", &pointLightPosition.x, 0.1f);
                //ImGui::ColorEdit3("Point Light Color", &pointLightColor.x);
                //ImGui::SliderFloat("Point Light Range", &pointLightRange, 0.0f, 10.0f);
                //ImGui::SliderFloat("Point Light Threshold", &pointLightColor.w, 0.0f, 10.0f);
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
