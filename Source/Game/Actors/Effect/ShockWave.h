#ifndef SHOCK_WAVE_H
#define SHOCK_WAVE_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Game/ShockWaveCollisionComponent.h"

class ShockWave :public Actor
{
public:
    ShockWave(std::string modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Effect/Models/ring.gltf");
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
        auto shockWave = this->NewSceneComponent<ShockWaveCollisionComponent>("shockWave", "skeltalComponent");
        //shockWave->Initialize(startRadius_, endRadius_, durationSeconds_, power_);

    }


    void SetWaveDetails(float startRadius, float endRadius, float durationSeconds, float beamPower)
    {
        this->startRadius_ = startRadius;
        this->endRadius_ = endRadius;
        this->durationSeconds_ = durationSeconds;
        this->elapsedTime_ = 0.0f;
        this->power_ = beamPower;
        // è’åÇîgÇí«â¡Ç∑ÇÈ
    }


    void Update(float deltaTime)override
    {
        elapsedTime_ += deltaTime;
        float t = std::clamp(elapsedTime_ / durationSeconds_, 0.0f, 1.0f);
        float currentRadius = std::lerp(startRadius_, endRadius_, t);
        DirectX::XMFLOAT3 scale = { currentRadius,currentRadius,currentRadius };
        skeltalMeshComponent->SetRelativeScaleDirect(scale);
    }

    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override
    {
    }


    void NotifyHit(CollisionComponent* selfComp, CollisionComponent* otherComp, Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
    }


    //Å@collisionComponentÅ@Ç™ Dynamic ÇÃï®Ç∆ìñÇΩÇ¡ÇΩéûÇ…í ÇÈ
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
    }

private:
    float power_ = 0.0f;
    float startRadius_ = 0.1f;
    float endRadius_ = 5.0f;
    float durationSeconds_ = 1.0f;
    float elapsedTime_ = 0.0f;

};




#endif // !SHOCK_WAVE_H
