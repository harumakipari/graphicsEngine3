#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>

#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/Sprite/Sprite.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Resource/ShaderToy.h"

#include "Graphics/PostProcess/Bloom.h"
#include "Graphics/Shadow/ShadowMap.h"
#include "Graphics/Environment/SkyMap.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"
#include "Graphics/Effect/EffectSystem.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"

class LoadingScene : public Scene
{
    std::unique_ptr<Sprite> hit_space_key;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shaders[8];
    std::unique_ptr<FullScreenQuad> bit_block_transfer;
    std::string preload_scene;

    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT4 lightDirection;
        DirectX::XMFLOAT4 cameraPosition;
        DirectX::XMFLOAT4 colorLight;
        DirectX::XMFLOAT4X4 view;   // PARTICLES
        DirectX::XMFLOAT4X4 projection;   // PARTICLES
        // CASCADED_SHADOW_MAPS
        DirectX::XMFLOAT4X4 invProjection;
        DirectX::XMFLOAT4X4 invViewProjection;
        float iblIntensity;
        bool enableSSAO;
        float reflectionIntensity;
        float time = 0.0f;
        // shader のフラグ
        int enableCascadedShadowMaps;
        int enableSSR;
        int enableFog;
        int enableBloom;
        DirectX::XMFLOAT4X4 invView;
    };
    SceneConstants sceneConstants;
    struct ShaderConstants
    {
        float extraction_threshold{ 0.8f };
        float gaussian_sigma{ 1.0f };
        float bloom_intensity{ 1.0f };
        float exposure{ 1.0f };

        float shadowColor = 0.2f;
        float shadowDepthBias = 0.0005f;
        //float shadowDepthBias = 0.001f;
        bool colorizeCascadedlayer = false;
        float maxDistance = 15.0f;// SCREEN_SPACE_REFLECTION

        float resolution = 0.3f;// SCREEN_SPACE_REFLECTION
        int steps = 10;// SCREEN_SPACE_REFLECTION
        float thickness = 0.5f;// SCREEN_SPACE_REFLECTION
        float pad;

    };
    ShaderConstants shaderConstants;
    // FOG
    struct FogConstants
    {
        float fogColor[4] = { 1.000f,1.000f, 1.000f, 1.000f }; // w: fog intensuty

        float fogDensity = 0.02f;
        float fogHeightFalloff = 0.05f;
        float groundLevel = 0.0f;
        float fogCutoffDistance = 500.0f;

        float mieScatteringCoef = 0.55f;

        int enableDither = 1;
        int enableBlur = 1;

        float timeScale = 0.35f;
        float noiseScale = 0.2f;
        float pads[3];
    };
    FogConstants fogConstants;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.64f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 colorLight{ 1.0f,1.0f,1.0f,3.0f };
    float iblIntensity = 1.0f;  //Image Basesd Lightingの強度

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];
    //std::unique_ptr<RenderState> renderState;

    // フルスクリーンクアッドを使ったレンダーターゲットのブレンド・コピー処理
    std::unique_ptr<FullScreenQuad> fullscreenQuadTransfer;

    //ブルーム
    std::unique_ptr<Bloom> bloomer;

    // CASCADED_SHADOW_MAPS
    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
    //float criticalDepthValue = 0.0f; // If this value is 0, the camera's far panel distance is used.
    float criticalDepthValue = 62.0f; // If this value is 0, the camera's far panel distance is used.

    // MULTIPLE_RENDER_TARGETS
    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;

    std::unique_ptr<Sprite> splash;

    //スカイマップ
    std::unique_ptr<SkyMap> skyMap;

    std::unique_ptr<FrameBuffer> framebuffers[8];

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];

    std::unique_ptr<FullScreenQuad> frameBufferBlit;

    void SetUpActors();

    SIZE framebufferDimensions;

    // shaderToy
    //std::unique_ptr<ShaderToy> shaderToy;

    //std::unique_ptr<RenderState> renderingState;

    struct constants
    {
        float time = 0;
        float width;
        float height;
        //NOISE
        float seedScale = 5.0f;
    };

    std::unique_ptr<ConstantBuffer<constants>> cbuffer;
    //前のシーン
    std::unique_ptr<ID3D11ShaderResourceView> preSceneTexture;

    // ShaderToy で追加
    std::unique_ptr<FullScreenQuad> shaderToyTransfer; // ShadowToy用の
    std::unique_ptr<FrameBuffer> shaderToyFrameBuffer; // ShadowToy用の
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shaderToyPS;

    struct ShaderToyCB
    {
        DirectX::XMFLOAT4 iResolution;
        DirectX::XMFLOAT4 iMouse;
        DirectX::XMFLOAT4 iChannelResolution[4];
        float iTime;
        float iFrame;
        float iPad0;
        float iPad1;
    };
    ShaderToyCB shaderToy;
    Microsoft::WRL::ComPtr<ID3D11Buffer> shaderToyConstantBuffer;

public:
    size_t type = 1;
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Update(ID3D11DeviceContext* immediate_context, float delta_time) override;

    float time = 0;
    void Start() override;

    void Render(ID3D11DeviceContext* immediate_context, float delta_time) override;

    bool Uninitialize(ID3D11Device* device) override;

    bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;

    void DrawGui() override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<LoadingScene> _autoenrollment;

private:
    std::shared_ptr<EmptyEnemy> enemy = nullptr;

    std::shared_ptr<TitleCamera> mainCameraActor = nullptr;

    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    DirectX::XMFLOAT3 cameraTarget = { 0.0f,0.0f,0.0f };


    // SCREEN_SPACE_AMBIENT_OCCLUSION
    bool enableSSAO = true;
    bool enableCascadedShadowMaps = true;
    bool enableSSR = true;
    bool enableFog = false;
    bool enableBloom = true;

    // SCREEN_SPACE_REFLECTION
    //float refrectionIntensity = 1.0f;
    float refrectionIntensity = 0.1f;
    float maxDistance = 15.0f;
    float resolution = 0.3f;
    int steps = 10;
    float thickness = 0.5f;

    Renderer actorRender;
};