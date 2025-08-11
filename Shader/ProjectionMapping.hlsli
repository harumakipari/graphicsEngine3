//Texture2D projectionMappingTexture : register(t15);

//float3 ProjectionMapping(float4 worldPosition, row_major float4x4 projectionMappingTransform, SamplerState samplerState)
//{
//    // PROJECTION_MAPPING
//    const float colorIntensity = 10;
//    float3 projectionMappingColor = 0;
//    float4 projectionTexturePosition = mul(worldPosition, projectionMappingTransform);
//    projectionTexturePosition /= projectionTexturePosition.w;
//    projectionTexturePosition.x = projectionTexturePosition.x * 0.5 + 0.5;
//    projectionTexturePosition.y = -projectionTexturePosition.y * 0.5 + 0.5;
//    if (saturate(projectionTexturePosition.z) == projectionTexturePosition.z)
//    {
//        float4 projectionTextureColor = projectionMappingTexture.Sample(samplerState, projectionTexturePosition.xy);
//        projectionMappingColor = projectionTextureColor.rgb * projectionTextureColor.a * colorIntensity;
//    }
//    return projectionMappingColor;
//}