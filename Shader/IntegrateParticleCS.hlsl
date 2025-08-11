#include "Particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

//  1. パーティクルシステムでIntegrateを常時呼び出す(Updateのたびに呼び出す)
//  2. スポーン関数を用意する（emitCounterMaxにそのフレームで生成したいパーティクル数をシェーダー側に送る）
static const uint emitCountMax = 10; //  1回の更新で生成する数(定数バッファで送る)
//  3. コンピュートシェーダー内で生成カウントアップする
RWStructuredBuffer<uint> emitCounter : register(u1);
//  4. emitCounter < emitCountMax であればパーティクルを生成する


#define POINT 0
#define LINER 1
#define ANISOTROPIC 2

SamplerState samplerState[3] : register(s0);
Texture2D colorTemperChart : register(t0);
Texture3D<float> noise3D : register(t1);

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    Particle p = particleBuffer[id];
    
    p.age += deltaTime;
    
    bool respawn = loop;
    if (p.age > p.lifespan) // Ligespan has expired.
    {
        uint currentCounter;
        //InterlockedAdd(emitCounter[0], 1, currentCounter);
        if (respawn)//  || currentCounter < emitCountMax)
        {
            SpawnTest(id, p);
            p.color.a = 0.0;
        }
    }
    //  死亡中
    if (p.age > p.lifespan)
    {
        p.color.a = 0.0;
    }
    
    if (p.age > 0)
    {
#if 0
        float3 windDir = float3(1, 0, 0);
        float windStrength = 1;
        p.velocity += normalize(windDir) * windStrength * deltaTime;
#endif
        float3 force = 0;
        //if (p.state == 1)
        //{ // particles are finished
        //    p.size.y = 0.0;
        //}
        //gravity
        force += p.mass * float3(0.0, gravity, 0.0);
        //炎の噴出向き
        //float3 dir = float3(0, 0, 1); //敵の方向
        //float fireStrength = 1; //炎の力
        //p.velocity += normalize(dir) * fireStrength * deltaTime;
        //float3 dir = direction; //敵の方向
#if 0
        float3 dir = float3(direction.x, RandSigned(1.0f), direction.z);
        float fireStrength = max(strength, 0.1); //炎の力
        float3 acceleration = normalize(dir) * fireStrength;
        p.velocity += acceleration * deltaTime;
        //p.velocity.y = (RandSigned(1.0f)) * 5.0;
#endif
#if 0
        //vector field 
        float3 electricChargePosition = float3(sin(currentTime), 1.0, cos(currentTime));
        
        float3 r = electricChargePosition - p.position;
        float w = 1.0;
        
        float l = max(1e-4, length(r));
        force += w * normalize(r) / (l * l * l);
#endif
        if (!isStatic)
        {
            p.velocity += force / p.mass * deltaTime;
            float f0 = Rand((float) id / maxParticleCount / 30000.0); //間隔を広げた
            float f1 = Rand(f0 * PI);
            //p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f1);
        
            //中心に集まるエフェクト
            if (type == 1)
            {
                float len = length(emissionPosition.xyz - p.position);
                if (len < 1.0)
                {
                    p.velocity = normalize(emissionPosition.xyz - p.position) * 3.0;
                }
            }
            
            
#if 1
            //float speed = length(p.velocity);
            //const float maxSpeed = 10.0;
            //p.velocity = min(maxSpeed, speed) * normalize(p.velocity);
#endif      
            //p.velocity = mul((float3x3) worldTrasnform, p.velocity);
            //p.position = emisionPosition;
            p.position += p.velocity * strength * deltaTime;
            
            // worldTransform (Local Coord => World Coord)
            //p.position = mul(float4(p.position, 1.0), worldTrasnform).xyz;
        }
        else
        {
            p.velocity = 0;
        }
            
#if 0
        
        // 色の変化（温度マップを使用）
        p.color = colorTemperChart.SampleLevel(samplerState[LINER], float2( p.age / p.lifespan,0.5), 0);
        p.color = pow(p.color, 2.3);
#else
        //色の変化（開始色と終了色で線形補間）
        p.color = lerp(emissionStartColor, emissionEndColor, (p.age / p.lifespan));
#endif 
        
        
#if 1
        float alpha = FadeIn(fadeDuration.x, p.age, 1) * FadeOut(fadeDuration.y, p.age, p.lifespan, 1);
        p.color.a = pow(alpha, 1);
#endif
        p.angle += p.angularSpeed * deltaTime;
    }
    // 更新後のパーティクルデータをバッファに書き戻す
    particleBuffer[id] = p;
}
