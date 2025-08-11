#include "PickUpItem.h"
#include "Game/Utils/SpawnValidator.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Managers/ItemManager.h"

void PickUpItem::Initialize(const Transform& transform)
{
    // 描画用コンポーネントを追加
    skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
    skeltalMeshComponent->SetModel("./Data/Models/Items/PickUpEnergyCore/pick_up_item.gltf");
    //skeltalMeshComponent->model->isModelInMeters = false;
    //skeltalMeshComponent->SetIsVisible(false); // アイテム生成時に一フレーム描画されてしまうから
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    skeltalMeshComponent->model->emission = 15.0f;
    skeltalMeshComponent->SetIsCastShadow(false);

    SetPosition(transform.GetLocation());
    SetQuaternionRotation(transform.GetRotation());
    SetScale(transform.GetScale());

    // 当たり判定のコンポーネントを追加
    sphereComponent = this->NewSceneComponent<class SphereComponent>("sphereComponent", "skeltalComponent");
    sphereComponent->SetRadius(0.4f);
    sphereComponent->SetMass(40.0f);
    sphereComponent->SetModelHeight(0.4f);
    sphereComponent->SetLayer(CollisionLayer::PickUpItem);
    sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
    //sphereComponent->SetKinematic(false);
    sphereComponent->Initialize();
    //sphereComponent->SetIsVisibleDebugBox(false);
    //sphereComponent->SetIsVisibleDebugShape(false);
    SpawnValidator::Register(sphereComponent->GetAABB());

};

void PickUpItem::Initialize()
{
    // 描画用コンポーネントを追加
    skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
    skeltalMeshComponent->SetModel("./Data/Models/Items/PickUpEnergyCore/pick_up_item.gltf");
    skeltalMeshComponent->SetIsVisible(false); // アイテム生成時に一フレーム描画されてしまうから
    skeltalMeshComponent->SetIsCastShadow(false);
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    skeltalMeshComponent->model->emission = 15.0f;
    SetPosition(tempPosition);    // こっちを使うよーーー

    // 当たり判定のコンポーネントを追加
    sphereComponent = this->NewSceneComponent<class SphereComponent>("sphereComponent", "skeltalComponent");
    sphereComponent->SetRadius(0.4f);
    sphereComponent->SetMass(40.0f);
    sphereComponent->SetModelHeight(0.4f);
    sphereComponent->SetLayer(CollisionLayer::PickUpItem);
    sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
    //sphereComponent->SetKinematic(false);
    sphereComponent->Initialize();
    //sphereComponent->SetIsVisibleDebugBox(false);
    //sphereComponent->SetIsVisibleDebugShape(false);

    SpawnValidator::Register(sphereComponent->GetAABB());
};

void PickUpItem::Finalize()
{
    SpawnValidator::Unregister(shared_from_this());
}

//更新処理
void PickUpItem::Update(float deltaTime)
{
    // アイテムとプレイヤーの当たり判定が無効になるのを避けるため
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.y = 0.3f;
    SetPosition(pos);

    if (itemType == Type::FromProps)
    {// 湧くアイテムだったら
        if (const auto& lifeTimeComponent = GetComponent<LifeTimeComponent>())
        {
            // 残り秒数が返ってくる
            float remainingTimer = lifeTimeComponent->GetRemainingTime();

            // 点滅までの秒数
            const float blinkThreashold = 4.0f;
            // 点滅間隔秒数
            const float blinkInterval = 0.005f;

            static bool visible = false;

            if (remainingTimer <= blinkThreashold)
            {
                blinkTimer += deltaTime;
                if (blinkTimer >= blinkInterval)
                {
                    visible = !visible;
                    blinkTimer = 0.0f;
                    skeltalMeshComponent->SetIsVisible(visible);
                }
            }
            else
            {
                skeltalMeshComponent->SetIsVisible(true);
                blinkTimer = 0.0f;
                visible = true;
            }

            //char b[256];
            //sprintf_s(b, "Item:LifeTime:%f isVisble:%s\n", remainingTimer, visible ? "true" : "false");
            //OutputDebugStringA(b);
        }
    }

    // 歯車境界
    if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(GetOwnerScene()->GetActorManager()->GetActorByName("enemy")))
    {
        if (enemy->IsEnemyJumping())
        {
            DirectX::XMFLOAT3 gearPos = enemy->GetJumpPosition();
            // ギアの半径
            float radius = 2.0f;

            // 現在位置とギア中心のXZ差分
            float dx = pos.x - gearPos.x;
            float dz = pos.z - gearPos.z;

            float distSq = dx * dx + dz * dz;
            float radiusSq = radius * radius;

            if (distSq < radiusSq)
            {
                SetPendingDestroy();
                // ステージ上に湧かせるアイテムの max の値を決める
                if (auto itemManager = GameManager::GetItemManager())
                {
                    if (GetType() == PickUpItem::Type::RandomSpawn)
                    {// ランダムスポーンのアイテムを拾ったら　エリアに通知する
                        const auto& pos = GetPosition();
                        itemManager->DecreaseAreaItemCount(pos);
                    }
                }
            }
        }
    };


#if 0
    if (isOnGround)
    {// 地面に接地したら
        sphereComponent->SetGravity(false);
        sphereComponent->SetKinematic(true);
    }

#endif // 0
}
