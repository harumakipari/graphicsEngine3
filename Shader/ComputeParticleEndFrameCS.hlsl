#include "ComputeParticle.hlsli"

RWByteAddressBuffer indirectDataBuffer : register(u2);

[numthreads(1, 1, 1)]
void main()
{
    //���S�J�E���^�[���擾��������
    uint destroyCounter = indirectDataBuffer.Load(IndirectArgumentsNumDeadParticle);
    indirectDataBuffer.Store(IndirectArgumentsNumDeadParticle, 0);
    
    //���݂̃t���[���ł̃p�[�e�B�N���������Čv�Z
    uint currentNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);
    indirectDataBuffer.Store(IndirectArgumentsNumCurrentParticle, currentNumParticle - destroyCounter);
    
    //�`��R�[�����������Ō��߂�
    indirectDataBuffer.Store(IndirectArgumentsDrawIndirect, currentNumParticle - destroyCounter);
}