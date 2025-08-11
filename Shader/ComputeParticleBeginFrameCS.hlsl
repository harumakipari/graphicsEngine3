#include "ComputeParticle.hlsli"
RWByteAddressBuffer indirectDataBuffer : register(u2);

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint index = dispatchThreadId.x;
    
    //1�t���[���O�̑��p�[�e�B�N�����{���݂̃t���[���̐����p�[�e�B�N�������h���́h���݂̃t���[���̑��p�[�e�B�N����
    uint previousNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);
    //�s�N�Z���G�~�b�g�p�[�e�B�N���̑��������Z
    uint numEmitParticle = totalEmitCount + indirectDataBuffer.Load(IndirectArgumentsNumEmitPixelParticleIndex);
    indirectDataBuffer.Store(IndirectArgumentsNumEmitPixelParticleIndex, 0);
    numEmitParticle = min(numEmitParticle, maxEmitParticles);
    
    uint currentNumParticle = previousNumParticle + numEmitParticle;
    
    //���݂̃t���[���̑��p�[�e�B�N�����̓V�X�e���̑��p�[�e�B�N�����Ő���
    currentNumParticle = min(systemNumParticles, currentNumParticle);
    
    //�������L�^
    indirectDataBuffer.Store(IndirectArgumentsNumCurrentParticle, currentNumParticle);
    indirectDataBuffer.Store(IndirectArgumentsNumPreviousParticle, previousNumParticle);
    
    //���S�J�E���^�[��������
    indirectDataBuffer.Store(IndirectArgumentsNumDeadParticle, 0);
    
    //�G�~�b�^�[�p��DispatchIndirect�ɋN������ݒ肷��
    uint3 emitDispatch;
    emitDispatch.x = currentNumParticle - previousNumParticle;
    emitDispatch.y = 1;
    emitDispatch.z = 1;
    indirectDataBuffer.Store3(IndirectArgumentsNumEmitParticleDispatchIndirect, emitDispatch);
    
    //�G�~�b�^�[�̐����ԍ���ݒ�
    //�\�[�g����̂łP�t���[���̃p�[�e�B�N���������̂܂܃G�~�b�g�ԍ��ɂȂ�
    //Append/ConsumeStructuredBuffer�͎�����s�v�ɂȂ�
    indirectDataBuffer.Store(IndirectArgumentsNumEmitParticleIndex, previousNumParticle);
    
    //�X�V�p��DispatchIndirect�ɋN������ݒ肷��
    uint3 updateDispatch;
    updateDispatch.x = ((currentNumParticle + (NumParticleThread - 1)) / NumParticleThread);
    updateDispatch.y = 1;
    updateDispatch.z = 1;
    indirectDataBuffer.Store3(IndirectArgumentsUpdateParticleDispatchIndirect, updateDispatch);
}