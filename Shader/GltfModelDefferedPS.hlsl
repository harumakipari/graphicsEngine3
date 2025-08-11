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
SamplerState samplerStates[5] : register(s0);

GBUFFER_PS_OUT main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) 
{
    GBUFFER_PS_OUT pout;
    const float GAMMA = 2.2;
    const MaterialConstants m = materials[material];

    float4 basecolorFactor = m.pbrMetallicRoughness.basecolorFactor;
    const int basecolorTexture = m.pbrMetallicRoughness.basecolorTexture.index;

    if (basecolorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        basecolorFactor *= sampled;
    }
    
    if (m.alphaMode == 0 /*OPAQUE*/)
    {
        basecolorFactor.a = 1.0;
    }
    if (basecolorFactor.a < m.alphaCutoff)
    {
        discard;
    }
    
    float3 emmisiveFactor = m.emissiveFactor;
    
    const int emissiveTexture = m.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[2], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;

    const float3 f0 = lerp(0.04, basecolorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(basecolorFactor.rgb, 0.0, metallicFactor);
    
    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPositon.xyz - pin.wPosition.xyz);
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    //For a back-facing surface, the tangential basis vectors are negated.
    if (isFrontFace == false)
    {
        T = -T;
        B = -B;
        N = -N;
    }
    
    const int normalTexture = m.normalTexture.index;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    pout.color = basecolorFactor;
    //pout.position = mul(pin.wPosition, view); // to viewSpace
    //pout.normal = mul(float4(N.xyz, 0), view); //to viewSpace;
    pout.position = pin.wPosition; // to viewSpace
    pout.normal = float4(N.xyz, 0); //to viewSpace;
    pout.emmisive = float4(emmisiveFactor, 0);
    pout.msr = float4(metallicFactor, occlusionFactor, roughnessFactor, occlusionStrength);
    
	return pout;
}