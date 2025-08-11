#include "ComputeParticle.hlsli"

Texture2D colorMap : register(t0);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    return colorMap.Sample(samplerStates[ANISOTROPIC], pin.texcoord) * pin.color;
}