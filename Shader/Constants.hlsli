#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE
cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 viewProjection;
    //float4 lightDirection;
    float4 cameraPositon;
    //float4 colorLight; //w colorPower
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 inverseProjection;
    row_major float4x4 inverseViewProjection;
    //float iblIntensity;
    bool enableSSAO;
    float reflectionIntensity;
    float time;
    // shader ‚Ìƒtƒ‰ƒO
    bool enableCascadedShadowMaps;
    bool enableSSR;
    bool enableFog;
    bool enableBloom;
    float pad;
    row_major float4x4 invView;
}

cbuffer SHADER_CONSTANT_BUFFER : register(b2)
{
    //‹P“x
    float luminanceThreshold;
    float gaussianSigma;
    float bloomIntenssity;
    float exposure;
    // CASCADED_SHADOW_MAPS
    float shadowColor;
    float shadowDepthBias;
    bool colorizeCascadedLayer;
    // SCREEN_SPACE_REFLECTION
    float maxDistance;
    float resolution;
    int steps;
    float thickness;
    
}
// CASCADED_SHADOW_MAPS
cbuffer CSM_CONSTANTS : register(b3)
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
}
// FOG
cbuffer FOG_CONSTANTS_BUFFER : register(b4)
{
    float4 fogColor;
    
    float fogDensity;
    float fogHeightFalloff;
    float groundLevel;
    float fogCutoffDistance;
    
    float mieScatteringCoef;
    
    bool enableDither;
    bool enableBlur;
    
    float timeScale;
    float noiseScale;
}

cbuffer VOLUMETRIC_CLOUDSCAPES_CONSTANT_BUFFER : register(b5)
{
    float2 windDirection;
    float2 cloudAltitudesMinMax; // highest and lowest altitudes at which clouds are distributed
    
    float windSpeed; // [0.0, 20.0]
    
    float densityScale; // [0.01,0.2]
    float cloudCoverageScale; // [0.1,1.0]
    float rainCloudAbsorptionScale;
    float cloudTypeScale;
    
    float earthRadius; // earth radius
    float horizonDistanceScale;
    float lowFreqyentlyPerlinWorleySamplingScale;
    float highFreqyentlyWorleySamplingScale;
    float cloudDensityLongDistanceScale;
    bool enablePowderedSugarEffect;
    
    uint rayMarchingSteps;
    bool autoRayMarchingSteps;
}

cbuffer VIEW_CONSTANTS_BUFFER : register(b8)
{
    row_major float4x4 vviewProjection;
    float4 vcameraPositon;
    row_major float4x4 vview;
    row_major float4x4 vprojection;
    row_major float4x4 vinverseProjection;
    row_major float4x4 vinverseViewProjection;
    row_major float4x4 vinvView;
}

#endif