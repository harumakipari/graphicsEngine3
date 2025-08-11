#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

// C++ �W�����C�u����
#include <vector>

// �����C�u����
#include <PxPhysicsAPI.h>

// �v���W�F�N�g�̑��̃w�b�_
#include "Core/Actor.h"
#include "Components/CollisionShape/CollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"


// �L�l�}�e�B�b�N���m��p�̃R���W�����V�X�e��
class CollisionSystem
{
public:
    static DirectX::XMFLOAT3 GetPushVectorForActor(Actor* actor)
    {
        auto it = actorPushMap_.find(actor);
        if (it != actorPushMap_.end())
        {
            return it->second;
        }
        return { 0.0f, 0.0f, 0.0f };
    }

    static void DetectAndResolveCollisions()
    {
        using namespace physx;

        actorPushMap_.clear(); // ������

        // expired �ȁ@weak_ptr�@
        allComponents_.erase(
            std::remove_if(
                allComponents_.begin(), allComponents_.end(),
                [](const std::pair<std::weak_ptr<Actor>, std::weak_ptr<CollisionComponent>>& p)->bool
                {
                    return p.first.expired() || p.second.expired();
                }),
            allComponents_.end());


        // �Փ˔��聕�����o��
        for (size_t i = 0; i < allComponents_.size(); ++i)
        {
            for (size_t j = i + 1; j < allComponents_.size(); ++j)
            {
                auto aActorWeak = allComponents_[i].first;
                auto bActorWeak = allComponents_[j].first;

                auto aComponentWeak = allComponents_[i].second;
                auto bComponentWeak = allComponents_[j].second;

                auto aActorShared = aActorWeak.lock();
                auto bActorShared = bActorWeak.lock();

                auto aComponentShared = aComponentWeak.lock();
                auto bComponentShared = bComponentWeak.lock();

                if (!aActorShared || !bActorShared || !aComponentShared || !bComponentShared)
                {
                    // �ǂꂩ�L������Ȃ��ꍇ�̓X�L�b�v�i�������͌��allComponents_����폜����j
                    continue;
                }

                Actor* aActor = aActorShared.get();
                Actor* bActor = bActorShared.get();
                CollisionComponent* aComponent = aComponentShared.get();
                CollisionComponent* bComponent = bComponentShared.get();

                if (!aActor->isActive || !bActor->isActive)
                {
                    continue;
                }

                // �ǂ��炩�̓����蔻��̃t���O�������Ȃ�X�L�b�v
                if (!aComponent->IsCollide() || !bComponent->IsCollide())
                {
                    continue;
                }

                // ����A�N�^�[�Ȃ�X�L�b�v
                if (aComponent->GetOwner() == bComponent->GetOwner())
                {
                    continue;
                }
                CollisionComponent::CollisionResponse aToB = aComponent->GetResponseTo(bComponent);
                CollisionComponent::CollisionResponse bToA = bComponent->GetResponseTo(aComponent);

                if (aToB == CollisionComponent::CollisionResponse::None && bToA == CollisionComponent::CollisionResponse::None)
                {
                    continue;
                }

                // �����x�[�X�Ō����`�F�b�N
                auto aShape = dynamic_cast<ShapeComponent*>(aComponent);
                auto bShape = dynamic_cast<ShapeComponent*>(bComponent);
                if (!aShape || !bShape)
                {// ���҂� shape �łȂ��Ɖ����o���ł��Ȃ�
                    continue;
                }

                //�@���҂��@kinematic�@�łȂ��Ɖ����o���ł��Ȃ�
                if (!aShape->IsKinematic() && !bShape->IsKinematic())
                {
                    continue; // PhysX ���m�͉����o���Ȃ�
                }



                // PxGeometry �擾
                auto aInfo = aShape->GetPhysicsShapeInfo();
                auto bInfo = bShape->GetPhysicsShapeInfo();

                PxVec3 dir;
                float depth = 0.0f;
                bool hit = PxGeometryQuery::computePenetration(
                    dir, depth,
                    aInfo.geometry.any(), aInfo.transform,
                    bInfo.geometry.any(), bInfo.transform);

                if (!hit || depth <= 0.0001f) // �����ȏՓ˂͖���
                    continue;

                std::pair<CollisionComponent*, CollisionComponent*> hitPairA = { aComponent, bComponent };
                std::pair<CollisionComponent*, CollisionComponent*> hitPairB = { bComponent, aComponent };

                // �Փ˒ʒm (Trigger or Block ) �ǂ����ɂ�
                aComponent->OnHit(hitPairA);
                bComponent->OnHit(hitPairB);


                // �Փ˃C�x���g(Actor �ʒm)
                aActor->BroadcastHit(hitPairA);
                bActor->BroadcastHit(hitPairB);

                // �Փ˃C�x���g(Actor �ʒm)
                aActor->OnHit(hitPairA);
                bActor->OnHit(hitPairB);



                // Block �łȂ���Ή����o���Ȃ�
                //if (aToB != CollisionComponent::CollisionResponse::Block && bToA != CollisionComponent::CollisionResponse::Block)
                if (aToB != CollisionComponent::CollisionResponse::Block || bToA != CollisionComponent::CollisionResponse::Block)
                {
                    //char msg[256];
                    //sprintf_s(msg, "skipPush: aToB=%d bToA=%d\n", (int)aToB, (int)bToA);
                    //OutputDebugStringA(msg);
                    continue;
                }

                //if (aComponent->IsStatic() && bComponent->IsStatic())
                //{
                //    continue; // �����ÓI�Ȃ疳��
                //}

                // ���ʂ��牟���o�������v�Z
                float massA = aShape->GetMass();
                float massB = bShape->GetMass();

                float rateA = 0.5f;
                float rateB = 0.5f;

                if (massA == 0.0f)
                {
                    rateA = 0.0f;
                    rateB = 1.0f;
                }
                else if (massB == 0.0f)
                {
                    rateA = 1.0f;
                    rateB = 0.0f;
                }
                else
                {
                    rateA = massB / (massA + massB);
                    rateB = 1.0f - rateA;
                }

                // �����o���x�N�g��
                float pushDistance = std::min<float>(depth, 1.0f); // �ő�[������
                dir = dir.getNormalized();


                // Actor�P�ʂɉ����o���ʂ����Z
                actorPushMap_[aActor].x += dir.x * pushDistance * rateA;
                //actorPushMap_[aActor].y += dir.y * pushDistance * rateA;
                actorPushMap_[aActor].z += dir.z * pushDistance * rateA;

                actorPushMap_[bActor].x -= dir.x * pushDistance * rateB;
                //actorPushMap_[bActor].y -= dir.y * pushDistance * rateB;
                actorPushMap_[bActor].z -= dir.z * pushDistance * rateB;

            }
        }
    }

    // ���o
    static void ApplyPushAll()
    {
        for (auto& [actor, pushVec] : actorPushMap_)
        {
            if (!actor) continue;

            auto rootComp = actor->GetRootComponent();
            if (rootComp)
            {
                rootComp->AddWorldOffset(pushVec);
            }
        }
    }

    static void RegisterCollisionComponent(std::shared_ptr<CollisionComponent> collisionComponent)
    {
        //allComponents_.emplace_back(collisionComponent->GetOwner(), collisionComponent);
        allComponents_.emplace_back(collisionComponent->GetActor(), collisionComponent);
    }

    static void UnRegisterCollisionComponent(CollisionComponent* collisionComponent)
    {
        allComponents_.erase(
            std::remove_if(
                allComponents_.begin(), allComponents_.end(),
                [collisionComponent](const std::pair<std::weak_ptr<Actor>, std::weak_ptr<CollisionComponent>>& p) {
                    return p.second.lock().get() == collisionComponent;
                }),
            allComponents_.end());
    }

    static void ClearAll()
    {
        allComponents_.clear();
    }

    static inline std::vector<std::pair<std::weak_ptr<Actor>, std::weak_ptr<CollisionComponent>>> allComponents_;
    //static inline std::vector<std::pair<Actor*, CollisionComponent*>> allComponents_;
private:
    // Actor ���̉��o�ʂ��v�Z���č�������p�� Map 
    static inline std::unordered_map<Actor*, DirectX::XMFLOAT3> actorPushMap_;
};
#endif //COLLISION_SYSTEM_H