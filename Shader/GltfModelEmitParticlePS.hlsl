#include "ComputeParticle.hlsli"
#include "GltfModel.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

RWStructuredBuffer<EmitParticleData> emitParticleBuffer : register(u0); //パーティクル生成情報バッファ
RWByteAddressBuffer indirectDataBuffer : register(u1); //インダイレクト用バッファ

float rand(float t)
{
    return frac(sin(dot(float2(t, t), float2(12.9898f, 78.233f))) * 43758.5453123f);
}

float2 rand2(float2 s)
{
    s = float2(dot(s, float2(12.9898f, 78.233f)), dot(s, float2(269.5f, 183.3f)));
    return frac(sin(s) * float2(43758.5453123f, 43758.5453123f));
}

void main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace)
{
    const float GAMMA = 2.2f;
    
    MaterialConstants m = materials[material];
    
    //ベースカラーを取得
    float4 baseColor = m.pbrMetallicRoughness.basecolorFactor;
    if (m.pbrMetallicRoughness.basecolorTexture.index > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColor *= sampled;
    }
    
    //適当にいじくってみる
    //pin.wPosition += pin.wNormal * deltaTime;
    
    //ピクセルパーティクルカウンターに加算
    uint particleIndex = 0;
    indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumEmitPixelParticleIndex, 1, particleIndex);
    particleIndex += totalEmitCount;
    emitParticleBuffer[particleIndex].parameter.x = 12;
    emitParticleBuffer[particleIndex].parameter.y = rand(time) * 2.0f + 1.0f;
    
    emitParticleBuffer[particleIndex].position.xyz = pin.wPosition.xyz;
    emitParticleBuffer[particleIndex].position.w = 1;
    emitParticleBuffer[particleIndex].rotation = float4(0, 0, 0, 1);
    emitParticleBuffer[particleIndex].scale.xyz = float3(0.1f, 0.1f, 0.1f);
    
    emitParticleBuffer[particleIndex].velocity = 0;
    
    emitParticleBuffer[particleIndex].acceleration = 0;
    emitParticleBuffer[particleIndex].acceleration.y = 0.1f;
    
    emitParticleBuffer[particleIndex].color = baseColor;
    
}