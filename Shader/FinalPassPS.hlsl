#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"


SamplerState pointSamplerState : register(s0);
SamplerState linearSamplerState : register(s1);
SamplerState anisotropicSamplerState : register(s2);
SamplerState linearBorderBlackSamplerState : register(s3);
SamplerState linearBorderWhiteSamplerState : register(s4);
SamplerState linearClampSamplerState : register(s5);

SamplerComparisonState comparisonSamplerstate : register(s7);

Texture2D colorTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D depthTexture : register(t3);
Texture2D bloomTexure : register(t4);
Texture2D fogTexure : register(t5);
Texture2DArray cascadedShadowMaps : register(t6);


float2 NdcToUv(float2 ndc)
{
    float2 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    return uv;
}

inline float FSchlick(float f0, float cos)
{
    return f0 + (1 - f0) * pow(1 - cos, 5);
}

// ToneMap
float3 JodieReinhardTonemap(float3 c)
{
    float l = dot(c, float3(0.2126, 0.7152, 0.0722));
    float3 tc = c / (c + 1.0);

    return lerp(c / (l + 1.0), tc, tc);
}


// SCREEN_SPACE_AMBIENT_OCCLUSION
static const float3 kernel_points[64] =
{
    { 0.00868211500, -0.0427273996, 0.0337389559 },
    { 0.0422878861, 0.0255667437, 0.0421174169 },
    { 0.00133369095, -0.000206216064, 0.000314872246 },
    { 0.00550761260, 0.0445004962, 0.0369733199 },
    { 0.00802796893, 0.000712828245, 0.0389925502 },
    { -0.00727070076, 0.00919260643, 0.00159413333 },
    { 0.0814671144, 0.0169006232, 0.108687885 },
    { 0.0345553271, -0.0601100996, 0.0623369254 },
    { 0.0575188287, -0.117980719, 0.0903726891 },
    { 0.0338569917, -0.0841888934, 0.0627225488 },
    { 0.0592471771, -0.169112802, 0.0244632196 },
    { -0.0238562804, 0.105675779, 0.209886223 },
    { 0.0631165877, 0.0301473364, 0.0615524314 },
    { -0.0134508293, -0.0277324617, 0.00691395300 },
    { 0.114152357, 0.102236181, 0.111014776 },
    { -0.0221489109, -0.00430791033, 0.0260636527 },
    { -0.0271239709, 0.0771735162, 0.0862243697 },
    { 0.298623502, -0.137056544, 0.0121193761 },
    { -0.217831656, -0.111817703, 0.126772851 },
    { 0.0746574551, -0.00350309932, 0.114315465 },
    { -0.0230329111, 0.106690325, 0.0136655448 },
    { -0.0474987105, 0.0584983639, 0.0424890034 },
    { 0.0188070145, 0.0568048507, 0.0293925218 },
    { -0.0112051172, 0.179476753, 0.122986622 },
    { 0.0295139719, 0.162125558, 0.0753630325 },
    { -0.00302972272, -0.0406616218, 0.000857106352 },
    { 0.00778160710, 0.0270030703, 0.0350351147 },
    { 0.0268135704, 0.183430821, 0.0638199970 },
    { 0.130813897, 0.00156025402, 0.224606648 },
    { -0.147132352, 0.152517274, 0.183435142 },
    { -0.0907125175, -0.00177383155, 0.160256311 },
    { -0.0266149752, -0.330671698, 0.0708868578 },
    { 0.0116967773, -0.117801137, 0.0648231357 },
    { -0.278127432, 0.243157208, 0.248168632 },
    { -0.0433821045, 0.0629939660, 0.0638747588 },
    { -0.341667235, 0.0813603401, 0.283274621 },
    { -0.294411331, 0.218125328, 0.119183138 },
    { 0.124180131, -0.377155364, 0.268380553 },
    { 0.0290975738, 0.110363945, 0.0442696661 },
    { -0.345448762, -0.294173360, 0.357681096 },
    { -0.198425651, 0.117884018, 0.280610353 },
    { -0.0471894965, 0.167634696, 0.0171351191 },
    { 0.283999801, 0.307914823, 0.0745619610 },
    { 0.278367370, 0.143483743, 0.0777348951 },
    { -0.00319555169, 0.0113931699, 0.0792912468 },
    { 0.408831716, -0.255240113, 0.389685363 },
    { 0.0870304331, 0.131976292, 0.0758789256 },
    { 0.508056879, -0.226139233, 0.0249240790 },
    { 0.443417460, -0.0719382539, 0.0811995938 },
    { -0.405097991, -0.0126497196, 0.468290806 },
    { -0.271491766, 0.266288817, 0.173985392 },
    { -0.454723060, 0.203438774, 0.416500956 },
    { -0.225664973, 0.251260072, 0.130069673 },
    { 0.0662298575, 0.152405098, 0.0794597119 },
    { 0.214381784, -0.0221292414, 0.277178794 },
    { -0.543329179, -0.327755034, 0.367753297 },
    { -0.107578896, -0.311785251, 0.116201803 },
    { -0.138324469, -0.586842418, 0.159054667 },
    { -0.535777450, -0.474822164, 0.298666269 },
    { 0.525617778, -0.189941183, 0.353068262 },
    { 0.418803364, 0.0256034732, 0.492332071 },
    { 0.237295657, 0.280489057, 0.375523627 },
    { 0.146710590, -0.156013578, 0.0835350007 },
    { -0.173016176, -0.680333912, 0.495088488 },
};

static const float3 noise[16] =
{
    { 0.772417068, -0.423055708, 0.00000000 },
    { -0.714291692, 0.509235382, 0.00000000 },
    { -0.780199528, -0.183640003, 0.00000000 },
    { 0.472295880, 0.162610054, 0.00000000 },
    { -0.442540765, -0.263137937, 0.00000000 },
    { -0.960392714, -0.472289205, 0.00000000 },
    { -0.347602427, 0.830676079, 0.00000000 },
    { -0.573735476, -0.472531378, 0.00000000 },
    { -0.629811406, -0.642059684, 0.00000000 },
    { -0.352298439, -0.320050240, 0.00000000 },
    { 0.599728346, 0.426071405, 0.00000000 },
    { -0.173549354, 0.653944254, 0.00000000 },
    { -0.287484705, -0.0727353096, 0.00000000 },
    { 0.324079156, 0.534158945, 0.00000000 },
    { 0.968484163, -0.229418755, 0.00000000 },
    { -0.377048135, 0.338529825, 0.00000000 },
};


float3 reinhard_tone_mapping(float3 color)
{
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float tone_mapped_luma = luma / (1. + luma);
    color *= tone_mapped_luma / luma;
    return color;
}


float4 CalculatedPositionNDC(VS_OUT pin)
{
    float depthNdc = depthTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).x;
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    return positionNdc;
}

float3 CalculatedCascadedShadowColor(VS_OUT pin)
{
    // CASCADED_SHADOW_MAPS
    float4 sampledColor = colorTexture.Sample(linearBorderBlackSamplerState, pin.texcoord);
    float3 sampleColor = sampledColor.rgb;
    float sampleAlpha = sampledColor.a;

    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;

    // ndc to world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;

    // Apply cascaded shadow mapping
    // Find alayer of cascaded view frustum volume
    float depthViewSpace = positionViewSpace.z;
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depthViewSpace)
        {
            cascadeIndex = layer;
            break;
        }
    }
    float shadowFactor = 1.0;
    if (cascadeIndex > -1)
    {
        // world space to loght view clip space, and to ndc
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w;
        // ndc to texture space
        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
        
        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisonSamplerstate, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
        
        float3 layerColor = 1;
#if 1
        if (colorizeCascadedLayer)
        {
            const float3 layerColors[4] =
            {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
                { 1, 1, 0 },
            };
            layerColor = layerColors[cascadeIndex];
        }
#endif
        //color *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
        //color *= float4(lerp(shadowColor, 1.0, shadowFactor) * layerColor, color.a);
        
        sampleColor *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
    }
    return sampleColor;
}


float3 CalculatedSSRColor(VS_OUT pin)
{
    // SCREEN_SPACE_REFLECTION
    int steps = 10;
    
    uint2 dimensions;
    uint mipLevel = 0, numberOfLevels;
    positionTexture.GetDimensions(mipLevel, dimensions.x, dimensions.y, numberOfLevels);
    
    float4 position = positionTexture.Sample(linearBorderWhiteSamplerState, pin.texcoord); // worldSpace
    float3 normal = normalTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).xyz; // worldSpace

    float4 positionFrom = position;
    float4 positionTo = positionFrom;
    
#if 0
    float3 incident = normalize(position);
#else
    float3 incident = normalize(position.xyz - cameraPositon.xyz);
#endif
    float3 reflection = normalize(reflect(incident, normal.xyz));

    float4 startWorld = float4(positionFrom.xyz + (reflection * 0), 1);
    float4 endWorld = float4(positionFrom.xyz + (reflection * maxDistance), 1);
    if (endWorld.z < 0)
    {
        float3 v = endWorld.xyz - startWorld.xyz;
        endWorld.xyz = startWorld.xyz + v * abs(startWorld.z / v.z);
    }

    //float4 startFrag = mul(startWorld, projection); // from view to clipSpace
    float4 startFrag = mul(startWorld, viewProjection); // from world to clipSpace
    startFrag /= startFrag.w; //from clipSpave to ndc
    startFrag.xy = NdcToUv(startFrag.xy); // from uv to fragment/pixel coordinate
    startFrag.xy *= dimensions;
    
    //float4 endFrag = mul(endWorld, projection); //from world to clipSpace
    float4 endFrag = mul(endWorld, viewProjection); //from world to clipSpace
    endFrag /= endFrag.w; //from clipSpace to ndc
    endFrag.xy = NdcToUv(endFrag.xy); //from ndc to uv
    endFrag.xy *= dimensions;
    
    float2 frag = startFrag.xy;
    
    float4 uv = 0;
    uv.xy = frag / dimensions;
    
    float deltaX = endFrag.x - startFrag.x;
    float deltaY = endFrag.y - startFrag.y;
    
    float useX = abs(deltaX) >= abs(deltaY) ? 1 : 0;
    float delta = lerp(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);
    
    float2 increment = float2(deltaX, deltaY) / max(delta, 0.001);
    
    float search0 = 0;
    float search1 = 0;
    
    int hit0 = 0;
    int hit1 = 0;
    
    float viewDistance = startWorld.z;
    float depth = thickness;
    
#define MAX_DELTA 64
    delta = min(MAX_DELTA, delta);
    [unroll(MAX_DELTA)]
    for (int i = 0; i < (int) delta; ++i)
    {
        frag += increment;
        uv.xy = frag / dimensions;
        if (uv.x <= 0 || uv.x >= 1 || uv.y <= 0 || uv.y >= 1)
        {
            hit0 = 0;
            break;
        }
#if 0
        positionTo = positionTexture.Sample(linearBorderWhiteSamplerState, uv.xy); //viewSpace
#else
        positionTo = positionTexture.Sample(linearBorderWhiteSamplerState, uv.xy); // worldSpace
        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), viewProjection);
        positionToClip /= positionToClip.w;
#endif   
        search1 = lerp((frag.y - startFrag.y) / deltaY, (frag.x - startFrag.x) / deltaX, useX);
        search1 = clamp(search1, 0.0, 1.0);
        
        // Perspective Correct Interpolation
        // NDC.z ベースで比較する
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
#if 0
        if (depth > 0 && depth < thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#else
        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#endif
    }
#if 0
    hit1 = hit0;
#else
    search1 = search0 + ((search1 - search0) / 2.0);
    steps *= hit0;
    
    [unroll]
    for (i = 0; i < steps; ++i)
    {
        frag = lerp(startFrag.xy, endFrag.xy, search1);
        uv.xy = frag / dimensions;
        
        positionTo = positionTexture.Sample(linearBorderWhiteSamplerState, uv.xy); //viewSpace
        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), viewProjection);
        positionToClip /= positionToClip.w;
        
        // PerspectiveCorrect Interpolation
#if 0
        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
        
        if (depth > 0 && depth < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#else
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#endif
    }
#endif
    float visiblity = hit1;
    visiblity *= (1 - max(dot(-normalize(positionFrom.xyz), reflection), 0));
    visiblity *= (1 - clamp(depth / thickness, 0, 1));
    visiblity *= positionTo.w;
    visiblity *= (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) ? 0 : 1;
    visiblity = clamp(visiblity, 0, 1);
    
    float fresnel = saturate(FSchlick(0.04, max(0, dot(reflection, normal.xyz))));
    float3 reflectionColor = colorTexture.Sample(linearBorderBlackSamplerState, uv.xy).rgb;
    reflectionColor = fresnel * reflectionColor * visiblity * reflectionIntensity;
    return reflectionColor;
}

float CalculatedSSAOColor(VS_OUT pin)
{
    float4 position = positionTexture.Sample(linearBorderWhiteSamplerState, pin.texcoord); //worldSpace
    float3 normal = normalTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).xyz; //worldSpace
    //float4 position = positionTexture.Sample(linearBorderWhiteSamplerState, pin.texcoord); //viewSpace
    //float3 normal = normalTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).xyz; //viewSpace

    // SCREEN_SPACE_AMBIENT_OCCLUSION
    float3 randomVec = noise[(pin.position.x % 4) + 4 * (pin.position.y % 4)];
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    const int kernelSize = 64;
    
    float occlusion = 0.0; //accumlated value
    
    for (int kernel = 0; kernel < kernelSize; ++kernel)
    {
        float3 samplePosition = mul(kernel_points[kernel], TBN);
        const float radius = 1.0;
        samplePosition = position.xyz + samplePosition * radius;
        
        //float4 intersection = mul(float4(samplePosition, 1.0), projection); // from view to clip-space
        float4 intersection = mul(float4(samplePosition, 1.0), viewProjection); // from world to clip-space
        intersection /= intersection.w; // from clip-space to ndc
        intersection.z = depthTexture.SampleLevel(linearBorderWhiteSamplerState, NdcToUv(intersection.xy), 0).x;
        //intersection = mul(intersection, inverseProjection); // from ndc to view-space
        intersection = mul(intersection, inverseViewProjection); // from ndc to world-space
        intersection /= intersection.w; // perspective divide
		
        float3 v = intersection.xyz - position.xyz;
        const float beta = 0.0; //bias distance
        const float epsilon = 0.01;
        occlusion += max(0, dot(normal, v) - position.z * beta) / (dot(v, v) + epsilon);
    }
     
    const float sigma = 0.3;
    occlusion = max(0.0, 1.0 - (2.0 * sigma * occlusion / kernelSize));
    
    const float power = 1.0;
    occlusion = pow(occlusion, power);
    return occlusion;
}

float3 CalculatedFogColor(VS_OUT pin)
{
    // FOG
    uint2 depthMapDimensions;
    uint depthMipLevel = 0, numberOfSamples, levels;
    depthTexture.GetDimensions(depthMipLevel, depthMapDimensions.x, depthMapDimensions.y, numberOfSamples);
    
    float fogFacter = 0;
    if (enableBlur)
    {
        float depth = depthTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).x;
    
        float accumulatedRadiance = 0.0;
        float accumulatedWeight = 0.0;
        const float radius = 4.0;
        for (float x = -radius; x <= radius; x += 1.0)
        {
            for (float y = -radius; y <= radius; y += 1.0)
            {
                float2 offset = float2(x, y) / depthMapDimensions;
                float2 texcoord = pin.texcoord + offset;
                
                float sampledRadiance = fogTexure.Sample(linearBorderBlackSamplerState, texcoord).x;

                float distance = x * x + y * y;
                const float sigma = 2.0 * radius * radius;
                float domainGaussian = exp(-distance / sigma);
                
                float sampledDepth = depthTexture.Sample(linearBorderBlackSamplerState, texcoord).x;
                distance = (depth - sampledDepth) * (depth - sampledDepth);
                const float sigma2 = 0.0001;
                float rangeGaussian = exp(-distance / sigma2);
                
                accumulatedRadiance += sampledRadiance * domainGaussian * rangeGaussian;
                accumulatedWeight += domainGaussian * rangeGaussian;
            }
        }
        fogFacter = accumulatedRadiance / max(accumulatedWeight, 0.00001f);
    }
    else
    {
        fogFacter = fogTexure.Sample(linearBorderBlackSamplerState, pin.texcoord).x;
    }

    float3 fogColor_ = fogColor.rgb * fogColor.a * max(0, fogFacter);
    return fogColor_;
}

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);

}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorTexture.Sample(linearBorderBlackSamplerState, pin.texcoord);
    float alpha = color.a;
    
    float depthNdc = depthTexture.Sample(linearBorderBlackSamplerState, pin.texcoord).x;
    
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
    // ndc to world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
    
    uint2 dimensions;
    uint mipLevel = 0, numberOfLevels;
    positionTexture.GetDimensions(mipLevel, dimensions.x, dimensions.y, numberOfLevels);
    
    // SCREEN_SPACE_REFLECTION
    if (enableSSR)
    {
        color.rgb += CalculatedSSRColor(pin);
    }
    
    // SCREEN_SPACE_AMBIENT_OCCLUSION
    float occlusion = CalculatedSSAOColor(pin);
    if (enableSSAO)
    {
        color.rgb *= occlusion;
    }

    // CASCADED_SHADOW_MAPS
    float3 cascadedShadowMapColor = CalculatedCascadedShadowColor(pin);
    if (enableCascadedShadowMaps)
    {
        color.rgb += cascadedShadowMapColor;
    }
    
    // FOG
    if (enableFog)
    {
        color.rgb += CalculatedFogColor(pin);
    }
    
    //確認
    //return float4(fogFacter.xxx, 1);
    
#if 1
    //BLOOM
    if (enableBloom)
    {
        float4 bloom = bloomTexure.Sample(pointSamplerState, pin.texcoord);
        color.rgb += bloom.rgb;
    }
    
#if 0
    // Tone mapping : HDR -> SDR
    const float exposure = 1.2;
    color.rgb = 1 - exp(-color.rgb * exposure);
#else
    color.rgb = JodieReinhardTonemap(color.rgb);
#endif
    
#if 0
    float2 glitchUV = pin.texcoord;
    float2 grid = floor(pin.texcoord * 10.0);
    float timeSeed = floor(time * 4.0); // 0.25秒ごとに変化
    
    
    float rand = frac(sin(dot(grid + timeSeed, float2(12.9898, 78.233))) * 43758.5453);
    
    float t = time * 1.0; // 高速変化させたいならスケーリング
    float2 block = floor(pin.texcoord * float2(30 * rand + 1, 10 * rand + 1));
    float2 noise = frac(sin(dot(block, float2(12.9898 + t, 78.233 + t))) * 43758.5433);
    glitchUV += (noise - 0.5) * 0.1;
#if 0
    //RGBずらし（色収差）
    float2 offset = float2(0.005, 0);
    
    float r = colorTexture.Sample(pointSamplerState, glitchUV + offset).r;
    float g = colorTexture.Sample(pointSamplerState, glitchUV).g;
    float b = colorTexture.Sample(pointSamplerState, glitchUV - offset).b;
    color.rgb = float3(r, g, b);
#else
    float3 col = colorTexture.Sample(pointSamplerState, glitchUV).rgb;
    color.rgb = col * lerp(1.0, 1.5, step(0.9, noise.x));
#endif
#else
#if 0
    float2 glitchUV = pin.texcoord;
    //乱数（時間によって少しずつ変わる）
    float noiseSeed = rand(floor(glitchUV * 40.0) + time * 5.0);
    //ノイズをかけるかどうか
    if (noiseSeed < 0.001f)
    {
        float noiseAmount = rand(glitchUV * 100.0 + time) * 0.05;
        
        glitchUV.x += noiseAmount;
        color = colorTexture.Sample(pointSamplerState, glitchUV);
    }
#endif    
#endif
    
	// Gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, 1.0 / GAMMA);
    return float4(color.rgb, alpha);
#endif
}
