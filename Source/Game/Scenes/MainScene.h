#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <vector>

#include "Graphics/Sprite/Sprite.h"
#include "Graphics/Sprite/SpriteBatch.h"
#include "Graphics/PostProcess/FrameBuffer.h"
#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/Core/RenderState.h"

#include "Graphics/Resource/GltfModelBase.h"
#include "Graphics/Renderer/ShapeRenderer.h"

#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Resource/Texture.h"


#include "Graphics/PostProcess/Bloom.h"
#include "Graphics/Shadow/ShadowMap.h"
#include "Graphics/Environment/SkyMap.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"

#include "Core/Actor.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/Stage.h"
#include "Game/Actors/Enemy/RiderEnemy.h"

#include "Physics/CollisionMesh.h"
#include "Graphics/Effect/Particles.h"
#include "Graphics/Cloud/VolumetricCloudscapes.h"
#include "Graphics/Core/Light.h"
#include "Graphics/PostProcess/GBuffer.h"

#include "Graphics/Effect/EffectSystem.h"

#include "Core/ActorManager.h"
#include "Core/World.h"

#include "Utils/EasingHandler.h"

class MainScene : public Scene
{
    std::unique_ptr<Sprite> sprites[8];
    std::unique_ptr<SpriteBatch> sprite_batches[8];

    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 viewProjection;
        //DirectX::XMFLOAT4 lightDirection;
        DirectX::XMFLOAT4 cameraPosition;
        //DirectX::XMFLOAT4 colorLight;
        DirectX::XMFLOAT4X4 view;   // PARTICLES
        DirectX::XMFLOAT4X4 projection;   // PARTICLES
        // CASCADED_SHADOW_MAPS
        DirectX::XMFLOAT4X4 invProjection;
        DirectX::XMFLOAT4X4 invViewProjection;
        //float iblIntensity;
        bool enableSSAO;
        float reflectionIntensity;
        float time = 0.0f;
        // shader のフラグ
        int enableCascadedShadowMaps;
        int enableSSR;
        int enableFog;
        int enableBloom;
        float pad;
        DirectX::XMFLOAT4X4 invView;
    };
    SceneConstants sceneConstants;

    struct PointLights
    {
        DirectX::XMFLOAT4 position{ 0.0f,0.0f,0.0f,0.0f };
        DirectX::XMFLOAT4 color{ 1.0f,0.0f,0.0f,1.0f };
        float range = 0.5f;
        float pads[3];
    };
    DirectX::XMFLOAT4 pointLightPosition[8] =
    {
        { -2.0f,  2.0f, 0.0f, 10.0f },
        { -1.0f,  2.0f, 0.0f, 10.0f },
        { 0.0f,  2.0f, 0.0f, 10.0f },
        { 1.0f,  2.0f, 0.0f, 10.0f },
        { 2.0f,  2.0f, 0.0f, 10.0f },
        { 3.0f,  2.0f, 0.0f, 10.0f },
        { 4.0f,  2.0f, 0.0f, 10.0f },
        { 5.0f,  2.0f, 0.0f, 10.0f },
    };

    DirectX::XMFLOAT4 pointLightColor[8] =
    {
        { 1.0f, 0.0f, 0.0f, 10.0f },  // 赤
        { 0.0f, 1.0f, 0.0f, 10.0f },  // 緑
        { 0.0f, 0.0f, 1.0f, 10.0f },  // 青
        { 1.0f, 1.0f, 0.0f, 10.0f },  // 黄
        { 1.0f, 0.0f, 1.0f, 10.0f },  // マゼンタ
        { 0.0f, 1.0f, 1.0f, 10.0f },  // シアン
        { 1.0f, 0.5f, 0.0f, 10.0f },  // オレンジ
        { 0.5f, 0.0f, 1.0f, 10.0f },  // 紫
    };

    float pointLightRange[8] =
    {
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
    };
    bool directionalLightEnable = true; // 平行光源の on / off
    bool pointLightEnable = true;
    int pointLightCount = 8;
    struct SpotLights
    {

    };

    struct LightConstants
    {
        DirectX::XMFLOAT4 lightDirection;
        DirectX::XMFLOAT4 colorLight;
        float iblIntensity;
        int directionalLightEnable = 1; // 平行光源の on / off
        int pointLightEnable = 1;
        int pointLightCount = 1;
        PointLights pointsLight[8];
    };
    LightConstants lightConstants = {};
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

    // ScreenSpaceProjectionMapping
    static constexpr int MAX_PROJECTION_MAPPING = 32;
    struct ProjectionMappingConstants
    {
        DirectX::XMFLOAT4X4 projectionMappingTransforms[MAX_PROJECTION_MAPPING]{};
        DirectX::XMUINT4 enableMapping[MAX_PROJECTION_MAPPING / 4]{};
        DirectX::XMUINT4 textureId[MAX_PROJECTION_MAPPING / 4]{};
    };
    ProjectionMappingConstants projectionMappingConstants;

    DirectX::XMFLOAT3 eyes[MAX_PROJECTION_MAPPING];
    DirectX::XMFLOAT3 targets[MAX_PROJECTION_MAPPING];
    float distance = 20.0f;
    float rotations[MAX_PROJECTION_MAPPING];
    float fovY = 10.0f;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> screenSpaceProjectionMappingPixelShader;


    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

    //DirectX::XMFLOAT4 lightDirection{ 0.4f, -0.8f, -0.4f, 0.0f };
    //DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.64f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightDirection{ -0.75f, -1.0f, 0.894f, 0.0f };
    DirectX::XMFLOAT4 colorLight{ 1.0f,1.0f,1.0f,3.1f };
    float iblIntensity = 1.0f;  //Image Basesd Lightingの強度
    //float iblIntensity = 4.5f;  //Image Basesd Lightingの強度

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

    std::unique_ptr<FrameBuffer> framebuffers[8];

    // フルスクリーンクアッドを使ったレンダーターゲットのブレンド・コピー処理
    std::unique_ptr<FullScreenQuad> fullscreenQuadTransfer;

    //std::unique_ptr<RenderState> renderingState;

    //ブルーム
    std::unique_ptr<Bloom> bloomer;

    // CASCADED_SHADOW_MAPS
    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
    //float criticalDepthValue = 0.0f; // If this value is 0, the camera's far panel distance is used.
    float criticalDepthValue = 62.0f; // If this value is 0, the camera's far panel distance is used.

    // MULTIPLE_RENDER_TARGETS
    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;

    //スカイマップ
    std::unique_ptr<SkyMap> skyMap;

    //当たり判定のメッシュ
    std::unique_ptr<CollisionMesh> collisionMesh;

    //デバックを描画
    //std::unique_ptr<ShapeRenderer> shapeRenderer = nullptr;

    //線分デバック用の配列
    ShapeRenderer::Type type = ShapeRenderer::Type::Segment;
    std::vector<DirectX::XMFLOAT3> points;

    // ShaderToy で追加
    //std::unique_ptr<FullScreenQuad> shaderToyTransfer; // ShadowToy用の
    //std::unique_ptr<FrameBuffer> shaderToyFrameBuffer; // ShadowToy用の
    //Microsoft::WRL::ComPtr<ID3D11PixelShader> shaderToyPS;

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
    std::unique_ptr<EffectSystem> effectSystem;
public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;
    void Start() override;
    void Update(ID3D11DeviceContext* immediate_context, float delta_time) override;
    void Render(ID3D11DeviceContext* immediate_context, float delta_time) override;
    bool Uninitialize(ID3D11Device* device) override;
    bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;
    void DrawGui() override;
private:
    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    //std::unique_ptr<World> gameWorld_;

    //画面内に描画されるもの
    std::vector<std::shared_ptr<Actor>> actors;

    //シーンで使うモデル
    std::map<std::string, std::shared_ptr<GltfModelBase>> models;

    //モデルをロード
    void LoadModel();

    //Actorをセット
    void SetUpActors();

    //パーティクルシステムセットする
    void SetParticleSystem();

    //プレイヤー
    std::shared_ptr<Player> player;

    //敵
    std::shared_ptr<RiderEnemy> enemies[1];

    // カメラシェイク
    float power = 0.01f;
    float timer = 0.17f;
public:
    DirectX::XMFLOAT4 imGuiNDC = { 0.0f,0.0f,0.0f,0.0f };



    //ステージ
    std::shared_ptr<Stage> stage;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];

    SIZE framebufferDimensions;

    //TODO:02IMGUI用
    DirectX::XMFLOAT3 spherePosition{ 2.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 playerDebugColor{ 1.0f,1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 enemyDebugColor{ 1.0f,1.0f,1.0f,1.0f };

    //前フレームの武器の座標
    DirectX::XMFLOAT3 preSpherePosition{ 0.0f,0.0f,0.0f };

    //武器と敵が当たったフラグ
    bool isHitWeapon = false;

    //プレイヤーと敵があたったフラグ
    bool isHitBody = false;

    //撃破イベント起動フラグ
    bool isBossDeath = false;
    bool isPlayerDeath = false;
    //フェード
    EasingHandler fadeHandler;
    float fadeValue = 0.0f;
    EasingHandler waitHandler;

    //パーティクル
    //bool integrateParticles{ true };
    //std::unique_ptr<ParticleSystem> particles;
    //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleTexture;
    //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorTemperChart;
    //int blendMode = 1; // 0:ALPHA, 1:ADD

    DirectX::XMFLOAT3 p1{ 0.0f,0.5f,0.0f };
    DirectX::XMFLOAT3 p2{ 1.0f,0.5f,0.0f };

    // シーンの自動登録
    static inline Scene::Autoenrollment<MainScene> _autoenrollment;

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

    // FOG
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise2d;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise3d;

    // VOLUMETRIC_CLOUDSCAPES
    //std::unique_ptr<VolumetricCloudscapes> volumetricCloudscapes;
    //int downsamplingFactor = 4;
    
    // GBUFFER
    std::unique_ptr<GBuffer> gBufferRenderTarget;
    bool useDeferredRendering = false;

    // VOLUMETRIC_CLOUDSCAPES
    DirectX::XMFLOAT4 cameraFocus{ 0.0f, 1.0f, 0.0f, 1.0f };

    Renderer actorRender;

    ActorColliderManager actorColliderManager;

    // ボスが画面外かどうか
    bool IsFrameOutEnemy()
    {
        return isFrameOutEnemy;
    }
    // カーソルの位置取得
    DirectX::XMFLOAT2 GetCursorIntersectPos()
    {
        return intersection;
    }
    // カーソルの度数
    float GetAngleCursorDegree()
    {
        return angleDegree;
    }
private:
    // 敵のスクリーン座標
    DirectX::XMFLOAT2 enemyScreenPosition = { 0.0f,0.0f };
    // プレイヤーのスクリーン座標
    DirectX::XMFLOAT2 playerScreenPosition = { 0.0f,0.0f };
    // カーソル描画の交点
    DirectX::XMFLOAT2 intersection = { 0.0f,0.0f };
    // カーソルの描画角度
    float angleDegree = 0.0f;
    // ボスが画面外かどうか
    bool isFrameOutEnemy = false;
};
