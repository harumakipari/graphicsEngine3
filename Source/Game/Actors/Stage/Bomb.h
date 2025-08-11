#ifndef BOMB_H
#define BOMB_H


#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Engine/Scene/Scene.h"

#include "Physics/Physics.h"
#include "Physics/DefferdPhysicsOperation.h"

#include "Components/Effect/EffectComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Game/LifeTimeComponent.h"
#include "Components/Game/EraseInAreaComponent.h"
#include "Components/Transform/Transform.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Components/Audio/AudioSourceComponent.h"
class Bomb :public Actor
{
public:
    Bomb(std::string modelName) :Actor(modelName)
    {
    }

    std::shared_ptr<EffectComponent> effectExplosionComponent;

    DirectX::XMFLOAT3 GetProjectionPosition()const
    {
        DirectX::XMFLOAT3 pos = GetPosition();
        return { pos.x,0.0f,pos.z };
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    //std::shared_ptr<AudioSourceComponent> audioComponent;
    std::shared_ptr<AudioSourceComponent> targetAudioComponent;
    void Initialize(const Transform& transform)override
    {
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Models/Stage/Bomb/bomb.gltf");
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;

        if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(GetOwnerScene()->GetActorManager()->GetActorByName("enemy")))
        {
            DirectX::XMFLOAT4X4 enemyTr = enemy->GetWorldTransform();
            DirectX::XMFLOAT3 pos;
            auto enemyModel = enemy->skeltalMeshComponent->model;
            if (enemyModel)
            {
                pos = enemyModel->GetJointWorldPosition("head_end_FK", enemyModel->GetNodes(), enemyTr);
            }
            SetPosition(pos);

            // ���e�̐�`�ɐ���
            DirectX::XMFLOAT3 enemyForward = enemy->GetForward();
            DirectX::XMVECTOR baseDirVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&enemyForward));

            float angleMin = -15.0f;
            float angleMax = +15.0f;

            float angleStep = 0.0f;
            if (fanTotal > 1)
                angleStep = (angleMax - angleMin) / (fanTotal - 1);

            float angleDeg = angleMin + fanIndex * angleStep;
            float angleRad = DirectX::XMConvertToRadians(angleDeg);

            // Y������� baseDirVec ����]�i���ɓW�J�j
            DirectX::XMVECTOR upAxis = XMVectorSet(0, 1, 0, 0);
            DirectX::XMVECTOR rotatedDir = XMVector3Rotate(
                baseDirVec,
                XMQuaternionRotationAxis(upAxis, angleRad)
            );

            // �ŏI�I�ȏ㏸�����i��{������j
            DirectX::XMVECTOR upOffset = XMVectorSet(0, 1.0f, 0, 0);
            DirectX::XMVECTOR mixed = XMVectorAdd(upOffset, rotatedDir);
            upDirection = {};
            DirectX::XMStoreFloat3(&upDirection, XMVector3Normalize(mixed));

        }
        DirectX::XMFLOAT4 rot;
        DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1.0f, 0.0f, 0.0f }, DirectX::XMConvertToRadians(180.0f));
        DirectX::XMStoreFloat4(&rot, rotQuat);
        SetQuaternionRotation(rot);
        //SetQuaternionRotation(transform.GetRotation());
        SetScale(DirectX::XMFLOAT3(1.2f, 1.2f, 1.2f));

        target = transform.GetLocation();
        target.y += 5.0f;
        targetRot = transform.GetRotation();

        // ���e�{�̂̓����蔻��
        float radius = 0.3f;
        sphereComponent = this->NewSceneComponent<class EraseInAreaComponent>("sphereComponent", "skeltalComponent");
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(40.0f);
        sphereComponent->SetModelHeight(radius * 0.5f);
        sphereComponent->SetLayer(CollisionLayer::Bomb);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::None);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
        sphereComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::None);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
        sphereComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);
        //sphereComponent->SetKinematic(false);
        sphereComponent->Initialize();
        //sphereComponent->SetGravity(false);
        sphereComponent->SetIsVisibleDebugBox(false);
        sphereComponent->SetIsVisibleDebugShape(false);

        eraseInAreaComponent = this->NewSceneComponent<class EraseInAreaComponent>("eraseInAreaComponent", "skeltalComponent");
        eraseInAreaComponent->SetRadius(1.5f);
        eraseInAreaComponent->SetModelHeight(1.5f * 0.5f);
        eraseInAreaComponent->SetMass(40.0f);
        eraseInAreaComponent->SetLayer(CollisionLayer::EraseInArea);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
        eraseInAreaComponent->Initialize();
        eraseInAreaComponent->DisableCollision();
        eraseInAreaComponent->SetIsVisibleDebugBox(false);
        eraseInAreaComponent->SetIsVisibleDebugShape(false);

        // �G�t�F�N�g�R���|�[�l���g��ǉ�
        effectExplosionComponent = this->NewSceneComponent<class EffectComponent>("effectExplosionComponet", "skeltalComponent");

        // �I�[�f�B�I�R���|�[�l���g��ǉ�
        //audioComponent = this->NewSceneComponent<AudioSourceComponent>("explosionSoundComponent", "skeltalComponent");
        //audioComponent->SetSource(L"./Data/Sound/SE/missile_explosion.wav");
        targetAudioComponent = this->NewSceneComponent<AudioSourceComponent>("targetSoundComponent", "skeltalComponent");
        targetAudioComponent->SetSource(L"./Data/Sound/SE/target.wav");

        AddHitCallback([&](std::pair<CollisionComponent*, CollisionComponent*> hitPair)
            {
                //OutputDebugStringA(hitPair.second->GetActor()->GetName().c_str());

                if (auto stage = std::dynamic_pointer_cast<Stage>(hitPair.second->GetActor()))
                {// �X�e�[�W�Ɠ���������
                    isOnGround = true;
                    //audioComponent->Play();
                }
                else if (auto build = std::dynamic_pointer_cast<Building>(hitPair.second->GetActor()))
                {
                    isOnGround = true;
                    //audioComponent->Play();
                }
                else if (auto build = std::dynamic_pointer_cast<BossBuilding>(hitPair.second->GetActor()))
                {
                    isOnGround = true;
                    //audioComponent->Play();
                }
            }
        );

    }

    // ���e�̏㏸�p�x��ݒ�
    void SetFanIndex(int index, int total)
    {
        fanIndex = index;
        fanTotal = total;
    }

    std::shared_ptr<EraseInAreaComponent> eraseInAreaComponent;
    std::shared_ptr<SphereComponent> sphereComponent;
    void Update(float deltaTime)override
    {
        if (isGoingUp)
        {// �㏸���Ȃ�
            DirectX::XMFLOAT3 pos = GetPosition();
            // �㏸�iY���̂݁j
            if (pos.y < target.y)
            {
                float speed = 10.0f; // �㏸���x
                pos.y += speed * deltaTime;

                if (pos.y > target.y)
                {
                    pos.y = target.y;
                }

                SetPosition(pos);
            }
            else
            {
                pos.x = target.x;
                pos.z = target.z;
                SetPosition(pos);

                isGoingUp = false;

                //�^�[�Q�b�g�o���^�C�~���O�ŉ��Đ�
                targetAudioComponent->Play();
            }
        }
        else if (!isOnGround)
        {
            DirectX::XMFLOAT3 pos = GetPosition();
            pos.y -= 5.0f * deltaTime;
            SetPosition(pos);
            SetQuaternionRotation(targetRot);
        }
        if (isOnGround && !hasExploded)
        {// �n�ʂɐڒn������
            sphereComponent->SetGravity(false);
            sphereComponent->SetKinematic(true);
            eraseInAreaComponent->EnableCollision();
            hasExploded = true;

            // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
            effectExplosionComponent->SetWorldLocationDirect(GetPosition());
            effectExplosionComponent->SetEffectPower(1.0f);
            //effectExplosionComponent->SetEffectImpulse(impulse);
            //effectExplosionComponent->SetEffectNormal(normal);
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();


            skeltalMeshComponent->SetIsVisible(false);
            // �n�ʂɕt�����炱�������

            //���j��
            Audio::PlayOneShot(L"./Data/Sound/SE/missile_explosion.wav");

            //auto lifeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeComponent");
            auto lifeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeComponent");
            lifeComponent->SetLifeTime(0.1f);
            
            //auto delayComponent = this->NewLogicComponent<TimerActionComponent>("timerComponent");
            //delayComponent->SetTimer(0.5f, [&]()
            //    {
            //        this->ScheduleDestroyComponentByName("eraseInAreaComponent");
            //        this->ScheduleDestroyComponentByName("sphereComponent");
            //    });
        }
        else
        {
            //eraseInAreaComponent->DisableCollision();
        }
    }

    // �㏸�����ǂ���
    bool IsGoingUp() const { return isGoingUp; }

    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override
    {
    }


    void NotifyHit(CollisionComponent* selfComp, CollisionComponent* otherComp, Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
        int a = 0;
    }


    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
        //char buf[256];
        //sprintf_s(buf, "actor=%s\n", otherActor->GetName().c_str());
        //OutputDebugStringA(buf);
        //if (auto stage = dynamic_cast<Stage*>(otherActor))
        //{
        //    isOnGround = true;
        //}
        //else if (auto build = dynamic_cast<Building*>(otherActor))
        //{
        //    isOnGround = true;
        //}
    }

private:
    bool hasExploded = false;
    bool isOnGround = false;
    bool isGoingUp = true;  // �㏸�����ǂ���
    DirectX::XMFLOAT3 target = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 targetRot = { 0.0f,0.0f,0.0f,1.0f };
    int fanIndex = 0;
    int fanTotal = 1;
    DirectX::XMFLOAT3 upDirection = { 0.0f,0.0f,0.0f };
};

#endif // !BOMB_H
