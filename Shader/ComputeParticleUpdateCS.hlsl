#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); //パーティクル管理バッファ
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); //パーティクル番号管理バッファ（末尾への追加専用）

RWByteAddressBuffer indirectDataBuffer : register(u2); // インダイレクト用バッファ

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); //パーティクルヘッダー管理バッファ

[numthreads(NumParticleThread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint headerIndex = dispatchThreadId.x;
    
    ParticleHeader header = particleHeaderBuffer[headerIndex];
    
    uint dataIndex = header.particleIndex;
    
    //有効フラグが立っているものだけ処理
    if (header.alive == 0)
        return;
    
    ParticleData data = particleDataBuffer[dataIndex];
    
    //経過時間分減少させる
    data.parameter.y -= deltaTime;
    if (data.parameter.y < 0)
    {
        //寿命が尽きたら未使用リストに追加
        header.alive = 0;
        particleUnusedBuffer.Append(dataIndex);
        
        //　ヘッダー情報更新
        particleHeaderBuffer[headerIndex] = header;
        particleDataBuffer[dataIndex] = data;
        
        //死亡数をカウントする
        indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumDeadParticle, 1);
        return;
    }
    
    
    //TODO:あとで任意の処理に変える
    //if (data.parameter.x == 0)
    switch ((int) (data.parameter.x + 0.5f))
    {
        case 1://ターゲット位置に集まるエフェクト
        {
            //速度更新
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
            
            //ターゲット位置に向くベクトルを計算
            float3 vec = normalize(data.customData.xyz - data.position.xyz);
            
            //位置更新
            data.position.xyz += vec * data.velocity.xyz * deltaTime;
        
            //切り取り座標を算出
            //uint type = (uint) (data.parameter.x + 0.5f);
            uint type = (uint) (data.parameter.z - data.parameter.y + 0.5f);
            float w = 1.0 / textureSplitCount.x;
            float h = 1.0 / textureSplitCount.y;
            float2 uv = float2((type % textureSplitCount.x) * w, (type / textureSplitCount.x) * h);
            data.texcoord.xy = uv;
            data.texcoord.zw = float2(w, h);        
            break;
        }
        case 12:
        {
            //速度更新
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
        
            //位置更新
            data.position.xyz += data.velocity.xyz * deltaTime * 5;
        
            //切り取り座標を算出
            uint type = (uint) (data.parameter.z - data.parameter.y + 0.5f);
            float w = 1.0 / textureSplitCount.x;
            float h = 1.0 / textureSplitCount.y;
            float2 uv = float2((type % textureSplitCount.x) * w, (type / textureSplitCount.x) * h);
            data.texcoord.xy = uv;
            data.texcoord.zw = float2(w, h);
        
            //徐々に透明にしていく
            data.color.a = saturate(data.parameter.y);
            break;
        }
        default:
        {
            //速度更新
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
        
            //位置更新
            data.position.xyz += data.velocity.xyz * deltaTime;
        
            //切り取り座標を算出
            //uint type = (uint)((data.parameter.z - data.parameter.y / data.parameter.z) * (textureSplitCount.x * textureSplitCount.y));
            //uint type = lerp(0, (textureSplitCount.x * textureSplitCount.y), ((data.parameter.z - data.parameter.y) / data.parameter.z));
            float lifeRatio = (data.parameter.z - data.parameter.y) / data.parameter.z;
            uint frameCount = textureSplitCount.x * textureSplitCount.y;
            uint type = min((uint) (lifeRatio * frameCount), frameCount - 1); // clamping
            
            float w = 1.0 / textureSplitCount.x;
            float h = 1.0 / textureSplitCount.y;
            float2 uv = float2((type % textureSplitCount.x) * w, (type / textureSplitCount.x) * h);
            data.texcoord.xy = uv;
            data.texcoord.zw = float2(w, h);
        
            //徐々に透明にしていく
            data.color.a = saturate(data.parameter.y);
            break;
        }
    }
        
    //深度ソート値算出
    header.depth = mul(float4(data.position.xyz, 1), viewProjection).w;

    //更新情報を格納
    particleHeaderBuffer[headerIndex] = header;
    particleDataBuffer[dataIndex] = data;
}