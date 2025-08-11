#include "staticMesh.hlsli"

Texture2D colorMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState pointSamplerState : register(s0);
SamplerState linerSamplerState : register(s1);
SamplerState anisotropocSamplerState : register(s2);


float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorMap.Sample(anisotropocSamplerState, pin.texcoord);
    //float4 color = colorMap.Sample(pointSamplerState, pin.texcoord);
    float alpha = color.a;
    
    float3 N = normalize(pin.worldNormal.xyz);
    
#if 1
    float3 T = float3(1.0001, 0, 0);
    float3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    
    float4 normal = normalMap.Sample(linerSamplerState, pin.texcoord);
    
    //float4 normal = normalMap.Sample(pointSamplerState, pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    normal.w = 0;
    N = normalize(normal.x * T) + (normal.y * B) + (normal.z * N);
#endif
    
    float3 L = normalize(-lightDirection.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    return float4(diffuse, 1);
    
    float3 V = normalize(-cameraPosition.xyz);
    float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
    //float3 specular = pow(max(0, dot(N, normalize(V + L))), 2);
    
    return float4(diffuse + specular, alpha) * pin.color;
    //return colorMap.Sample(linerSamplerState, pin.texcoord) * pin.color;
}