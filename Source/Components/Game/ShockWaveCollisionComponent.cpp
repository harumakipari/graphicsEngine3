#include "ShockWaveCollisionComponent.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Stage/Objects/StageProp.h"
#include "Game/Actors/Stage/Building.h"
#include "Game/Actors/Stage/BossBuilding.h"

#include "Game/Utils/ShockWaveTargetRegistry.h"
#include "Components/Game/LifeTimeComponent.h"

void ShockWaveCollisionComponent::Tick(float deltaTime)
{
    //SphereComponent::Tick(deltaTime);

    elapsedTime_ += deltaTime;
    float t = std::clamp(elapsedTime_ / durationSeconds_, 0.0f, 1.0f);
    float currentRadius = std::lerp(startRadius_, endRadius_, t);
    //ResizeSphere(currentRadius);


    // �S�A�N�^�[�ɑ΂��ă`�F�b�N
    //for (auto actor : ActorManager::allActors_)
    for (auto& actor : ShockWaveTargetRegistry::GetTargets())
    {
        if (!actor || !actor->GetIsValid())
        {
            continue;
        }
        if (actor.get() == owner_.lock().get())
        {
            continue;
        }
        auto building = std::dynamic_pointer_cast<Building>(actor);
        auto bossBuilding = std::dynamic_pointer_cast<BossBuilding>(actor);
        auto stageProp = std::dynamic_pointer_cast<StageProp>(actor);
        if (!building && !stageProp && !bossBuilding)
        {
            continue;
        }

        DirectX::XMFLOAT3 pos = actor->GetPosition();
        DirectX::XMFLOAT3 p = GetComponentLocation();

        float dist = Distance(pos, p);
        if (dist <= currentRadius)
        {
            // ���d�K�p��h��
            if (alreadyAffected.contains(actor.get()))
                continue;

            alreadyAffected.insert(actor.get());

            auto comp = actor->GetComponent<CollisionComponent>();
            if (comp)
            {
                DirectX::XMFLOAT3 dir = Subtract(actor->GetPosition(), GetComponentLocation());
                Normalize(dir);
                float power = 40.0f * (1.0f - dist / endRadius_); // ��������
                DirectX::XMFLOAT3 impulse = Multiply(dir, power);


                // �����Ȃ�_���[�W���^����
                if (auto building = dynamic_cast<Building*>(actor.get()))
                {
                    DirectX::XMFLOAT3 hitPos = actor->GetPosition();
                    building->CallHitShockWave(power, beamItemCount_, hitPos, dir, impulse);
                }
                else if (auto bossBuilding = dynamic_cast<BossBuilding*>(actor.get()))
                {
                    DirectX::XMFLOAT3 hitPos = actor->GetPosition();
                    bossBuilding->CallHitShockWave(power, beamItemCount_, hitPos, dir, impulse);
                }
                else
                {
                    comp->SetKinematic(false); // ������΂���悤��
                    comp->AddImpulse(impulse);
                }
            }
        }
    }

    if (elapsedTime_ > durationSeconds_)
    {
        // �I����͔j������
        this->GetOwner()->ScheduleDestroyComponentByName(this->name_);
    }
}

