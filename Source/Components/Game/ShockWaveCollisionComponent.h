#ifndef SHOCK_WAVE_COLLISION_COMPONENT_H
#define SHOCK_WAVE_COLLISION_COMPONENT_H

#include <unordered_set>
#include <DirectXMath.h>

#include "Components/Base/SceneComponent.h"
//#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Stage/Objects/StageProp.h"
using namespace DirectX;

class ShockWaveModelComponent :public SkeltalMeshComponent
{
public:
    ShockWaveModelComponent(const std::string& name, std::shared_ptr<Actor> owner) : SkeltalMeshComponent(name, owner) {}

    void Initialize(float startRadius, float endRadius, float durationSeconds, float beamPower)
    {
        this->startRadius_ = startRadius;
        this->endRadius_ = endRadius;
        this->durationSeconds_ = durationSeconds;
        this->elapsedTime_ = 0.0f;
        isShockWaveStart_ = true;
        SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 0.1f, 0.1f));
    }

    void Tick(float deltaTime)override
    {
        if (isShockWaveStart_)
        {
            elapsedTime_ += deltaTime;
            float t = std::clamp(elapsedTime_ / durationSeconds_, 0.0f, 1.0f);
            float currentRadius = std::lerp(startRadius_, endRadius_, t);
            SetRelativeScaleDirect(DirectX::XMFLOAT3(currentRadius, 1.0f, currentRadius));
        }

        if (elapsedTime_ > durationSeconds_)
        {
            // 終了後は破棄する
            this->GetOwner()->ScheduleDestroyComponentByName(this->name_);
        }
    }

private:
    float startRadius_ = 0.1f;
    float endRadius_ = 5.0f;
    float durationSeconds_ = 1.0f;
    float elapsedTime_ = 0.0f;
    bool isShockWaveStart_ = false;
};

class ShockWaveCollisionComponent :public SceneComponent
{
public:
    ShockWaveCollisionComponent(const std::string& name, std::shared_ptr<Actor> owner) : SceneComponent(name, owner) {}

    void Initialize(float startRadius, float endRadius, float durationSeconds, float beamPower, int beamItemCount)
    {
        this->startRadius_ = startRadius;
        this->endRadius_ = endRadius;
        this->durationSeconds_ = durationSeconds;
        this->elapsedTime_ = 0.0f;
        this->power_ = beamPower;
        this->beamItemCount_ = beamItemCount;
    }

    void Tick(float deltaTime)override;

private:
    float power_ = 0.0f;

    std::unordered_set<Actor*> alreadyAffected;
    float Distance(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    XMFLOAT3 Subtract(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }

    XMFLOAT3 Multiply(const XMFLOAT3& v, float scale)
    {
        return { v.x * scale, v.y * scale, v.z * scale };
    }

    void Normalize(XMFLOAT3& v)
    {
        float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len > 0.0001f)
        {
            v.x /= len;
            v.y /= len;
            v.z /= len;
        }
    }

    float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    float startRadius_ = 0.1f;
    float endRadius_ = 5.0f;
    float durationSeconds_ = 1.0f;
    float elapsedTime_ = 0.0f;
    int beamItemCount_ = 0; // ビームに含まれているアイテムの数
};

#endif // !SHOCK_WAVE_COLLISION_COMPONENT_H

