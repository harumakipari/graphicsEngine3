struct VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    uint4 joints[2] : JOINTS;
    float4 weights[2] : WEIGHTS;
};

struct BATCH_VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

struct INSTANCE_VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    row_major float4x4 instance_matrix : INSTANCE_MATRIX;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 wPosition : POSITION;
    float4 wNormal : NORMAL;
    float4 wTangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    
    float4 cpuColor;
    
    int material;
    bool hasTangent;
    int skin;
    float dissolveValue;//ディゾルブ用
    
    float emission;
}

//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPositon;
//    float4 colorLight; //w colorPower
//    row_major float4x4 view;
//    row_major float4x4 projection;
//    row_major float4x4 inverseProjection;
//    row_major float4x4 inverseViewProjection;
//    float iblIntensity;
//    bool enableSsao;
//    float refrectionIntensity;
//}
#include "Constants.hlsli"

struct TextureInfo
{
    int index;
    int texcoord;
};

struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};

struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};

struct PbrMetallicRoughness
{
    float4 basecolorFactor;
    TextureInfo basecolorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};

struct MaterialConstants
{
    float3 emissiveFactor; // length 3. default [0, 0, 0]
    int alphaMode; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2 
    float alphaCutoff; // default 0.5
    bool doubleSided; // default false;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};

StructuredBuffer<MaterialConstants> materials : register(t0);

//PRIMITIVE_JOINT_CONSTANTS定数バッファを定義
static const uint PRIMITIVE_MAX_JOINTS = 512;
cbuffer PRIMITIVE_JOINT_CONSTANTS : register(b2)
{
    row_major float4x4 jointMatrices[PRIMITIVE_MAX_JOINTS];
}


// MULTIPLE_RENDER_TARGETS
struct PS_OUT
{
    float4 color : SV_TARGET0;
    float4 position : SV_TARGET1;
    float4 normal : SV_TARGET2;
};

struct GBUFFER_PS_OUT
{
    float4 normal : SV_TARGET0;
    float4 msr : SV_TARGET1; // metallic occulusion roughness occlusionStrength
    float4 color : SV_TARGET2;
    float4 position : SV_TARGET3;   // 
    float4 emmisive : SV_TARGET4;
};