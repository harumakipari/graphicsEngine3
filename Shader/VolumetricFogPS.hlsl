#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Lights.hlsli"

SamplerState pointSamplerState : register(s0);
SamplerState linearSamplerState : register(s1);
SamplerState anisotropicSamplerState : register(s2);
SamplerState linearBorderBlackSamplerState : register(s3);
SamplerState linearBorderWhiteSamplerState : register(s4);
SamplerState linearClampSamplerState : register(s5);
SamplerState linearMirrorSamplerState : register(s6);

SamplerComparisonState comparisonSamplerState : register(s7);

Texture2D colorTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2DArray cascadedShadowMaps : register(t2);

// NOISE
Texture3D noise3D : register(t10);

float SunlightRadiance(float3 position, VS_OUT pin)
{
    float depthNdc = depthTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).x;
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
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
    float4 p = mul(float4(position, 1.0), viewProjection); //world to clip space
    p = p / p.w; // clip to ndc
    // ndc to tecture coordinate
    p.x = p.x * 0.5 + 0.5;
    p.y = -p.y * 0.5 + 0.5;
    //return shadowTexture.SampleCmpLevelZero(comparisonSamplerState, p.xy, p.z - shadowDepthBias).x;
    return cascadedShadowMaps.SampleCmpLevelZero(comparisonSamplerState, float3(p.xy, cascadeIndex), p.z - shadowDepthBias).x;
}

void ApplyHeightFog(float3 position /*world space*/, inout float density)
{
    density *= exp(-(position.y - groundLevel) * fogHeightFalloff);
}

float MieScattering(float cosAngle, float g)
{
    return (1.0 / (4.0 * 3.14159265358979)) * ((1 - (g * g)) / (pow((1 + (g * g)) - (2 * g) * cosAngle, 1.5)));
}

float DitheredRayMarch(float2 screenPos, float3 rayStart, float3 rayDir, float rayLength, VS_OUT pin)
{
    float ditherValue = 0;
    if (enableDither)
    {
        const float4x4 ditherPattern =
        {
            { 0.0f, 0.5f, 0.125f, 0.625f },
            { 0.75f, 0.22f, 0.875f, 0.375f },
            { 0.1875f, 0.6875f, 0.0625f, 0.5625 },
            { 0.9375f, 0.4375f, 0.8125f, 0.3125 }
        };
        ditherValue = ditherPattern[screenPos.x % 4][screenPos.y % 4];
    }

    const int stepCount = 16;
    
    float stepSize = rayLength / stepCount;
    float3 step = rayDir * stepSize;
    
    float currentPosition = rayStart + step * ditherValue;
    
    float extinction = 0;
    float accumulatedRadiance = 0;
    [loop]
    for (int i = 0; i < stepCount; ++i)
    {
        float radiance = SunlightRadiance(currentPosition, pin);
        
        float density = fogDensity;
        
   #if 0 
        const float3 noiseVelocity = normalize(float3(1, 0, 0));
        float3 noiseSamplePosition = frac(currentPosition * noiseScale + noiseVelocity * time * timeScale);
        float noise = 0.5 * noise3D.Sample(linearSamplerState, noiseSamplePosition);
        density *= noise;
   #endif
        
        ApplyHeightFog(currentPosition, density);
        
        const float scatteringCoef = 0.815f;
        const float extinctionCoef = 0.0031f;
        float scattering = scatteringCoef * stepSize * density;
        extinction += extinctionCoef * stepSize * density;
        
        accumulatedRadiance += radiance * scattering * exp(-extinction);
        
        currentPosition += step;
    }
    
    const float cosAngle = dot(normalize(lightDirection.xyz), -rayDir);
    accumulatedRadiance *= MieScattering(cosAngle, mieScatteringCoef);
    
    return accumulatedRadiance;
}

float main(VS_OUT pin) : SV_TARGET
{
    const float steps = 16;
    float depth = depthTexture.Sample(pointSamplerState, pin.texcoord).x;
    float4 position = mul(float4(pin.texcoord.x * 2.0 - 1.0, -pin.texcoord.y * 2.0 + 1.0, depth, 1.0), inverseViewProjection);
    position = position / position.w; // world space
    
    float3 rayStart = cameraPositon.xyz;
    float3 rayDir = position.xyz - rayStart;
    float rayLength = length(rayDir);
    rayDir /= rayLength;
    
#if 1
    const float maxRayLength = fogCutoffDistance;
    rayLength = min(rayLength, maxRayLength);
#endif
    
    //float4 color = colorTexture.Sample(linearBorderBlackSamplerState, pin.texcoord);
    //float alpha = color.a;

    
    //float fogFactor = DitheredRayMarch(pin.position.xy, rayStart, rayDir, rayLength, pin);
    
    //color.rgb += color.rgb + fogColor.rgb * fogColor.a * fogFactor;
    //return float4(color.rgb, alpha);
    return DitheredRayMarch(pin.position.xy, rayStart, rayDir, rayLength, pin);
}