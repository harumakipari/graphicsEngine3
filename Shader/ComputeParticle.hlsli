#include "Constants.hlsli"

//�p�[�e�B�N���X���b�h��
static const int NumParticleThread = 1024;

//�����p�[�e�B�N���\����
struct EmitParticleData
{
    float4 parameter; // x : �p�[�e�B�N�������^�C�v, y : �c�萶������, z : ��������, w : ��

    float4 position; // �������W
    float4 rotation; // ��]���
    float4 scale; //�g�k��� (xy:startSize, zw: endSize)

    float4 velocity; // ����
    float4 acceleration; // �����x
    
    float4 color; // �F���
    
    float4 customData; //�J�X�^���f�[�^
};

//�p�[�e�B�N���\����
struct ParticleData
{
    float4 parameter; // x : �p�[�e�B�N�������^�C�v, y : �c�萶������, z : ��������,�@w : ��

    float4 position; // �������W
    float4 rotation; // ��]���
    float4 scale; //�g�k��� (xy:startSize, zw: endSize)

    float4 velocity; // ����
    float4 acceleration; // �����x

    float4 texcoord; //  UV���W
    float4 color; // �F���
    
    float4 customData; // �J�X�^���f�[�^
};

//�p�[�e�B�N���w�b�_�[�\����
struct ParticleHeader
{
    uint alive; //�����t���O
    uint particleIndex; //�p�[�e�B�N���ԍ�
    float depth; //�[�x
    uint dummy;
};

//IndirectDataBuffer�ւ̃A�N�Z�X�p�o�C�g�I�t�Z�b�g
static const uint IndirectArgumentsNumCurrentParticle = 0;
static const uint IndirectArgumentsNumPreviousParticle = 4;
static const uint IndirectArgumentsNumDeadParticle = 8;
static const uint IndirectArgumentsNumEmitParticleDispatchIndirect = 12;

//DrawIndirect�p�\����
struct DrawIndirect
{
    uint vertexCountPerInstance;
    uint instanceCount;
    uint startVertexLocation;
    uint startInstanceLocation;
};
static const uint IndirectArgumentsUpdateParticleDispatchIndirect = 24;
static const uint IndirectArgumentsNumEmitParticleIndex = 36;
static const uint IndirectArgumentsNumEmitPixelParticleIndex = 40;
static const uint IndirectArgumentsDrawIndirect = 44;

//=========================================================================================
//  �ėp���
cbuffer COMPUTE_PARTICLE_COMMON_CONSTANT_BUFFER : register(b10)
{
    float deltaTime;
    uint2 textureSplitCount;
    uint systemNumParticles;
    uint totalEmitCount;
    
    uint maxEmitParticles;
    uint2 commonDummy;
};

//�o�C�g�j�b�N�\�[�g���
cbuffer COMPUTE_PARTICLE_BITONIC_SORT_CONSTANT_BUFFER : register(b11)
{
    uint increment;
    uint direction;
    uint sortDummy[2];
};
static const uint BitonicSortB2Thread = 256;
static const uint BitonicSortC2Thread = 512;

//=========================================================================================
//  ���_�V�F�[�_�[����W�I���g���V�F�[�_�[�ɓ]��������
struct GS_IN
{
    uint vertexId : VERTEX_ID;
};

//  �W�I���g���V�F�[�_�[����s�N�Z���V�F�[�_�[�ɓ]��������
struct PS_IN
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
