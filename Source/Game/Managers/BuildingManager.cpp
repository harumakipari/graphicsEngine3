#include "BuildingManager.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/Building.h"

void BuildingManager::SpawnItemArea(Area& area, int col, int row)
{
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto actorManager = currentScene->GetActorManager();  // ActorManager取得
#if 0
    SpawnHelper::TrySpawnWithValidation<TestCollision>(3,
        [&area]() {return area.GetRandomSpawnPosition(); }, [&area](auto building) {area.currentItemCount++; });
    const int maxTries = 10;
    int tryCount = 0;

    while (tryCount < maxTries)
    {
        ++tryCount;

        DirectX::XMFLOAT3 spawnPos = area.GetRandomSpawnPosition();
        // ここで position を取ってきたら生成できる

        Transform transform(spawnPos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        auto building = ActorManager::CreateAndRegisterActorWithTransform<Building>("building", transform);
        //building->SetTempPosition(spawnPos);
        //building->Initialize();
        //building->PostInitialize();

        auto shape = building->GetSceneComponentByName("boxComponent");
        if (auto box = std::dynamic_pointer_cast<BoxComponet>(shape))
        {
            if (SpawnValidator::IsAreaFree(box->GetAABB()))
            {
                SpawnValidator::Register(shared_from_this());

                //item->GetComponent<SkeltalMeshComponent>()->SetIsVisible(true);
                building->preSkeltalMeshComponent->SetIsVisible(true);
                area.currentItemCount++;
                return; //成功したのでこの area のループをやめる
            }
        }
        building->SetValid(false);
    }
#else
    DirectX::XMFLOAT3 spawnPos = area.GetGridSpawnPosition(col, row);


    //std::shared_ptr<InstancedStaticMeshComponent> instanceBuildMeshComponent=

    auto player = std::dynamic_pointer_cast<Player>(actorManager->GetActorByName("actor"));
    DirectX::XMFLOAT3 pos = player->GetPosition();
    DirectX::XMVECTOR disVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&spawnPos));
    float x = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(disVec));
    static float r = 4.0f;
    if ((x - r * r) <= 0.1f)
    {
        return;
    }

    spawnPos.y = -5.0f;
    Transform transform(spawnPos, DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT3(1, 1, 1));



    auto building = actorManager->CreateAndRegisterActorWithTransform<Building>("building", transform);

    //auto shape = building->GetSceneComponentByName("boxComponent");
    //if (auto box = std::dynamic_pointer_cast<BoxComponet>(shape))
    //{
    //    if (SpawnValidator::IsAreaFree(box->GetAABB()))
    //    {
    //        SpawnValidator::RegisterAABB(box->GetAABB());
    //        building->preSkeltalMeshComponent->SetIsVisible(true);
    //        area.currentItemCount++;
    //        return;
    //    }
    //}

    //building->SetValid(false);
#endif
}

void BuildingManager::SpawnBossBuildArea(Area& area, int col, int row)
{
    DirectX::XMFLOAT3 spawnPos = area.GetGridSpawnPosition(col, row);
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto actorManager = currentScene->GetActorManager();  // ActorManager取得

    //std::shared_ptr<InstancedStaticMeshComponent> instanceBuildMeshComponent=

    auto player = std::dynamic_pointer_cast<Player>(actorManager->GetActorByName("actor"));
    DirectX::XMFLOAT3 pos = player->GetPosition();
    DirectX::XMVECTOR disVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&spawnPos));
    float x = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(disVec));
    static float r = 4.0f;
    if ((x - r * r) <= 0.1f)
    {
        return;
    }

    spawnPos.y = -5.0f;
    Transform transform(spawnPos, DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT3(1, 1, 1));

    auto building = actorManager->CreateAndRegisterActorWithTransform<BossBuilding>("bossbuilding", transform);

    //auto shape = building->GetSceneComponentByName("boxComponent");
    //if (auto box = std::dynamic_pointer_cast<BoxComponet>(shape))
    //{
    //    if (SpawnValidator::IsAreaFree(box->GetAABB()))
    //    {
    //        SpawnValidator::RegisterAABB(box->GetAABB());
    //        building->preSkeltalMeshComponent->SetIsVisible(true);
    //        area.currentItemCount++;
    //        return;
    //    }
    //}

    //building->SetValid(false);
}

