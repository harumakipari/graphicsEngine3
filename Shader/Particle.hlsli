cbuffer PARTICLE_SYSTEM_CONSTANTS : register(b9)
{
    float4 emissionPosition; // ���q�̐����ʒu
    float2 emissionOffset; //�����ʒu�̔��a�͈� (x: �ŏ�, y: �ő�)
    float2 emissionSize; // ���q�̐����T�C�Y�Ə��ŃT�C�Y
    float2 emisionConeAngle; // �����p�x�͈� (���W�A��, x: �ŏ�, y: �ő�)
    float2 emissionSpeed; // ���q�̐������x�͈�
    float2 emissionAngularSpeed; // �p���x�͈̔�
    
    float2 lifespan; // ���q�̎����͈�
    float2 spawnDelay; // ���q�����̒x�����Ԕ͈�
    float2 fadeDuration; // �t�F�[�h�̎��Ԕ͈� (x: �t�F�[�h�C��, y: �t�F�[�h�A�E�g)
    
    float4 emissionStartColor;//�F�ω��O�̐F
    float4 emissionEndColor;//�F�ω���̐F
    
    float currentTime; // ���݂̎���
    float deltaTime; // �t���[���Ԃ̌o�ߎ���
    float _noiseScale; // �m�C�Y�X�P�[�� 
    float gravity; // �d��
    
    uint2 spriteSheetGrid; // �X�v���C�g�V�[�g�̃O���b�h�T�C�Y 
    uint maxParticleCount; // ���q�̍ő吔 (�Ċm�F�p)
    
    float3 direction; //����
    float strength; //�����̋���
    
    row_major float4x4 nodeWorldTransform;
    row_major float4x4 worldTransform;
    float2 radius;
    
    bool loop;// ���[�v���邩�ǂ���
    int type = 0;//�p�[�e�B�N���̎��
    int isStatic;//�����Ȃ����ǂ���
}

//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPositon;
//    float4 colorLight; //w colorPower
//    //row_major float4x4 lightViewProjection; // SHADOW
//    // PARTICLES
//    row_major float4x4 view;
//    row_major float4x4 projection;
//    row_major float4x4 inverseProjection;
//    row_major float4x4 inverseViewProjection;

//    float iblIntensity;
//}
#include "Constants.hlsli"

struct VS_OUT
{
    uint vertexId : VERTEXID;
};

struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct Particle
{
    int state; // ���q�̏��
    
    float4 color; // ���q�̐F
    
    float3 position; // ���q�̈ʒu  World space position
    float mass; // ���q�̎���
    
    float angle; // ���q�̉�]�p�x (���W�A��)
    float angularSpeed; // ���q�̊p���x
    float3 velocity; // ���q�̑��x (�x�N�g���`��) World space Velocity
    
    float lifespan; // ���q�̎��� (�b�P��)
    float age; // ���݂̗��q�̌o�ߎ��� The current age counting down from lifespan to zero
    
    float2 size; // ���q�̃T�C�Y (�������Ə��Ŏ��̃T�C�Y)
    
    uint chip; // �e�N�X�`���̑I��p�C���f�b�N�X
    
    
};

#define PI 3.14159265358979
float Rand(float n)
{
    // Return value is greater than or equal to 0 and less than 1.
    return frac(sin(n) * 43758.5453123);
}

// Returns a random value in the specified range [min, max] 
float RandRange(float seed, float min, float max)
{
    return lerp(min, max, Rand(seed));
}

#define NUMTHREADS_X 16

void Spawn(in uint id, inout Particle p)
{
    

#if 1
    //float theta = Rand((float) id / maxParticleCount * 2.0f * PI);
    //float r = Rand(theta * PI);
    //float theta = Rand(2.0f * PI * 0.01f * id); 
    //float phi = Rand(2.0f * PI * 0.01f * id);
    //float r = Rand(1.0 + id) * 1.0;
    //float theta = RandRange(2.0f * PI * 0.01f * id, -0.15 * PI, 0.15 * PI);
    float theta = RandRange(2.0f * PI * 0.01f * id, 0, 2.0 * PI);
    float phi = RandRange(2.0f * PI * 0.01f * id, -PI * 0.5, PI * 0.5);
    //float r = RandRange(1.0 + id, 0, 1);
    float r = RandRange(1.0 + id, radius.x, radius.y);
    
#if 1
    p.position = emissionPosition.xyz/*world space*/;
    //p.position = mul(worldTransform, float4(p.position, 1)).xyz;
    
#if 1
    p.velocity.z = -1;
    float2 offset = float2(r * (cos(theta)), r * sin(theta));
    float2 velocityXY = offset;
    p.velocity.xy = velocityXY;
#else
    p.velocity.x =  1;
    float2 offset = float2(r * (sin(theta)), r * cos(theta));
    float2 velocityYZ = offset;
    p.velocity.yz = velocityYZ;
    //p.velocity.y = 1;
    //float2 offset = float2(r * (cos(theta)), r * sin(theta));
    //float2 velocityXZ = offset;
    //p.velocity.xz = velocityXZ;
#endif
    //p.velocity = normalize(p.velocity);
    
    //p.position = mul(worldTransform, float4(emisionPosition.xyz, 1.0)).xyz;
    //p.velocity = mul((float3x3) worldTransform, p.velocity);
    //p.velocity = mul(mul(nodeWorldTransform, worldTransform), float4(p.velocity, 0)).xyz;
    
    
    //p.velocity = mul(mul(worldTransform, nodeWorldTransform), float4(p.velocity, 0)).xyz;
    //p.velocity = mul(float4(p.velocity, 0), mul(worldTransform, nodeWorldTransform)).xyz;
    p.velocity = mul(float4(p.velocity, 0), worldTransform).xyz;
    
    
    //p.velocity = mul(nodeWorldTransform, float4(p.velocity, 0)).xyz;
    p.velocity = normalize(p.velocity);
    //p.velocity.x =/* direction **/ normalize(position.x - emisionPosition.x);
    //p.velocity.y =/* direction **/ normalize(position.y - emisionPosition.y);
#else
     p.velocity.x = (direction.x);
    float2 offset = float2(r * (cos(theta)), r * sin(theta));
    p.position = emisionPosition.xyz;
    float2 velocityXY = normalize(offset);
    p.velocity.z = direction.z * velocityXY.x - direction.y * velocityXY.y;
    p.velocity.y = direction.z * velocityXY.y + direction.y * velocityXY.x;

    
    //float3 randomDir = normalize(float3(cos(theta) * r, sin(theta) * r, direction.z));
    //p.velocity = normalize(direction + randomDir) * strength;
#endif
    float f0 = Rand((float) id / maxParticleCount / 10000.0); //�Ԋu���L����
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI + 0.01f * id);
#else
#if 1
    //float f0 = Rand((float) id / maxParticleCount * 2.0f * PI);
    float f0 = Rand((float) id / maxParticleCount /30000.0);//�Ԋu���L����
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI);
#else
    float f0 = 0.5f;
    float f1 = 0.6f;
    float f2 = Rand(f1 * PI);
    float f3 = 0.8f;
    //p.age = 0;
#endif
    
    float3 offset = 0;
#if 1
    float radius = lerp(emissionOffset.x, emissionOffset.y, f0);
    offset.x = radius * cos(f1 * 2.0 * PI);
    offset.z = radius * sin(f1 * 2.0 * PI);
#endif
    p.position = emisionPosition.xyz + offset;
    
    float phi = 2.0 * PI * f0;
    float theta = lerp(emisionConeAngle.x, emisionConeAngle.y, f1);
    
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    
    p.velocity.x = sinTheta * cosPhi;
    //p.velocity.y = cosTheta;
    p.velocity.y = 0.0;
    //p.velocity.y = (Rand(1.0f) - 0.5f) * 5.0;
    p.velocity.z = sinTheta * sinPhi;
    p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
#endif
    //p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
    p.color = 1.0;
    
    p.mass = 1.0;
    
#if 1
    p.angle = PI * f0;
    p.angularSpeed = lerp(emissionAngularSpeed.x, emissionAngularSpeed.y, f3);
#else
    p.angle =0.0;
    p.angularSpeed = 0.0;
#endif
    
    //p.lifespan = RandRange(f0, lifespan.x, lifespan.y);
    p.lifespan = lerp(lifespan.x, lifespan.y, f0);
    p.age = -lerp(spawnDelay.x, spawnDelay.y, f0);
    p.state = 0;
    p.size.x = emissionSize.x * f1;
    p.size.y = emissionSize.y * f2;
    
    int count = spriteSheetGrid.x * spriteSheetGrid.y;
    p.chip = f3 * count;
}

void SpawnTest(in uint id, inout Particle p)
{
    float f0 = Rand((float) id / maxParticleCount * 2.0 * PI);
    float f1 = Rand(f0 * PI);
    float f2 = Rand(f1 * PI);
    float f3 = Rand(f2 * PI);
    
    float3 offset = 0;
    float radius = lerp(emissionOffset.x, emissionOffset.y, f0);
#if 0
    offset.x = radius * cos(f1 * 2.0 * PI);
    offset.z = radius * sin(f1 * 2.0 * PI);
#else
    offset.x = radius * cos(f3 * 2.0 * PI);
    offset.z = radius * sin(f3 * 2.0 * PI);
#endif
    p.position = emissionPosition.xyz + offset;
#if 0
    float phi = 2.0 * PI * f0;
    float theta = lerp(emisionConeAngle.x, emisionConeAngle.y, f1);
#else
    float phi = 2.0 * PI * f0;
    float theta = clamp(f2, emisionConeAngle.x, emisionConeAngle.y);
#endif
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    
    //p.velocity.x = sinTheta * cosPhi;
    //p.velocity.y = cosTheta;
    //p.velocity.z = sinTheta * sinPhi;
    
    
    // ���[�J����Ԃ̕����iY�����S�j
    float3 localDir;
    localDir.x = sinTheta * cosPhi;
    localDir.y = cosTheta;
    localDir.z = sinTheta * sinPhi;
    
    float3 worldDir;

    switch (type)
    {
        case 0:
        {
            if (dot(direction, direction) < 1e-6)
            {
                worldDir = 0;
            }
            else
            {
                // Y�� �� direction �ւ̉�]�}�g���N�X���\��
                float3 dir = normalize(direction); // ���S�̂��ߐ��K��
                float3 up = abs(dir.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
                float3 right = normalize(cross(up, dir));
                float3 forward = normalize(cross(dir, right));
                float3x3 rotMatrix = float3x3(right, dir, forward); // �E��n�F�� right, up, forward

                // ��]�K�p���Đ��E��ԕ����ɕϊ�
                worldDir = mul(localDir, rotMatrix);
            }
            break;
        }
        case 1://���S�ɏW�܂�G�t�F�N�g
        {
            p.position += (localDir * strength);
            worldDir = emissionPosition.xyz - p.position;
            break;
        }
    };
    
    p.velocity = worldDir * lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
    p.color = 1.0;
    
    p.mass = 1.0;
    
#if 1
    p.angle = PI * f0;
    p.angularSpeed = lerp(emissionAngularSpeed.x, emissionAngularSpeed.y, f3);
#else
    p.angle = 0.0;
    p.angularSpeed = 0.0;
#endif
    
    p.lifespan = lerp(lifespan.x, lifespan.y, f0);
    p.age = -lerp(spawnDelay.x, spawnDelay.y, f0);
    p.state = 0;
    p.size.x = emissionSize.x * f1;
    p.size.y = emissionSize.y * f2;
    
    int count = spriteSheetGrid.x * spriteSheetGrid.y;
    p.chip = f3 * count;
}


float FadeIn(float duration, float age, float exponent)
{
    //return clamp(age / duration, 0.0, 1.0);
    return pow(smoothstep(0.0, 1.0, age / duration), exponent);
}

float FadeOut(float duration, float age, float lifespan, float exponent)
{
	//return clamp((lifespan - age) / duration, 0.0, 1.0);  
    return pow(smoothstep(0.0, 1.0, (lifespan - age) / duration), exponent);
}




