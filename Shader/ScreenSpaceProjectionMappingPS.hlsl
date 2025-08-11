#include "ScreenSpaceProjectionMapping.hlsli"
#include "FullScreenQuad.hlsli"
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
SamplerState sampler_states[5] : register(s0);

Texture2D sceneTexture : register(t20);
Texture2D depthTexture : register(t21);

Texture2D projectionMappingTexture[2] : register(t15);


float3 ScreenToWorld(float2 texcoord, float depth, row_major float4x4 invViewProj)
{
    float2 ndc = texcoord * 2.0 - 1.0;
    ndc.y = -ndc.y;
	
    float4 clipPos = float4(ndc.x, ndc.y, depth, 1.0);
	
    float4 worldPos = mul(clipPos, invViewProj);
    
    return worldPos.xyz / worldPos.w;
}

float4 ProjectionTextureSample(int texture_id, SamplerState samplerState, float2 texcoord)
{
    switch (texture_id)
    {
        case 0: return projectionMappingTexture[0].Sample(samplerState, texcoord);
        case 1: return projectionMappingTexture[1].Sample(samplerState, texcoord);
    }
    return 0;
}

float3 ProjectionMapping(float4 worldPosition, row_major float4x4 projectionMappingTransform, SamplerState samplerState, int texture_id)
{
    // PROJECTION_MAPPING
    const float colorIntensity = 10;
    float3 projectionMappingColor = 0;
    float4 projectionTexturePosition = mul(worldPosition, projectionMappingTransform);
    projectionTexturePosition /= projectionTexturePosition.w;
    projectionTexturePosition.x = projectionTexturePosition.x * 0.5 + 0.5;
    projectionTexturePosition.y = -projectionTexturePosition.y * 0.5 + 0.5;
    if (saturate(projectionTexturePosition.z) == projectionTexturePosition.z)
    {
        float4 projectionTextureColor = ProjectionTextureSample(texture_id, samplerState, projectionTexturePosition.xy);
        projectionMappingColor = projectionTextureColor.rgb * projectionTextureColor.a * colorIntensity;
    }
    return projectionMappingColor;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float depth = depthTexture.Sample(sampler_states[LINEAR], pin.texcoord).r;
    float3 worldPosition = ScreenToWorld(pin.texcoord, depth, inverseViewProjection);
    float3 color = sceneTexture.Sample(sampler_states[LINEAR], pin.texcoord).rgb;
    
    [unroll]
    for (int i = 0; i < MAX_PROJECTION_MAPPING; i++)
    {
        if (enableMapping[i / 4][i % 4])
        {
            int t = textureId[i / 4][i % 4];
            color += ProjectionMapping(float4(worldPosition, 1.0), projectionMappingTransforms[i], sampler_states[LINEAR_BORDER_BLACK], t);
        }
    }
    
    return float4(color, 1.0);
}