#include "GltfModel.hlsli"
//#include "LineSegment.hlsli"
cbuffer CONSTANTS : register(b12)
{
    float4 color;
}


float4 main(VS_OUT pin) : SV_TARGET
{
    return color; //�f�o�b�N�p�ŐF�w��
    const MaterialConstants m = materials[material];
    
    float4 basecolorFactor = m.pbrMetallicRoughness.basecolorFactor;
    return basecolorFactor;
}