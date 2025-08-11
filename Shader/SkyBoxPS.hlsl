#include "SkyMap.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
TextureCube skybox : register(t0); // latitude-longitude mapped texture

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 R = mul(float4((pin.texcoord.x * 2.0) - 1.0, 1.0 - (pin.texcoord.y * 2.0), 1.0, 1.0), inverse_view_projection);
    R /= R.w;

    const float lod = 0;
    return skybox.SampleLevel(sampler_states[LINEAR], R.xyz, lod);
}

