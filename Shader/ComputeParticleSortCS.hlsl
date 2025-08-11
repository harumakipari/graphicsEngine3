#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0);
RWByteAddressBuffer indirectDataBuffer : register(u2);

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3);

[numthreads(1, 1, 1)]
void main()
{
    //�p�[�e�B�N���������擾
    uint currentNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);
    if (currentNumParticle == 0)
        return;
    
    //�ȈՓI�Ƀ\�[�g���s��
    //�N�C�b�N�\�[�g�Ȃǂ̍ċA�֐����g�����̂̓R���s���[�g�V�F�[�_�[�̕��Ő������������Ă��邽�ߖ����B
    for (int h = currentNumParticle / 2; h > 0; h /= 2)
    {
        for (int i = h; i < currentNumParticle; i += 1)
        {
            ParticleHeader k = particleHeaderBuffer[i];
            
            int j;
            for (j = i; j >= h && (particleHeaderBuffer[j - h].alive < k.alive); j -= h)
            {
                particleHeaderBuffer[j] = particleHeaderBuffer[j - h];
            }
            
            particleHeaderBuffer[j] = k;
        }

    }

}