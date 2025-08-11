#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // �p�[�e�B�N���Ǘ��o�b�t�@
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); // �p�[�e�B�N���ԍ��Ǘ��o�b�t�@�i�����ւ̒ǉ���p�j

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); // �p�[�e�B�N���w�b�_�[�Ǘ��o�b�t�@

[numthreads(NumParticleThread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    int index = dispatchThreadId.x;
    
    //�p�[�e�B�N����񏉊���
    particleHeaderBuffer[index].alive = 0;
    particleHeaderBuffer[index].particleIndex = 0;
    
    //���g�p���X�g�iAppendStructuredBuffer�j�̖����ɒǉ�
    particleUnusedBuffer.Append(index);
}