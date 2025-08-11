#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); //�p�[�e�B�N���Ǘ��o�b�t�@
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); //�p�[�e�B�N���ԍ��Ǘ��o�b�t�@�i�����ւ̒ǉ���p�j

RWByteAddressBuffer indirectDataBuffer : register(u2); // �C���_�C���N�g�p�o�b�t�@

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); //�p�[�e�B�N���w�b�_�[�Ǘ��o�b�t�@

[numthreads(NumParticleThread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint headerIndex = dispatchThreadId.x;
    
    ParticleHeader header = particleHeaderBuffer[headerIndex];
    
    uint dataIndex = header.particleIndex;
    
    //�L���t���O�������Ă�����̂�������
    if (header.alive == 0)
        return;
    
    ParticleData data = particleDataBuffer[dataIndex];
    
    //�o�ߎ��ԕ�����������
    data.parameter.y -= deltaTime;
    if (data.parameter.y < 0)
    {
        //�������s�����疢�g�p���X�g�ɒǉ�
        header.alive = 0;
        particleUnusedBuffer.Append(dataIndex);
        
        //�@�w�b�_�[���X�V
        particleHeaderBuffer[headerIndex] = header;
        particleDataBuffer[dataIndex] = data;
        
        //���S�����J�E���g����
        indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumDeadParticle, 1);
        return;
    }
    
    
    //TODO:���ƂŔC�ӂ̏����ɕς���
    //if (data.parameter.x == 0)
    switch ((int) (data.parameter.x + 0.5f))
    {
        case 1://�^�[�Q�b�g�ʒu�ɏW�܂�G�t�F�N�g
        {
            //���x�X�V
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
            
            //�^�[�Q�b�g�ʒu�Ɍ����x�N�g�����v�Z
            float3 vec = normalize(data.customData.xyz - data.position.xyz);
            
            //�ʒu�X�V
            data.position.xyz += vec * data.velocity.xyz * deltaTime;
        
            //�؂�����W���Z�o
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
            //���x�X�V
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
        
            //�ʒu�X�V
            data.position.xyz += data.velocity.xyz * deltaTime * 5;
        
            //�؂�����W���Z�o
            uint type = (uint) (data.parameter.z - data.parameter.y + 0.5f);
            float w = 1.0 / textureSplitCount.x;
            float h = 1.0 / textureSplitCount.y;
            float2 uv = float2((type % textureSplitCount.x) * w, (type / textureSplitCount.x) * h);
            data.texcoord.xy = uv;
            data.texcoord.zw = float2(w, h);
        
            //���X�ɓ����ɂ��Ă���
            data.color.a = saturate(data.parameter.y);
            break;
        }
        default:
        {
            //���x�X�V
            data.velocity.xyz += data.acceleration.xyz * deltaTime;
        
            //�ʒu�X�V
            data.position.xyz += data.velocity.xyz * deltaTime;
        
            //�؂�����W���Z�o
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
        
            //���X�ɓ����ɂ��Ă���
            data.color.a = saturate(data.parameter.y);
            break;
        }
    }
        
    //�[�x�\�[�g�l�Z�o
    header.depth = mul(float4(data.position.xyz, 1), viewProjection).w;

    //�X�V�����i�[
    particleHeaderBuffer[headerIndex] = header;
    particleDataBuffer[dataIndex] = data;
}