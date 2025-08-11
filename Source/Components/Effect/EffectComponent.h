#ifndef EFFECT_COMPONENT_H
#define EFFECT_COMPONENT_H

#include <string>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "Components/Base/SceneComponent.h"
#include "Graphics/Effect/Particles.h"
#include "Graphics/Resource/Texture.h"

class EffectComponent :public SceneComponent
{
public:
    enum class EffectType
    {
        BeamCharge,     // �r�[�������O�̗���
        Beam,           // �r�[�����̂���
        Explosion,
        ShockWave,      // �Ռ��g
        Spark,          // �Ή�
    };

    enum class EffectState
    {
        InActive,   // ���N��
        Initlaizeing, // �N������� particle �� initialize ����t���[�������ʂ�
        Active, // �Đ���
        Ending, // �I�����ɉ����������N���������Ƃ�
        Finished,   // ���S�ɏI��
    };

public:
    EffectComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    virtual ~EffectComponent() = default;

    virtual void Initialize() override;

    virtual void Tick(float deltaTime) override;

    void Activate();

    // particle�̈�t���[���̏��������I�������ĂԊ֐�
    void Initialized();

    void Deactivate();

    bool  IsPlay() const;

    void SetEffectType(EffectComponent::EffectType effectType) { this->effecttype_ = effectType; }

    EffectComponent::EffectType GetEffectType() const { return this->effecttype_; }

    EffectState GetEffectState() const { return effectState_; }

    void SetEffectPower(float power) { this->power_ = power; }

    float GetEffectPower() const { return this->power_; }

    float GetEffectMaxPower()const { return this->effectMaxPower_;}

    void SetEffectMaxPower(float maxPower) { this->effectMaxPower_ = maxPower; }

    void SetEffectImpulse(DirectX::XMFLOAT3 impulse) { this->impulse_ = impulse; }

    // �q�b�g���̏Ռ����擾����
    DirectX::XMFLOAT3 GetEffectImpulse() const { return this->impulse_; }

    void SetEffectNormal(DirectX::XMFLOAT3 normal) { this->normal_ = normal; }

    // �q�b�g���̖@�����擾����
    DirectX::XMFLOAT3 GetEffectNormal() const { return this->normal_; }

    // �G�t�F�N�g�������Ԃ�ݒ肷��
    void SetEffectDuration(float time)
    {
        effectDuration_ = time;
        elapsedTime_ = 0.0f;
    }

    // �O�����̃x�N�g�����擾����
    DirectX::XMFLOAT3 GetEffectForward() const { return this->forward_; }

    void SetEffectForward(DirectX::XMFLOAT3 forward) { this->forward_ = forward; }
private:
    EffectComponent::EffectType effecttype_;

    bool isActivated_ = false;  // �N���ς݂�(Activate()���ꂽ���ǂ���)

    bool isFinished_ = false;   // ���o���I��������

    EffectComponent::EffectState effectState_;

    float power_=0.0f;

    DirectX::XMFLOAT3 normal_ = { 0.0f,0.0f,0.0f };

    DirectX::XMFLOAT3 impulse_ = { 0.0f,0.0f,0.0f };

    float elapsedTime_ = 0.0f;

    float effectDuration_ = 0.0f;

    float effectMaxPower_ = 0.0f;

    DirectX::XMFLOAT3 forward_ = { 0.0f,0.0f,0.0f };
};

#endif