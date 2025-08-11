cbuffer CONSTANT_BUFFER : register(b8)
{
    float iTime;
    float2 iResolution;
};

#define PI 3.14159

#define THICCNESS 0.03
#define RADIUS 0.2
#define SPEED 4.0

#define aa 2.0 / min(iResolution.x, iResolution.y)

float2 remap(float2 coord)
{
    return coord / min(iResolution.x, iResolution.y);
}

float circle(float2 uv, float2 pos, float rad)
{
    return 1.0 - smoothstep(rad, rad + 0.005, length(uv - pos));
}

float ring(float2 uv, float2 pos, float innerRad, float outerRad)
{
    return (1.0 - smoothstep(outerRad, outerRad + aa, length(uv - pos))) * smoothstep(innerRad - aa, innerRad, length(uv - pos));
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = remap(fragCoord.xy);
    uv -= float2(0.5 / iResolution.y * iResolution.x, 0.5);

    float geo = 0.0;

    geo += ring(uv, 0.0, RADIUS - THICCNESS, RADIUS);

    float rot = -iTime * SPEED;

    uv = mul(uv, float2x2(cos(rot), sin(rot), -sin(rot), cos(rot)));

    float a = atan2(uv.x, uv.y) * PI * 0.05 + 0.5;

    a = max(a, circle(uv, float2(0.0, -RADIUS + THICCNESS / 2.0), THICCNESS / 2.0));

    fragColor = a * geo;
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    float4 fragColor = 0;
    float2 fragCoord = position.xy;

    mainImage(fragColor, fragCoord);

    return fragColor;
}