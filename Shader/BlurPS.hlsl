struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

#define POINT 0
#define LINEAR 1
#define ANISOTROPHIC 2
SamplerState samplerStates[3] : register(s0);
Texture2D textureMaps[4] : register(t0);


//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPosition;
//}

//cbuffer SHADER_CONSTANT_BUFFER : register(b2)
//{
//    //‹P“x
//    float luminanceThreshold;
//    float gaussianSigma;
//    float bloomIntenssity;
//    float exposure;
//}
#include "Constants.hlsli"


float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, numberOfLevels;
    textureMaps[1].GetDimensions(mipLevel, width, height, numberOfLevels);
    
    float4 color = textureMaps[0].Sample(samplerStates[LINEAR], pin.texcoord);
    float alpha = color.a;
    
    float3 blurColor = 0;
    float gaussianKernerTotal = 0;
    
    const int gaussianHalfKernelSize = 3;
    
    [unroll]
    for (int x = -gaussianHalfKernelSize; x <= +gaussianHalfKernelSize; x += 1)
    {
        [unroll]
        for (int y = -gaussianHalfKernelSize; y <= +gaussianHalfKernelSize; y += 1)
        {
            float gaussianKernel = exp(-(x * x + y * y) / (2.0 * gaussianSigma * gaussianSigma)) / (2 * 3.14159265358979 * gaussianSigma * gaussianSigma);
            //float gaussianKernel = 1;
            blurColor += textureMaps[1].Sample(samplerStates[LINEAR], pin.texcoord + float2(x * 1.0 / width, y * 1.0 / height)).rgb * gaussianKernel;
            gaussianKernerTotal += gaussianKernel;
        }
    }
    blurColor /= gaussianKernerTotal;
    

    //const float bloomIntenssity = 1.0;
    //color.rgb = blurColor * bloomIntenssity;
    
    color.rgb += blurColor * bloomIntenssity;
    
#if 1
    //Tone mapping :HDR ->SDR
    //const float exposure = 1.2;
    color.rgb = 1 - exp(-color.rgb * exposure);
#endif
    
#if 1
    //Tone mapping :HDR ->SDR
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, 1.0 / GAMMA);
#endif
    
    
    return float4(color.rgb, alpha);
}

