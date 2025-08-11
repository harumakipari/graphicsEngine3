//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPositon;
//    float4 colorLight;
//    row_major float4x4 view;
//    row_major float4x4 projection;
//    row_major float4x4 inverseProjection;
//    row_major float4x4 inverseViewProjection;
//    float iblIntensity_;
//    bool enableSsao_;
//    float refrectionIntensity_;
//    float time_;
//    int enableCascadedShadowMaps_;
//    int enableSSR_;
//    int enableFog_;
//    int enableBloom_;
//    row_major float4x4 invView_;
//}
#include "Constants.hlsli"

#define MAX_PROJECTION_MAPPING 32
cbuffer PROJECTION_MAPPING_CONSTANT_BUFFER : register(b5)
{
    row_major float4x4 projectionMappingTransforms[MAX_PROJECTION_MAPPING];
    uint4 enableMapping[MAX_PROJECTION_MAPPING / 4];
    uint4 textureId[MAX_PROJECTION_MAPPING / 4];
}