#ifndef STAGE_PROP_H
#define STAGE_PROP_H

#include <string>

#include "Core/Actor.h"

#include "Components/Transform/Transform.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Game/ItemSpawnerComponent.h"
#include "Components/Game/LifeTimeComponent.h"

#include "Game/Actors/Beam/Beam.h"
#include "Game/Utils/SpawnValidator.h"
//#include "Game/Actors/Stage/Building.h"

class StageProp :public Actor
{
public:
    //�����t���R���X�g���N�^
    StageProp(std::string actorName) :Actor(actorName) {}


    // �p���������T�u�N���X�̐�pGUI
    void DrawImGuiDetails() override
    {
#ifdef USE_IMGUI
        ImGui::DragFloat("prop mass", &mass_, 0.5f);
        ImGui::DragFloat3("boxHalfExtent_", &boxHalfExtent_.x, 0.05f);
#endif
    };

    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*//* std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) override
    {
        //if (auto build = dynamic_cast<Building*>(otherActor))
        //{
        //    box_->SetKinematic(false);
        //    box_->AddImpulse(impulse);
        //}
        //if (auto beam = dynamic_cast<Beam*>(otherActor))
        //{
        //    auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", mesh_);
        //    itemSpawner->SpawnItems(1, true);
        //}
    }

    // �����蔻��̃��W�b�N����������
    void AttachHitLogic()
    {
        AddHitCallback([&](std::pair<CollisionComponent*, CollisionComponent*> hitPair)
            {
                if (hitPair.first == box_.get())
                {// 
                    if (auto beam = std::dynamic_pointer_cast<Beam>(hitPair.second->GetActor()))
                    {
                        if (beam.get() == lastHitBeam_)
                        {
                            // �����r�[���Ƃ͊��ɓ������Ă���̂Ŗ���
                            return;
                        }

                        lastHitBeam_ = beam.get(); // �V�����r�[�����L�^
                        box_->SetKinematic(false);  // 
                        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", mesh_);
                        itemSpawner->SpawnItems(1, true);

                        // �A�C�e�����o����A3�b��ɍ폜����
                        //auto lifeTimeComponent =this ->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
                        auto lifeTimeComponent =this ->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
                        lifeTimeComponent->SetLifeTime(3.0f);

#if 0
                        DirectX::XMFLOAT3 pos = GetPosition();
                        pos.y += 0.5f;
                        // �r������A�C�e�����h���b�v����
                        Transform transform(pos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
                        auto buildDroppedItem = ActorManager::CreateAndRegisterActorWithTransform<PickUpItem>("buildDroppedItem", transform);
                        auto lifeTimeComponent = buildDroppedItem->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
                        lifeTimeComponent->SetLifeTime(3.0f);

                        float itemPower = beam->GetItemCount();
                        buildDroppedItem->skeltalMeshComponent->SetIsVisible(true); // ������悤�ɂ���

#endif // 0
                    }
                }
            });
    }

protected:
    std::shared_ptr<MeshComponent> mesh_;
    std::shared_ptr<BoxComponet> box_;
    Beam* lastHitBeam_ = nullptr;
    float mass_ = 10.0f;
    DirectX::XMFLOAT3 boxHalfExtent_ = { 0.5f,0.5f,0.5f };
};

// �����̔��@
class VendingMachineProp :public StageProp
{
public:
    //�����t���R���X�g���N�^
    VendingMachineProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // �`��p�R���|�[�l���g��ǉ�
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/vending_machine.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // �����蔻��R���|�[�l���g��ǉ�
        box_ = this->NewSceneComponent<class BoxComponet>("box", "model");
        box_->SetMass(100.0f);
        box_->SetHalfBoxExtent(boxHalfExtent_);
        box_->SetModelHeight(boxHalfExtent_.y);
        //box_->SetKinematic(false);
        box_->SetLayer(CollisionLayer::WorldProps);
        //box_->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::Trigger);
        box_->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        //box_->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //boxComponent->SetResponseToLayer(CollisionLayer::Item, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        box_->Initialize();
        box_->UpdateTransformImmediate();
        // �A�C�e�����N���Ȃ��悤�ɐݒ肷��
        //SpawnValidator::Register(shared_from_this());

        AttachHitLogic();
    }

};

// �M���@
class TrafficLightProp :public StageProp
{
public:
    //�����t���R���X�g���N�^
    TrafficLightProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // �`��p�R���|�[�l���g��ǉ�
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/traffic_light.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // �����蔻��R���|�[�l���g��ǉ�
        box_ = this->NewSceneComponent<class BoxComponet>("box", "model");
        box_->SetStatic(true);

        box_->SetHalfBoxExtent(boxHalfExtent_);
        box_->SetModelHeight(boxHalfExtent_.y);
        box_->SetLayer(CollisionLayer::WorldProps);
        box_->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::Trigger);
        box_->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        //box_->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //boxComponent->SetResponseToLayer(CollisionLayer::Item, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        box_->Initialize();
        box_->UpdateTransformImmediate();
        // �A�C�e�����N���Ȃ��悤�ɐݒ肷��
        //SpawnValidator::Register(shared_from_this());



        AttachHitLogic();
    }

};


// ���O�@
class OutDoorUnitProp :public StageProp
{
public:
    //�����t���R���X�g���N�^
    OutDoorUnitProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 110.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // �`��p�R���|�[�l���g��ǉ�
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/outdoor_unit.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // �����蔻��R���|�[�l���g��ǉ�
        box_ = this->NewSceneComponent<class BoxComponet>("box", "model");
        box_->SetMass(110.0f);
        box_->SetHalfBoxExtent(boxHalfExtent_);
        box_->SetModelHeight(boxHalfExtent_.y);
        box_->SetLayer(CollisionLayer::WorldProps);
        box_->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::Trigger);
        box_->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        //box_->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //boxComponent->SetResponseToLayer(CollisionLayer::Item, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        box_->Initialize();
        box_->UpdateTransformImmediate();
        // �A�C�e�����N���Ȃ��悤�ɐݒ肷��
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }

};

// �d�b�{�b�N�X
class PhoneBoothProp :public StageProp
{
public:
    //�����t���R���X�g���N�^
    PhoneBoothProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // �`��p�R���|�[�l���g��ǉ�
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/phone_booth.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // �����蔻��R���|�[�l���g��ǉ�
        box_ = this->NewSceneComponent<class BoxComponet>("box", "model");
        box_->SetStatic(true);
        box_->SetHalfBoxExtent(boxHalfExtent_);
        box_->SetModelHeight(boxHalfExtent_.y);
        box_->SetLayer(CollisionLayer::WorldProps);
        box_->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::Trigger);
        box_->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        //box_->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //boxComponent->SetResponseToLayer(CollisionLayer::Item, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        box_->Initialize();
        box_->UpdateTransformImmediate();
        // �A�C�e�����N���Ȃ��悤�ɐݒ肷��
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }

};

// �S�~��
class TrashCanProp :public StageProp
{
public:
    //�����t���R���X�g���N�^
    TrashCanProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // �`��p�R���|�[�l���g��ǉ�
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/trash_can.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // �����蔻��R���|�[�l���g��ǉ�
        box_ = this->NewSceneComponent<class BoxComponet>("box", "model");
        box_->SetStatic(true);
        box_->SetHalfBoxExtent(boxHalfExtent_);
        box_->SetModelHeight(boxHalfExtent_.y);
        box_->SetLayer(CollisionLayer::WorldProps);
        box_->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::Trigger);
        box_->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        //box_->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //boxComponent->SetResponseToLayer(CollisionLayer::Item, CollisionComponent::CollisionResponse::Block);
        box_->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        box_->Initialize();
        box_->UpdateTransformImmediate();
        // �A�C�e�����N���Ȃ��悤�ɐݒ肷��
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }
};


#endif // !STAGE_PROP_H
