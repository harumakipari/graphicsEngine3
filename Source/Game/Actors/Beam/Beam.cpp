#include "Beam.h"

#include "Physics/Physics.h"
#include "Physics/DefferdPhysicsOperation.h"

#include "Game/Actors/Stage/Building.h"
#include "Game/Actors/Stage/BossBuilding.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Actors/Enemy/TutorialEnemy.h"
#include "Game/Actors/Stage/Objects/StageProp.h"
#include "Components/Game/LifeTimeComponent.h"


// キネマティック同士の当たり判定を検知する
void Beam::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    if (auto build = std::dynamic_pointer_cast<Building>(hitPair.second->GetActor()))
    {
        //if (build->GetDestroy())
        //{// ビルが壊れていたら無視する
        //    return;
        //}
        if (!onceSetPosition)
        {
            effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
            DirectX::XMFLOAT3 pos = GetPosition();
            effectSparkComponent->SetWorldLocationDirect(GetPosition());
            effectSparkComponent->SetEffectPower(itemPower);
            //effectSparkComponent->SetEffectNormal(normal);
            //effectSparkComponent->SetEffectImpulse(impulse);
            effectSparkComponent->Activate();
            onceSetPosition = true;
        }
        this->itemCount -= 6;

        char buf[256];
        sprintf_s(buf, "Now Beam Power: %d\n", itemCount);
        OutputDebugStringA(buf);

        if (itemCount <= 0)
        {
            this->itemCount = 0;
            this->SetPendingDestroy();
            //effectBeamComponent->Deactivate();
            //this->DestroyComponentByName("effectBeamComponet");

            ////Physics::EnqueueDefferfOperations({ DefferdPhysicsOperation::Type::DestroyComponent,sphereComponent.get() });
            //skeltalMeshComponent->SetIsVisible(false);

            //this->ScheduleDestroyComponentByName("skeltalComponent");
            //this->ScheduleDestroyComponentByName("sphereComponent");
            //this->ScheduleDestroyComponentByName("effectBeamComponet");

            //auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
            //lifeTimeComponent->SetLifeTime(3.0f);
        }
        //int a = 0;
    }
    else if (auto bossBuild = std::dynamic_pointer_cast<BossBuilding>(hitPair.second->GetActor()))
    {
        //if (bossBuild->GetDestroy())
        //{// ビル爆弾が壊れていたら
        //    return;
        //}
        if (!onceSetPosition)
        {
            effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
            DirectX::XMFLOAT3 pos = GetPosition();
            effectSparkComponent->SetWorldLocationDirect(GetPosition());
            effectSparkComponent->SetEffectPower(itemPower);
            //effectSparkComponent->SetEffectNormal(normal);
            //effectSparkComponent->SetEffectImpulse(impulse);
            effectSparkComponent->Activate();
            onceSetPosition = true;
            this->itemCount -= 6;
        }

        if (itemCount <= 0)
        {
            this->itemCount = 0;
            //this->SetPendingDestroy();

        //effectBeamComponent->Deactivate();
        //this->DestroyComponentByName("effectBeamComponet");

        ////Physics::EnqueueDefferfOperations({ DefferdPhysicsOperation::Type::DestroyComponent,sphereComponent.get() });
            //skeltalMeshComponent->SetIsVisible(false);

            //this->ScheduleDestroyComponentByName("skeltalComponent");
            //this->ScheduleDestroyComponentByName("sphereComponent");
            //this->ScheduleDestroyComponentByName("effectBeamComponet");
            this->SetPendingDestroy();

            //auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
            //lifeTimeComponent->SetLifeTime(3.0f);
            //int a = 0;
        }
    }
    else if (auto tutorialEnemy = std::dynamic_pointer_cast<TutorialEnemy>(hitPair.second->GetActor()))
    {
        if (!onceSetPosition)
        {
            effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
            DirectX::XMFLOAT3 pos = GetPosition();
            effectSparkComponent->SetWorldLocationDirect(GetPosition());
            effectSparkComponent->SetEffectPower(itemPower);
            //effectSparkComponent->SetEffectNormal(normal);
            //effectSparkComponent->SetEffectImpulse(impulse);
            effectSparkComponent->Activate();
            onceSetPosition = true;
            if (TutorialSystem::GetCurrentStep() == TutorialStep::CreateBuild)
            {
                TutorialSystem::SetCurrentStep(TutorialStep::CreateBuild);
            }
            if (TutorialSystem::GetCurrentStep() == TutorialStep::ThirdAttack)
            {
                TutorialSystem::SetCurrentStep(TutorialStep::ManyCollect2);
            }
            //if (TutorialSystem::GetCurrentStep() == TutorialStep::ManyCollect2)
            //{
            //    TutorialSystem::SetCurrentStep(TutorialStep::ManyCollect2);
            //}
        }
        //this->SetPendingDestroy();
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("effectBeamComponet");

        //effectBeamComponent->Deactivate();
        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");
        //auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        auto  lifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);

    }
    else if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(hitPair.second->GetActor()))
    {
        if (hitPair.second->name() != "capsuleComponent")
        {// enemy の体じゃなかったら
            return;
        }
        if (!onceSetPosition)
        {
            effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
            DirectX::XMFLOAT3 pos = GetPosition();
            effectSparkComponent->SetWorldLocationDirect(GetPosition());
            effectSparkComponent->SetEffectPower(itemPower);
            //effectSparkComponent->SetEffectNormal(normal);
            //effectSparkComponent->SetEffectImpulse(impulse);
            effectSparkComponent->Activate();
            onceSetPosition = true;
        }
        //this->SetPendingDestroy();
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("effectBeamComponet");

        //effectBeamComponent->Deactivate();
        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");
        //auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        auto  lifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);

    }
    else if (auto stageProps = std::dynamic_pointer_cast<StageProp>(hitPair.second->GetActor()))
    {
        //this->SetPendingDestroy();
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("effectBeamComponet");

        //effectBeamComponent->Deactivate();
        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");

        //auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        auto  lifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);
    }
    else if (auto stageProps = std::dynamic_pointer_cast<Stage>(hitPair.second->GetActor()))
    {
        TutorialSystem::AchievedAction(TutorialStep::FirstAttack);
        switch (TutorialSystem::GetCurrentStep())
        {
        case TutorialStep::FirstAttack:
            TutorialSystem::SetCurrentStep(TutorialStep::Collect);
            break;
        case TutorialStep::CreateBuild:
            TutorialSystem::SetCurrentStep(TutorialStep::CreateBuild);
            break;
        case TutorialStep::ThirdAttack:
            TutorialSystem::SetCurrentStep(TutorialStep::ManyCollect2);
            break;
        default:
            break;
        }
        this->SetPendingDestroy();
    }
    int a = 0;

}

//　collisionComponent　が Dynamic の物と当たった時に通る
void Beam::NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
#if 0
    effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
    effectSparkComponent->SetWorldLocationDirect(hitPos);
    effectSparkComponent->SetEffectPower(itemPower);
    effectSparkComponent->SetEffectNormal(normal);
    effectSparkComponent->SetEffectImpulse(impulse);
    effectSparkComponent->Activate();

    if (auto build = dynamic_cast<Building*>(otherActor))
    {
        //effectBeamComponent->Deactivate();
        //this->DestroyComponentByName("effectBeamComponet");

        ////Physics::EnqueueDefferfOperations({ DefferdPhysicsOperation::Type::DestroyComponent,sphereComponent.get() });
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");
        this->ScheduleDestroyComponentByName("effectBeamComponet");
        //this->SetPendingDestroy();

        auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);
        //int a = 0;
    }
    else if (auto enemy = dynamic_cast<RiderEnemy*>(otherActor))
    {
        //this->SetPendingDestroy();
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("effectBeamComponet");

        //effectBeamComponent->Deactivate();
        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");
        auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);
    }
    else if (auto stageProps = dynamic_cast<StageProp*>(otherActor))
    {
        //this->SetPendingDestroy();
        skeltalMeshComponent->SetIsVisible(false);

        this->ScheduleDestroyComponentByName("effectBeamComponet");

        //effectBeamComponent->Deactivate();
        this->ScheduleDestroyComponentByName("skeltalComponent");
        this->ScheduleDestroyComponentByName("sphereComponent");

        auto  lifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        lifeTimeComponent->SetLifeTime(3.0f);
    }
    else if (auto stage = dynamic_cast<Stage*>(otherActor))
    {
        this->SetPendingDestroy();
    }
    int a = 0;
#endif // 0
}

