RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3);

bool comparer(in ParticleHeader x0, in ParticleHeader x1)
{
    //  ������+1�A���S��0
    //  �����t���O�ł���ΐ[�x��r
    return (x0.alive > x1.alive || (x0.alive == x1.alive && x0.depth > x1.depth));
}