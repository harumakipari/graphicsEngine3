#include "LineSegment.hlsli"

cbuffer CONSTANTS : register(b12)
{
    float4 color;
}

float4 main() : SV_TARGET
{
    return color;
}