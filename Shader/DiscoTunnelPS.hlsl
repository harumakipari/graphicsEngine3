cbuffer CONSTANT_BUFFER : register(b8)
{
    float iTime;
    float2 iResolution;
};

float2 position(float z)
{
    return float2(
		0.0 + sin(z * 0.1) * 1.0 + sin(cos(z * 0.031) * 4.0) * 1.0 + sin(sin(z * 0.0091) * 3.0) * 3.0,
		0.0 + cos(z * 0.1) * 1.0 + cos(cos(z * 0.031) * 4.0) * 1.0 + cos(sin(z * 0.0091) * 3.0) * 3.0
	) * 1.0;
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 p = (fragCoord.xy * 2.0 - iResolution.xy) / min(iResolution.x, iResolution.y);
    float camZ = 25.0 * iTime;
    float2 cam = position(camZ);

    float dt = 0.5;
    float camZ2 = 25.0 * (iTime + dt);
    float2 cam2 = position(camZ2);
    float2 dcamdt = (cam2 - cam) / dt;

    float3 f = (0.0);
    for (int j = 1; j < 300; j++)
    {
        float i = float(j);
        float realZ = floor(camZ) + i;
        float screenZ = realZ - camZ;
        float r = 1.0 / screenZ;
        float2 c = (position(realZ) - cam) * 10.0 / screenZ - dcamdt * 0.4;
        float3 color = (float3(sin(realZ * 0.07), sin(realZ * 0.1), sin(realZ * 0.08)) + (1.0)) / 2.0;
        f += color * 0.06 / screenZ / (abs(length(p - c) - r) + 0.01);
    }

    fragColor = float4(f, 1.0);
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    float4 fragColor = 0;
    float2 fragCoord = position.xy;

    mainImage(fragColor, fragCoord);

    return fragColor;
}