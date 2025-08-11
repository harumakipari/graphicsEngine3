#include "GltfModel.hlsli"

VS_OUT main(INSTANCE_VS_IN vsIn)
{
    VS_OUT vsOut;
    
    //float4x4 worldTransform = mul(world, vsIn.instance_matrix);
    row_major float4x4 worldTransform = vsIn.instance_matrix;
    
    vsOut.position = mul(float4(vsIn.position.xyz, 1), mul(worldTransform, viewProjection));
    vsOut.wPosition = mul(float4(vsIn.position.xyz, 1), worldTransform);
    
    vsOut.wNormal = normalize(mul(float4(vsIn.normal.xyz, 0), worldTransform));
    vsOut.wNormal.w = 0;

    float sigma = vsIn.tangent.w;
    vsOut.wTangent = normalize(mul(float4(vsIn.tangent.xyz, 0), worldTransform));
    
    vsOut.wTangent.w = sigma;
    // vsOut ‚É bitangent ‚ª‚ ‚Á‚½‚ç
    // vsOut.bitangent = normalize(cross(vsOut.wNormal.xyz,vsOut.tangent.xyz) * sigma);
    
    vsOut.texcoord = vsIn.texcoord;
    //vsOut.texcoord[1] = vsIn.texcoord;
    
    return vsOut;
}