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
        BeamCharge,     // ビームを撃つ前の溜め
        Beam,           // ビームそのもの
        Explosion,
        ShockWave,      // 衝撃波
        Spark,          // 火花
    };

    enum class EffectState
    {
        InActive,   // 未起動
        Initlaizeing, // 起動されて particle の initialize を一フレームだけ通る
        Active, // 再生中
        Ending, // 終了時に何か処理を起こしたいとき
        Finished,   // 完全に終了
    };

public:
    EffectComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    virtual ~EffectComponent() = default;

    virtual void Initialize() override;

    virtual void Tick(float deltaTime) override;

    void Activate();

    // particleの一フレームの初期化が終わったら呼ぶ関数
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

    // ヒット時の衝撃を取得する
    DirectX::XMFLOAT3 GetEffectImpulse() const { return this->impulse_; }

    void SetEffectNormal(DirectX::XMFLOAT3 normal) { this->normal_ = normal; }

    // ヒット時の法線を取得する
    DirectX::XMFLOAT3 GetEffectNormal() const { return this->normal_; }

    // エフェクト発現時間を設定する
    void SetEffectDuration(float time)
    {
        effectDuration_ = time;
        elapsedTime_ = 0.0f;
    }

    // 前方向のベクトルを取得する
    DirectX::XMFLOAT3 GetEffectForward() const { return this->forward_; }

    void SetEffectForward(DirectX::XMFLOAT3 forward) { this->forward_ = forward; }
private:
    EffectComponent::EffectType effecttype_;

    bool isActivated_ = false;  // 起動済みか(Activate()されたかどうか)

    bool isFinished_ = false;   // 演出が終了したか

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