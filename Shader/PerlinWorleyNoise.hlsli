/*
This tab contains all the necessary noise functions required to model a cloud shape.
*/

// Hash by David_Hoskins
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uint2(UI0, UI1)
#define UI3 uint3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

float3 Hash33(float3 p)
{
    uint3 q = uint3(int3(p)) * UI3;
    q = (q.x ^ q.y ^ q.z) * UI3;
    return -1.0 + 2.0 * float3(q) * UIF;
}

float Remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Gradient noise by iq (modified to be tileable)
float GradientNoise(float3 x, float freq)
{
    // grid
    float3 p = floor(x);
    float w = frac(x);
    
    // quintic interpolant
    float3 u = w * w * w * (w * (w * 6.0 - 15.0) + 10.0);
    
    // gradients
    float3 ga = Hash33(fmod(p + float3(0.0, 0.0, 0.0), freq));
    float3 gb = Hash33(fmod(p + float3(1.0, 0.0, 0.0), freq));
    float3 gc = Hash33(fmod(p + float3(0.0, 1.0, 0.0), freq));
    float3 gd = Hash33(fmod(p + float3(1.0, 1.0, 0.0), freq));
    float3 ge = Hash33(fmod(p + float3(0.0, 0.0, 1.0), freq));
    float3 gf = Hash33(fmod(p + float3(1.0, 0.0, 1.0), freq));
    float3 gg = Hash33(fmod(p + float3(0.0, 1.0, 1.0), freq));
    float3 gh = Hash33(fmod(p + float3(1.0, 1.0, 1.0), freq));
    
    // projections
    float va = dot(ga, w - float3(0.0, 0.0, 0.0));
    float vb = dot(gb, w - float3(1.0, 0.0, 0.0));
    float vc = dot(gc, w - float3(0.0, 1.0, 0.0));
    float vd = dot(gd, w - float3(1.0, 1.0, 0.0));
    float ve = dot(ge, w - float3(0.0, 0.0, 1.0));
    float vf = dot(gf, w - float3(1.0, 0.0, 1.0));
    float vg = dot(gg, w - float3(0.0, 1.0, 1.0));
    float vh = dot(gh, w - float3(1.0, 1.0, 1.0));
    
    // interpolation
    return va +
           u.x * (vb - va) +
           u.y * (vc - va) +
           u.z * (ve - va) +
           u.x * u.y * (va - vb - vc + vd) +
           u.y * u.z * (va - vc - ve + vg) +
           u.z * u.x * (va - vb - ve + vf) +
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

// Tileable 3D worley noise
float WorleyNoise(float3 uv, float freq)
{
    float3 id = floor(uv);
    float3 p = frac(uv);
    
    float minDist = 10000.0;
    for (float x = -1; x <= 1; ++x)
    {
        for (float y = -1; y <= 1; ++y)
        {
            for (float z = -1; z <= 1; ++z)
            {
                float3 offset = float3(x, y, z);
                float3 h = Hash33(fmod(id + offset, freq)) * 0.5 + 0.5;
                h += offset;
                float3 d = p - h;
                minDist = min(minDist, dot(d, d));
            }
        }
    }
    
    // inverted worley noise
    return 1.0 - minDist;
}

// Fbm for Perlin noise based on iq's blog
float PerlinFbm(float3 p, float freq, int octaves)
{
    float g = exp2(-0.85);
    float amp = 1.0;
    float noise = 0.0;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * GradientNoise(p * freq, freq);
        freq *= 2.0;
        amp *= g;
    }

    return noise;
}

// Tileable Worley fbm inspired by Andrew Schneider's Real_Time Volumetric Cloudscapes chapter in GPU Pro 7.
float WorleyFbm(float3 p, float freq)
{
    return WorleyNoise(p * freq, freq) * 0.625 +
        WorleyNoise(p * freq * 2.0, freq * 2.0) * 0.25 +
        WorleyNoise(p * freq * 4.0, freq * 4.0) * 0.125;
}