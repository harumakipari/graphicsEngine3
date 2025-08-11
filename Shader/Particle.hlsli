cbuffer PARTICLE_SYSTEM_CONSTANTS : register(b9)
{
    float4 emissionPosition; // 粒子の生成位置
    float2 emissionOffset; //生成位置の半径範囲 (x: 最小, y: 最大)
    float2 emissionSize; // 粒子の生成サイズと消滅サイズ
    float2 emisionConeAngle; // 生成角度範囲 (ラジアン, x: 最小, y: 最大)
    float2 emissionSpeed; // 粒子の生成速度範囲
    float2 emissionAngularSpeed; // 角速度の範囲
    
    float2 lifespan; // 粒子の寿命範囲
    float2 spawnDelay; // 粒子生成の遅延時間範囲
    float2 fadeDuration; // フェードの時間範囲 (x: フェードイン, y: フェードアウト)
    
    float4 emissionStartColor;//色変化前の色
    float4 emissionEndColor;//色変化後の色
    
    float currentTime; // 現在の時間
    float deltaTime; // フレーム間の経過時間
    float _noiseScale; // ノイズスケール 
    float gravity; // 重力
    
    uint2 spriteSheetGrid; // スプライトシートのグリッドサイズ 
    uint maxParticleCount; // 粒子の最大数 (再確認用)
    
    float3 direction; //方向
    float strength; //方向の強さ
    
    row_major float4x4 nodeWorldTransform;
    row_major float4x4 worldTransform;
    float2 radius;
    
    bool loop;// ループするかどうか
    int type = 0;//パーティクルの種類
    int isStatic;//動かないかどうか
}

//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPositon;
//    float4 colorLight; //w colorPower
//    //row_major float4x4 lightViewProjection; // SHADOW
//    // PARTICLES
//    row_major float4x4 view;
//    row_major float4x4 projection;
//    row_major float4x4 inverseProjection;
//    row_major float4x4 inverseViewProjection;

//    float iblIntensity;
//}
#include "Constants.hlsli"

struct VS_OUT
{
    uint vertexId : VERTEXID;
};

struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct Particle
{
    int state; // 粒子の状態
    
    float4 color; // 粒子の色
    
    float3 position; // 粒子の位置  World space position
    float mass; // 粒子の質量
    
    float angle; // 粒子の回転角度 (ラジアン)
    float angularSpeed; // 粒子の角速度
    float3 velocity; // 粒子の速度 (ベクトル形式) World space Velocity
    
    float lifespan; // 粒子の寿命 (秒単位)
    float age; // 現在の粒子の経過時間 The current age counting down from lifespan to zero
    
    float2 size; // 粒子のサイズ (生成時と消滅時のサイズ)
    
    uint chip; // テクスチャの選択用インデックス
    
    
};

#define PI 3.14159265358979
float Rand(float n)
{
    // Return value is greater than or equal to 0 and less than 1.
    return frac(sin(n) * 43758.5453123);
}

// Returns a random value in the specified range [min, max] 
float RandRange(float seed, float min, float max)
{
    return lerp(min, max, Rand(seed));
}

#define NUMTHREADS_X 16

void Spawn(in uint id, inout Particle p)
{
    

#if 1
    //float theta = Rand((float) id / maxParticleCount * 2.0f * PI);
    //float r = Rand(theta * PI);
    //float theta = Rand(2.0f * PI * 0.01f * id); 
    //float phi = Rand(2.0f * PI * 0.01f * id);
    //float r = Rand(1.0 + id) * 1.0;
    //float theta = RandRange(2.0f * PI * 0.01f * id, -0.15 * PI, 0.15 * PI);
    float theta = RandRange(2.0f * PI * 0.01f * id, 0, 2.0 * PI);
    float phi = RandRange(2.0f * PI * 0.01f * id, -PI * 0.5, PI * 0.5);
    //float r = RandRange(1.0 + id, 0, 1);
    float r = RandRange(1.0 + id, radius.x, radius.y);
    
#if 1
    p.position = emissionPosition.xyz/*world space*/;
    //p.position = mul(worldTransform, float4(p.position, 1)).xyz;
    
#if 1
    p.velocity.z = -1;
    float2 offset = float2(r * (cos(theta)), r * sin(theta));
    float2 velocityXY = offset;
    p.velocity.xy = velocityXY;
#else
    p.velocity.x =  1;
    float2 offset = float2(r * (sin(theta)), r * cos(theta));
    float2 velocityYZ = offset;
    p.velocity.yz = velocityYZ;
    //p.velocity.y = 1;
    //float2 offset = float2(r * (cos(theta)), r * sin(theta));
    //float2 velocityXZ = offset;
    //p.velocity.xz = velocityXZ;
#endif
    //p.velocity = normalize(p.velocity);
    
    //p.position = mul(worldTransform, float4(emisionPosition.xyz, 1.0)).xyz;
    //p.velocity = mul((float3x3) worldTransform, p.velocity);
    //p.velocity = mul(mul(nodeWorldTransform, worldTransform), float4(p.velocity, 0)).xyz;
    
    
    //p.velocity = mul(mul(worldTransform, nodeWorldTransform), float4(p.velocity, 0)).xyz;
    //p.velocity = mul(float4(p.velocity, 0), mul(worldTransform, nodeWorldTransform)).xyz;
    p.velocity = mul(float4(p.velocity, 0), worldTransform).xyz;
    
    
    //p.velocity = mul(nodeWorldTransform, float4(p.velocity, 0)).xyz;
    p.velocity = normalize(p.velocity);
    //p.velocity.x =/* direction **/ normalize(position.x - emisionPosition.x);
    //p.velocity.y =/* direction **/ normalize(position.y - emisionPosition.y);
#else
     p.velocity.x = (direction.x);
    float2 offset = float2(r * (cos(theta)), r * sin(theta));
    p.position = emisionPosition.xyz;
    float2 velocityXY = normalize(offset);
    p.velocity.z = direction.z * velocityXY.x - direction.y * velocityXY.y;
    p.velocity.y = direction.z * velocityXY.y + direction.y * velocityXY.x;

    
    //float3 randomDir = normalize(float3(cos(theta) * r, sin(theta) * r, direction.z));
    //p.velocity = normalize(direction + randomDir) * strength;
#endif
    float f0 = Rand((float) id / maxParticleCount / 10000.0); //間隔を広げた
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI + 0.01f * id);
#else
#if 1
    //float f0 = Rand((float) id / maxParticleCount * 2.0f * PI);
    float f0 = Rand((float) id / maxParticleCount /30000.0);//間隔を広げた
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI);
#else
    float f0 = 0.5f;
    float f1 = 0.6f;
    float f2 = Rand(f1 * PI);
    float f3 = 0.8f;
    //p.age = 0;
#endif
    
    float3 offset = 0;
#if 1
    float radius = lerp(emissionOffset.x, emissionOffset.y, f0);
    offset.x = radius * cos(f1 * 2.0 * PI);
    offset.z = radius * sin(f1 * 2.0 * PI);
#endif
    p.position = emisionPosition.xyz + offset;
    
    float phi = 2.0 * PI * f0;
    float theta = lerp(emisionConeAngle.x, emisionConeAngle.y, f1);
    
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    
    p.velocity.x = sinTheta * cosPhi;
    //p.velocity.y = cosTheta;
    p.velocity.y = 0.0;
    //p.velocity.y = (Rand(1.0f) - 0.5f) * 5.0;
    p.velocity.z = sinTheta * sinPhi;
    p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
#endif
    //p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
    p.color = 1.0;
    
    p.mass = 1.0;
    
#if 1
    p.angle = PI * f0;
    p.angularSpeed = lerp(emissionAngularSpeed.x, emissionAngularSpeed.y, f3);
#else
    p.angle =0.0;
    p.angularSpeed = 0.0;
#endif
    
    //p.lifespan = RandRange(f0, lifespan.x, lifespan.y);
    p.lifespan = lerp(lifespan.x, lifespan.y, f0);
    p.age = -lerp(spawnDelay.x, spawnDelay.y, f0);
    p.state = 0;
    p.size.x = emissionSize.x * f1;
    p.size.y = emissionSize.y * f2;
    
    int count = spriteSheetGrid.x * spriteSheetGrid.y;
    p.chip = f3 * count;
}

void SpawnTest(in uint id, inout Particle p)
{
    float f0 = Rand((float) id / maxParticleCount * 2.0 * PI);
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI);
    
    float3 offset = 0;
    float radius = lerp(emissionOffset.x, emissionOffset.y, f0);
#if 0
    offset.x = radius * cos(f1 * 2.0 * PI);
    offset.z = radius * sin(f1 * 2.0 * PI);
#else
    offset.x = radius * cos(f3 * 2.0 * PI);
    offset.z = radius * sin(f3 * 2.0 * PI);
#endif
    p.position = emissionPosition.xyz + offset;
#if 0
    float phi = 2.0 * PI * f0;
    float theta = lerp(emisionConeAngle.x, emisionConeAngle.y, f1);
#else
    float phi = 2.0 * PI * f0;
    float theta = clamp(f2, emisionConeAngle.x, emisionConeAngle.y);
#endif
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    
    //p.velocity.x = sinTheta * cosPhi;
    //p.velocity.y = cosTheta;
    //p.velocity.z = sinTheta * sinPhi;
    
    
    // ローカル空間の方向（Y軸中心）
    float3 localDir;
    localDir.x = sinTheta * cosPhi;
    localDir.y = cosTheta;
    localDir.z = sinTheta * sinPhi;
    
    float3 worldDir;

    switch (type)
    {
        case 0:
        {
            if (dot(direction, direction) < 1e-6)
            {
                worldDir = 0;
            }
            else
            {
                // Y軸 → direction への回転マトリクスを構成
                float3 dir = normalize(direction); // 安全のため正規化
                float3 up = abs(dir.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
                float3 right = normalize(cross(up, dir));
                float3 forward = normalize(cross(dir, right));
                float3x3 rotMatrix = float3x3(right, dir, forward); // 右手系：列が right, up, forward

                // 回転適用して世界空間方向に変換
                worldDir = mul(localDir, rotMatrix);
            }
            break;
        }
        case 1://中心に集まるエフェクト
        {
            p.position += (localDir * strength);
            worldDir = emissionPosition.xyz - p.position;
            break;
        }
    };
    
    p.velocity = worldDir * lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
    p.color = 1.0;
    
    p.mass = 1.0;
    
#if 1
    p.angle = PI * f0;
    p.angularSpeed = lerp(emissionAngularSpeed.x, emissionAngularSpeed.y, f3);
#else
    p.angle = 0.0;
    p.angularSpeed = 0.0;
#endif
    
    p.lifespan = lerp(lifespan.x, lifespan.y, f0);
    p.age = -lerp(spawnDelay.x, spawnDelay.y, f0);
    p.state = 0;
    p.size.x = emissionSize.x * f1;
    p.size.y = emissionSize.y * f2;
    
    int count = spriteSheetGrid.x * spriteSheetGrid.y;
    p.chip = f3 * count;
}


float FadeIn(float duration, float age, float exponent)
{
    //return clamp(age / duration, 0.0, 1.0);
    return pow(smoothstep(0.0, 1.0, age / duration), exponent);
}

float FadeOut(float duration, float age, float lifespan, float exponent)
{
	//return clamp((lifespan - age) / duration, 0.0, 1.0);  
    return pow(smoothstep(0.0, 1.0, (lifespan - age) / duration), exponent);
}




