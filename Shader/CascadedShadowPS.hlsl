#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

SamplerState samplerStates[5] : register(s0);
SamplerComparisonState comparisonSamplerstate : register(s7);

Texture2D colorMap : register(t0);
Texture2D depthMap : register(t2); //t1  bloom
Texture2DArray cascadedShadowMaps : register(t3);


float4 main(VS_OUT pin) : SV_TARGET
{
    float4 sampledColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    float3 color = sampledColor.rgb;
    float alpha = sampledColor.a;
    
    float depthNdc = depthMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
    // ndc to world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
    
    
    
    // Apply vasvaded shadow mapping
    // Find alayer of cascaded view frustum volume
    float depthViewSpace = positionViewSpace.z;
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depthViewSpace)
        {
            cascadeIndex = layer;
            break;
        }
    }
    float shadowFactor = 1.0;
    if (cascadeIndex > -1)
    {
        // world space to loght view clip space, and to ndc
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w;
        // ndc to texture space
        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
        
        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisonSamplerstate, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
        
        float3 layerColor = 1;
#if 1
        if (colorizeCascadedLayer)
        {
            const float3 layerColors[4] =
            {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
                { 1, 1, 0 },
            };
            layerColor = layerColors[cascadeIndex];
        }
#endif
        color *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
    }
    
    
    
#if 1
    // Tone mapping : HDR -> SDR
    const float exposure = 1.2;
    color = 1 - exp(-color * exposure);
#endif
    
#if 1
    // Gamma process
    const float GAMMA = 2.2;
    color = pow(color, 1.0 / GAMMA);
#endif
    
    return float4(color, alpha);
    return 1;
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}