#ifndef EFFECT_SYSTEM_H
#define EFFECT_SYSTEM_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <d3d11.h>

#include "Graphics/Effect/Particles.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Resource/Texture.h"

//仮
#include "Graphics/Resource/GltfModel.h"

#include "ComputeParticleSystem.h"
class EffectComponent;

class EffectSystem
{
public:
    Microsoft::WRL::ComPtr<ID3D11PixelShader> dissolvePixelShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> meshParticlePixelShader;

    std::unique_ptr<ComputeParticleSystem> computeParticles[10];
    struct EmitterData
    {
        int count;//パーティクル生成数
        int blendMode = 1; // 0:ALPHA, 1:ADD
        ParticleSystem::ParticleSystemConstants data;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    };
    std::vector<EmitterData> presets;
public:
    EffectSystem() = default;
    virtual ~EffectSystem() = default;

    void Initialize();

    void Update(float deltaTime);
    void Render(ID3D11DeviceContext* immediateContext);

    ParticleSystem* SpawnEmitter(EmitterData data) {
        std::unique_ptr<ParticleSystem> effect = std::make_unique<ParticleSystem>(Graphics::GetDevice(), data.count);
        effect->particleTexture = data.texture;
        effect->blendMode = data.blendMode;
        effect->particleSystemData = data.data;
        return particles.emplace_back(std::move(effect)).get();
    }

    ParticleSystem* CreateEmitter(int count) {
        std::unique_ptr<ParticleSystem> effect = std::make_unique<ParticleSystem>(Graphics::GetDevice(), count);
        MakeDummyTexture(Graphics::GetDevice(), effect->particleTexture.GetAddressOf(), 0xFFFFFFFF, 16);
        return particles.emplace_back(std::move(effect)).get();
    }

    void SpawnEmitter(EffectComponent* effectComponent);

    //ボスチャージ用
    void SpawnEmitter(int type, DirectX::XMFLOAT3 position, float power);

    void ResultParticle(DirectX::XMFLOAT4 color);

    void DrawGUI();

    bool allParticleIntegrate{ false };

    //パーティクル
    bool integrateParticles{ false };

    

    std::vector<std::unique_ptr<ParticleSystem>> particles;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorTemperChart;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> projectionTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> maskTexture;

    //仮
    std::unique_ptr<GltfModel> model;
};

#endif //EFFECT_SYSTEM_H