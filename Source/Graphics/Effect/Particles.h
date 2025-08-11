#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>

#define NUMTHREAD_X 16

struct Particle
{
    // 粒子の状態
    int state = 0;
    // 粒子の色
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
    // 粒子の位置
    DirectX::XMFLOAT3 position{ 0,0,0 };
    // 粒子の質量
    float mass = 1.0f;
    // 粒子の回転角度 (ラジアン)
    float angle = 0.0f;
    // 粒子の角速度
    float angularSpeed = 0.0f;
    // 粒子の速度 (ベクトル形式)
    DirectX::XMFLOAT3 velocity{ 0,0,0 };
    // 粒子の寿命 (秒単位)
    float lifespan = 1.0f;
    // 現在の粒子の経過時間
    float age = 0.0f;
    // 粒子のサイズ (生成時と消滅時のサイズ)
    DirectX::XMFLOAT2 size{ 0 };// x : spawn, y: despawn
    // テクスチャの選択用インデックス
    int chip = 0;
};

struct ParticleSystem
{
    // 粒子の最大数
    const int maxParticleCount;
    // 粒子システムの定数バッファ用構造体
    struct ParticleSystemConstants
    {
        // 粒子の生成位置
        DirectX::XMFLOAT4 emissionPosition{};
        // 生成位置の半径範囲 (x: 最小, y: 最大)
        DirectX::XMFLOAT2 emissionOffset{ 0.0f,0.0f }; // x: minimum radius , y: maximum radius 
        // 粒子の生成サイズと消滅サイズ
        DirectX::XMFLOAT2 emissionSize{ 0.02f,0.5f }; // x: spawn, y: despawn
        // 生成角度範囲 (ラジアン, x: 最小, y: 最大)
        DirectX::XMFLOAT2 emissionConeAngle{ 0.0f,0.2f }; // x: minimum radian, y: maximum radian
        // 粒子の生成速度範囲
        DirectX::XMFLOAT2 emissionSpeed{ 0.5f,1.0f }; // x:minimum speed, y: maximum speed
        // 角速度の範囲
        DirectX::XMFLOAT2 emissionAngularSpeed{ 0.0f,1.0f }; // x: minimum angular speed, y: maximum angular speed
        // 粒子の寿命範囲
        DirectX::XMFLOAT2 lifespan{ 2.2f,2.2f }; // x: minimum second, y: maximum second
        // 粒子生成の遅延時間範囲
        //DirectX::XMFLOAT2 spawnDelay{ 0.0f,/*3.8f*/ 0.55f}; // x: minimum second, y: maximum second
        DirectX::XMFLOAT2 spawnDelay{ 0.1362f,/*3.8692f*/4.4777f }; // x: minimum second, y: maximum second
        // フェードの時間範囲 (x: フェードイン, y: フェードアウト)
        DirectX::XMFLOAT2 fadeDuration{ 0.0f,0.63f }; // x: fade in, y: fade out
        
        //色変化
        DirectX::XMFLOAT4 emissionStartColor{ 1,1,1,1 };
        DirectX::XMFLOAT4 emissionEndColor{ 1,1,1,1 };

        // 現在の時間
        float time = 0.0f;
        // フレーム間の経過時間
        float deltaTime = 0.0f;
        // ノイズスケール 
        float noiseScale = 0.001f;
        // 重力
        //float gravity = -0.1f;
        float gravity = 0.17f;
        // スプライトシートのグリッドサイズ 
        DirectX::XMUINT2 spriteSheetGrid{ 1,1 };
        // 粒子の最大数 (再確認用)
        int maxParticleCount = 0;
        int state = -1; // 0 : starting 1: finished  2: テスト用
        //float pads[1]; //(調整用)
        //粒子が進む方向
        DirectX::XMFLOAT3 direction{ 0,1,0 };// Z 軸の方向
        //粒子が進む速さ
        float strength = 0.6f;
        //ワールドトランスフォーム
        DirectX::XMFLOAT4X4 nodeWorldTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        //ワールドトランスフォーム
        DirectX::XMFLOAT4X4 worldTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        //円錐の半径
        DirectX::XMFLOAT2 radius{ 0,0.1f };

        bool loop = false;
        bool none[3]{};
        int type = 0;//パーティクルの種類
        int isStatic = 0;
        int pad__[3];
    };
    int blendMode = 1; // 0:ALPHA, 1:ADD
    ParticleSystemConstants particleSystemData;
    ParticleSystemConstants presetData;

    //パーティクルテクスチャ
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleTexture;

    // GPUリソース (粒子データを保持するバッファ)
    Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv;
    // GPUシェーダ (頂点シェーダ、ピクセルシェーダ、ジオメトリシェーダなど)
    Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGeometricShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleComputeShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerComputeShader;
    // 定数バッファ (CPUからGPUにデータを送るため)
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    ParticleSystem(ID3D11Device* pDevice, int particleCount);
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) noexcept = delete;
    ParticleSystem& operator=(ParticleSystem&&) noexcept = delete;
    virtual ~ParticleSystem() = default;
    // 粒子の動きを統合する関数
    void Integrate(ID3D11DeviceContext* immediateContext, float deltaTime);
    // 粒子の初期化関数
    void Initialize(ID3D11DeviceContext* immediateContext, float deltaTime);
    // 粒子を描画する関数
    void Render(ID3D11DeviceContext* immediateContext);

    //GUI描画
    void DrawGUI();

};