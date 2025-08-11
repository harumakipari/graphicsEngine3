#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>

#define NUMTHREAD_X 16

struct Particle
{
    // ���q�̏��
    int state = 0;
    // ���q�̐F
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
    // ���q�̈ʒu
    DirectX::XMFLOAT3 position{ 0,0,0 };
    // ���q�̎���
    float mass = 1.0f;
    // ���q�̉�]�p�x (���W�A��)
    float angle = 0.0f;
    // ���q�̊p���x
    float angularSpeed = 0.0f;
    // ���q�̑��x (�x�N�g���`��)
    DirectX::XMFLOAT3 velocity{ 0,0,0 };
    // ���q�̎��� (�b�P��)
    float lifespan = 1.0f;
    // ���݂̗��q�̌o�ߎ���
    float age = 0.0f;
    // ���q�̃T�C�Y (�������Ə��Ŏ��̃T�C�Y)
    DirectX::XMFLOAT2 size{ 0 };// x : spawn, y: despawn
    // �e�N�X�`���̑I��p�C���f�b�N�X
    int chip = 0;
};

struct ParticleSystem
{
    // ���q�̍ő吔
    const int maxParticleCount;
    // ���q�V�X�e���̒萔�o�b�t�@�p�\����
    struct ParticleSystemConstants
    {
        // ���q�̐����ʒu
        DirectX::XMFLOAT4 emissionPosition{};
        // �����ʒu�̔��a�͈� (x: �ŏ�, y: �ő�)
        DirectX::XMFLOAT2 emissionOffset{ 0.0f,0.0f }; // x: minimum radius , y: maximum radius 
        // ���q�̐����T�C�Y�Ə��ŃT�C�Y
        DirectX::XMFLOAT2 emissionSize{ 0.02f,0.5f }; // x: spawn, y: despawn
        // �����p�x�͈� (���W�A��, x: �ŏ�, y: �ő�)
        DirectX::XMFLOAT2 emissionConeAngle{ 0.0f,0.2f }; // x: minimum radian, y: maximum radian
        // ���q�̐������x�͈�
        DirectX::XMFLOAT2 emissionSpeed{ 0.5f,1.0f }; // x:minimum speed, y: maximum speed
        // �p���x�͈̔�
        DirectX::XMFLOAT2 emissionAngularSpeed{ 0.0f,1.0f }; // x: minimum angular speed, y: maximum angular speed
        // ���q�̎����͈�
        DirectX::XMFLOAT2 lifespan{ 2.2f,2.2f }; // x: minimum second, y: maximum second
        // ���q�����̒x�����Ԕ͈�
        //DirectX::XMFLOAT2 spawnDelay{ 0.0f,/*3.8f*/ 0.55f}; // x: minimum second, y: maximum second
        DirectX::XMFLOAT2 spawnDelay{ 0.1362f,/*3.8692f*/4.4777f }; // x: minimum second, y: maximum second
        // �t�F�[�h�̎��Ԕ͈� (x: �t�F�[�h�C��, y: �t�F�[�h�A�E�g)
        DirectX::XMFLOAT2 fadeDuration{ 0.0f,0.63f }; // x: fade in, y: fade out
        
        //�F�ω�
        DirectX::XMFLOAT4 emissionStartColor{ 1,1,1,1 };
        DirectX::XMFLOAT4 emissionEndColor{ 1,1,1,1 };

        // ���݂̎���
        float time = 0.0f;
        // �t���[���Ԃ̌o�ߎ���
        float deltaTime = 0.0f;
        // �m�C�Y�X�P�[�� 
        float noiseScale = 0.001f;
        // �d��
        //float gravity = -0.1f;
        float gravity = 0.17f;
        // �X�v���C�g�V�[�g�̃O���b�h�T�C�Y 
        DirectX::XMUINT2 spriteSheetGrid{ 1,1 };
        // ���q�̍ő吔 (�Ċm�F�p)
        int maxParticleCount = 0;
        int state = -1; // 0 : starting 1: finished  2: �e�X�g�p
        //float pads[1]; //(�����p)
        //���q���i�ޕ���
        DirectX::XMFLOAT3 direction{ 0,1,0 };// Z ���̕���
        //���q���i�ޑ���
        float strength = 0.6f;
        //���[���h�g�����X�t�H�[��
        DirectX::XMFLOAT4X4 nodeWorldTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        //���[���h�g�����X�t�H�[��
        DirectX::XMFLOAT4X4 worldTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        //�~���̔��a
        DirectX::XMFLOAT2 radius{ 0,0.1f };

        bool loop = false;
        bool none[3]{};
        int type = 0;//�p�[�e�B�N���̎��
        int isStatic = 0;
        int pad__[3];
    };
    int blendMode = 1; // 0:ALPHA, 1:ADD
    ParticleSystemConstants particleSystemData;
    ParticleSystemConstants presetData;

    //�p�[�e�B�N���e�N�X�`��
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleTexture;

    // GPU���\�[�X (���q�f�[�^��ێ�����o�b�t�@)
    Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv;
    // GPU�V�F�[�_ (���_�V�F�[�_�A�s�N�Z���V�F�[�_�A�W�I���g���V�F�[�_�Ȃ�)
    Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGeometricShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleComputeShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerComputeShader;
    // �萔�o�b�t�@ (CPU����GPU�Ƀf�[�^�𑗂邽��)
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    ParticleSystem(ID3D11Device* pDevice, int particleCount);
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) noexcept = delete;
    ParticleSystem& operator=(ParticleSystem&&) noexcept = delete;
    virtual ~ParticleSystem() = default;
    // ���q�̓����𓝍�����֐�
    void Integrate(ID3D11DeviceContext* immediateContext, float deltaTime);
    // ���q�̏������֐�
    void Initialize(ID3D11DeviceContext* immediateContext, float deltaTime);
    // ���q��`�悷��֐�
    void Render(ID3D11DeviceContext* immediateContext);

    //GUI�`��
    void DrawGUI();

};