#include "ItemSpawnerComponent.h"

#include "Core/ActorManager.h"
#include "Core/Actor.h"
#include "Game/Actors/Item/PickUpItem.h"

#include "Components/Transform/Transform.h"
#include "Components/Game/LifeTimeComponent.h"

#include "Math/MathHelper.h"
#include "Engine/Scene/Scene.h"
void ItemSpawnerComponent::SpawnItems(int count/*, float beamPower = 1.0f*/, bool hasLifeTime, float itemLifeTimer)
{
    const float spawnRadius = 1.0f;
    for (int i = 0; i < count; i++)
    {
        float angle = MathHelper::RandomRange(0.0f, DirectX::XM_2PI);
        float radius = spawnRadius * std::sqrt(MathHelper::RandomRange(0.0f, 1.0f));
        float offsetX = radius * std::cosf(angle);
        float offsetZ = radius * std::sinf(angle);

        DirectX::XMFLOAT3 center = GetOwner()->GetPosition();
        center.y += 0.175f;

        DirectX::XMFLOAT3 spawnPos = {
            center.x + offsetX,
            0.175f,
            center.z + offsetZ
        };
        //DirectX::XMFLOAT3 spawnPos = GetOwner()->GetPosition();
        //spawnPos.y += 0.175f; // Optional offset
        Transform transform(DirectX::XMFLOAT3(spawnPos), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        Scene* currentScene = Scene::GetCurrentScene();
        if (!currentScene)
        {
            return;
        }
        auto item = currentScene->GetActorManager()->CreateAndRegisterActorWithTransform<PickUpItem>("spawnItemFrom", transform);
        item->SetType(PickUpItem::Type::FromProps);

        if (hasLifeTime)
        {// 寿命付きアイテムだったら
            auto lifeTimeComponent = item->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
            //auto lifeTimeComponent = item->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
            lifeTimeComponent->SetLifeTime(itemLifeTimer);
        }
    }
}
