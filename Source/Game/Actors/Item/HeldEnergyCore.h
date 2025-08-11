#ifndef HELD_ENERGY_CORE_H
#define HELD_ENERGY_CORE_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

// プレイヤーが所持するアイテムクラス
class HeldEnergyCore :public Actor
{
public:
    //引数付きコンストラクタ
    HeldEnergyCore(std::string modelName) :Actor(modelName) {}

    virtual ~HeldEnergyCore() = default;

    void Initialize()override
    {
        // 親を外から先に設定しておく
        if (!rootComponent_)
        {
            _ASSERT("親のコンポーネントが設定されていません。");
        }
        // 描画用コンポーネントを追加
        std::shared_ptr<SkeltalMeshComponent>skeltalMeshComponent = this->NewSceneComponentWithParent<class SkeltalMeshComponent>("skeltalComponent", rootComponent_ ? rootComponent_ : nullptr);
        skeltalMeshComponent->SetModel("./Data/Models/Items/HeldEnergyCore/heldEnergyCore.gltf");
        //skeltalMeshComponent->model->isModelInMeters = false;
        SetScale(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
        //skeltalMeshComponent->SetIsVisible(false); // アイテム生成時に一フレーム描画されてしまうから
        skeltalMeshComponent->SetRelativeLocationDirect(tempPosition); // player の横に生成する position
        //// 当たり判定のコンポーネントを追加
        //std::shared_ptr<SphereComponent> sphereComponent = this->NewComponent<class SphereComponent>("sphereComponent", rootComponent_ ? rootComponent_ : nullptr);
        //sphereComponent->SetRadius(0.1f);
        //sphereComponent->SetModelHeight(0.1f);
        //sphereComponent->SetMass(40.0f);
        //sphereComponent->SetRelativeLocationDirect(tempPosition); // player の横に生成する position
        //sphereComponent->SetLayer(CollisionLayer::HeldItem);    // player の横に所持するアイテム
        //// 押し出し判定するもの
        //sphereComponent->SetResponseToLayer(CollisionLayer::HeldItem, CollisionComponent::CollisionResponse::None); // 地面
        //sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::None); // 地面
        //sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block); // 地面
        //sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block); // 瓦礫
        //sphereComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);  // ビル
        //sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);  // ビル
        //sphereComponent->Initialize();


    }

    // 更新処理
    void Update(float deltaTime)override {}

    float radius = 0.5f;
};

#endif //HELD_ENERGY_CORE_H