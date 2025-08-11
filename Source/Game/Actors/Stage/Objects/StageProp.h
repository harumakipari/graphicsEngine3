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
    //引数付きコンストラクタ
    StageProp(std::string actorName) :Actor(actorName) {}


    // 継承押したサブクラスの専用GUI
    void DrawImGuiDetails() override
    {
#ifdef USE_IMGUI
        ImGui::DragFloat("prop mass", &mass_, 0.5f);
        ImGui::DragFloat3("boxHalfExtent_", &boxHalfExtent_.x, 0.05f);
#endif
    };

    //　collisionComponent　が Dynamic の物と当たった時に通る
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

    // 当たり判定のロジックをくっつける
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
                            // 同じビームとは既に当たっているので無視
                            return;
                        }

                        lastHitBeam_ = beam.get(); // 新しいビームを記録
                        box_->SetKinematic(false);  // 
                        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", mesh_);
                        itemSpawner->SpawnItems(1, true);

                        // アイテムが出たら、3秒後に削除する
                        //auto lifeTimeComponent =this ->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
                        auto lifeTimeComponent =this ->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
                        lifeTimeComponent->SetLifeTime(3.0f);

#if 0
                        DirectX::XMFLOAT3 pos = GetPosition();
                        pos.y += 0.5f;
                        // ビルからアイテムをドロップする
                        Transform transform(pos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
                        auto buildDroppedItem = ActorManager::CreateAndRegisterActorWithTransform<PickUpItem>("buildDroppedItem", transform);
                        auto lifeTimeComponent = buildDroppedItem->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
                        lifeTimeComponent->SetLifeTime(3.0f);

                        float itemPower = beam->GetItemCount();
                        buildDroppedItem->skeltalMeshComponent->SetIsVisible(true); // 見えるようにする

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

// 自動販売機
class VendingMachineProp :public StageProp
{
public:
    //引数付きコンストラクタ
    VendingMachineProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // 描画用コンポーネントを追加
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/vending_machine.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 当たり判定コンポーネントを追加
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
        // アイテムが湧かないように設定する
        //SpawnValidator::Register(shared_from_this());

        AttachHitLogic();
    }

};

// 信号機
class TrafficLightProp :public StageProp
{
public:
    //引数付きコンストラクタ
    TrafficLightProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // 描画用コンポーネントを追加
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/traffic_light.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 当たり判定コンポーネントを追加
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
        // アイテムが湧かないように設定する
        //SpawnValidator::Register(shared_from_this());



        AttachHitLogic();
    }

};


// 室外機
class OutDoorUnitProp :public StageProp
{
public:
    //引数付きコンストラクタ
    OutDoorUnitProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 110.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // 描画用コンポーネントを追加
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/outdoor_unit.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 当たり判定コンポーネントを追加
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
        // アイテムが湧かないように設定する
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }

};

// 電話ボックス
class PhoneBoothProp :public StageProp
{
public:
    //引数付きコンストラクタ
    PhoneBoothProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // 描画用コンポーネントを追加
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/phone_booth.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 当たり判定コンポーネントを追加
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
        // アイテムが湧かないように設定する
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }

};

// ゴミ箱
class TrashCanProp :public StageProp
{
public:
    //引数付きコンストラクタ
    TrashCanProp(std::string actorName) :StageProp(actorName)
    {
        mass_ = 10.0f;
        boxHalfExtent_ = { 0.5f,0.5f,0.5f };
    }

    void Initialize(const Transform& transform) override
    {
        // 描画用コンポーネントを追加
        mesh_ = this->NewSceneComponent<class SkeltalMeshComponent>("model");
        mesh_->SetModel("./Data/Models/Stage/Props/trash_can.gltf");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 当たり判定コンポーネントを追加
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
        // アイテムが湧かないように設定する
        //SpawnValidator::Register(shared_from_this());


        AttachHitLogic();
    }
};


#endif // !STAGE_PROP_H
