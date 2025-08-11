#include "staticMesh.hlsli"

VS_OUT main(float4 positon : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
    VS_OUT vout;
    vout.position = mul(positon, mul(world, viewProjection));

    vout.worldPosition = mul(positon,world);
    normal.w = 0;
    vout.worldNormal = normalize(mul(normal, world));
    
    //float4 N = normalize(mul(normal, world));
    //float4 L = normalize(-lightDirection);
    
    //vout.color.rgb = materialColor.rgb * max(0, dot(L, N));
    //vout.color.a = materialColor.a;
    vout.color = materialColor;
    vout.texcoord = texcoord;
    
    return vout;
}