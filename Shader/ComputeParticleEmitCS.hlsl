#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // �p�[�e�B�N���Ǘ��o�b�t�@
ConsumeStructuredBuffer<uint> particlePoolBuffer : register(u1); // �p�[�e�B�N���ԍ��Ǘ��o�b�t�@�i����������o����p�j

StructuredBuffer<EmitParticleData> emitParticleBuffer : register(t0); //�p�[�e�B�N���������o�b�t�@
RWByteAddressBuffer indirectDataBuffer : register(u2); //�C���_�C���N�g�p�o�b�t�@

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); //�p�[�e�B�N���w�b�_�[�Ǘ��o�b�t�@

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    //���g�p���X�g�̖������疢�g�p�p�[�e�B�N����index���擾
    uint particleIndex = particlePoolBuffer.Consume();
    uint emitIndex = dispatchThreadId.x;

    //�w�b�_�[�̖�������擾
    uint headerIndex = 0;
    indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumEmitParticleIndex, 1, headerIndex);
    
    //�p�[�e�B�N����������
    particleHeaderBuffer[headerIndex].alive = 1; //�����t���O
    particleHeaderBuffer[headerIndex].particleIndex = particleIndex; //�p�[�e�B�N���f�[�^�o�b�t�@�̍��W
    particleHeaderBuffer[headerIndex].depth = 1; //�[�x
    particleHeaderBuffer[headerIndex].dummy = 0; //��
    
    //�p�[�e�B�N����������
    particleDataBuffer[particleIndex].parameter.x = emitParticleBuffer[emitIndex].parameter.x;
    particleDataBuffer[particleIndex].parameter.y = emitParticleBuffer[emitIndex].parameter.y;//�������ԃJ�E���g�p
    particleDataBuffer[particleIndex].parameter.z = emitParticleBuffer[emitIndex].parameter.y;//�������ԋL�^�p
    particleDataBuffer[particleIndex].parameter.w = 0;
    
    particleDataBuffer[particleIndex].position = emitParticleBuffer[emitIndex].position;
    particleDataBuffer[particleIndex].rotation = emitParticleBuffer[emitIndex].rotation;
    particleDataBuffer[particleIndex].scale = emitParticleBuffer[emitIndex].scale;
    
    particleDataBuffer[particleIndex].velocity = emitParticleBuffer[emitIndex].velocity;
    particleDataBuffer[particleIndex].acceleration = emitParticleBuffer[emitIndex].acceleration;
    particleDataBuffer[particleIndex].color = emitParticleBuffer[emitIndex].color;
    particleDataBuffer[particleIndex].customData = emitParticleBuffer[emitIndex].customData;
}