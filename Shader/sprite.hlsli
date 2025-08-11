struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

cbuffer SPRITE_CONSTANTS : register(b10)
{
    float elapsedTime;
    uint enableGlitch;
};