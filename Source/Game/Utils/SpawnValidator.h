#ifndef SPAWN_VALIDATOR_H
#define SPAWN_VALIDATOR_H

#include <vector>
#include <string>

#include <DirectXMath.h>

#include "Physics/Collider.h"
#include "Core/Actor.h"
#include "Core/ActorManager.h"
//#include "Game/Actors/Player/Player.h"
#include "Components/CollisionShape/ShapeComponent.h"

struct Area
{
    std::string name;
    DirectX::XMFLOAT2 min; // エリアの左下座標
    DirectX::XMFLOAT2 max; // エリアの右上座標
    int maxItemCount = 1;   // このエリアの最大アイテム数
    int currentItemCount = 0; // 現在出現しているアイテム数
    float spawnTimer = 0.0f;

    int numCols = 0;
    int numRows = 0;
    int currentRow = 0;
    int currentCol = 0;

    float buildingSize = 1.6f;
    float spacing = 0.2f;

    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f }; // デフォルト色（透明青）

    void ComputeGridFromAreaSize()
    {
        float width = max.x - min.x;
        float depth = max.y - min.y;

        float cellSize = buildingSize + spacing;

        numCols = static_cast<int>(width / cellSize);
        numRows = static_cast<int>(depth / cellSize);

        maxItemCount = numCols * numRows;
        currentCol = 0;
        currentRow = 0;
    }

    DirectX::XMFLOAT3 GetGridSpawnPosition(int col, int row)
    {
        float width = max.x - min.x;
        float depth = max.y - min.y;
        float cellSize = buildingSize + spacing;

        float totalUsedWidth = numCols * cellSize;
        float totalUsedDepth = numRows * cellSize;

        float offsetX = (width - totalUsedWidth) * 0.5f;
        float offsetZ = (depth - totalUsedDepth) * 0.5f;

        float spawnX = min.x + offsetX + col * cellSize + cellSize * 0.5f;
        //float spawnZ = min.y + offsetZ + row * cellSize + cellSize * 0.5f;
        float spawnZ = min.y + offsetZ + (numRows - 1 - row) * cellSize + cellSize * 0.5f;

        return DirectX::XMFLOAT3(spawnX, 0.0f, spawnZ);
    }

    DirectX::XMFLOAT3 GetNextGridSpawnPosition()
    {
        if (numCols == 0 || numRows == 0)
        {
            ComputeGridFromAreaSize();
        }

        float width = max.x - min.x;
        float depth = max.y - min.y;
        float cellSize = buildingSize + spacing;

        float totalUsedWidth = numCols * cellSize;
        float totalUsedDepth = numRows * cellSize;

        float offsetX = (width - totalUsedWidth) * 0.5f;
        float offsetZ = (depth - totalUsedDepth) * 0.5f;

        float spawnX = min.x + offsetX + currentCol * cellSize + cellSize * 0.5f;
        float spawnZ = min.y + offsetZ + currentRow * cellSize + cellSize * 0.5f;

        // インデックス更新
        currentCol++;
        if (currentCol >= numCols)
        {
            currentCol = 0;
            currentRow++;
        }

        return DirectX::XMFLOAT3(spawnX, 0.0f, spawnZ);
    }

    void ResetGrid()
    {
        currentCol = 0;
        currentRow = 0;
    }
    bool Contains(const DirectX::XMFLOAT3& pos)const
    {
        // X-Z　平面
        return (pos.x >= min.x && pos.x <= max.x && pos.z >= min.y && pos.z <= max.y);
    }

    DirectX::XMFLOAT3 GetRandomSpawnPosition(float height = 0.175f)const
    {
        float x = MathHelper::RandomRange(min.x, max.x);
        float z = MathHelper::RandomRange(min.y, max.y);
        return { x,height,z };
    }
};

namespace std {
    template<>
    struct hash<AABB> {
        size_t operator()(const AABB& a) const noexcept {
            size_t h = 146527;
            auto mix = [&](float v) {
                h ^= std::hash<float>{}(v)+0x9e3779b9 + (h << 6) + (h >> 2);
                };
            mix(a.min.x); mix(a.min.y); mix(a.min.z);
            mix(a.max.x); mix(a.max.y); mix(a.max.z);
            return h;
        }
    };
}
class SpawnValidator
{
public:
    static void Register(AABB aabb)
    {
        //targets_.insert(actor);
        //if (auto shapeComponent = actor->GetComponent<ShapeComponent>())
        {
            //AABB aabb = shapeComponent->GetAABB();
            occupiedAABBs_.insert(aabb);
        }
    }

    //static void Unregister(AABB aabb)
    static void Unregister(std::shared_ptr<Actor> actor)
    {
        //targets_.erase(actor);
        if (auto shapeComponent = actor->GetComponent<ShapeComponent>())
        {
            AABB aabb = shapeComponent->GetAABB();
            occupiedAABBs_.erase(aabb);
        }
    }

    static const std::unordered_set<std::shared_ptr<Actor>>& GetTargets()
    {
        return targets_;
    }

    static const std::unordered_set<AABB>& GetAABBs()
    {
        return occupiedAABBs_;
    }

private:
    static inline std::unordered_set<std::shared_ptr<Actor>> targets_;
    static inline std::unordered_set<AABB> occupiedAABBs_;
public:
    //static void Clear()
    //{
    //    occupiedAABBs.clear();
    //}

    //static void RegisterAABB(const AABB& aabb)
    //{
    //    occupiedAABBs.emplace_back(aabb);
    //}

    //static void Unregister(const AABB& aabb)
    //{
    //    //std::erase_if(occupiedAABBs, [&](const AABB& o)
    //    //    {
    //    //        return o == aabb;
    //    //    });
    //}

    //// Actor から AABB を抽出して登録
    //static void RegisterFromActor(std::shared_ptr<Actor> actor)
    //{
    //    if (!actor)
    //    {
    //        return;
    //    }

    //    if (auto root = actor->GetRootComponent())
    //    {
    //        if (auto shape = std::dynamic_pointer_cast<MeshComponent>(root))
    //        {
    //            RegisterAABB(shape->model->GetAABB());
    //        }
    //    }
    //}

    // ここのエリアにものを置けるかどうか
    static bool IsAreaFree(const AABB& testAABB)
    {
        for (const auto& occupied : occupiedAABBs_)
        {
            if (ColliderAABBVsAABB(occupied, testAABB))
            {
                return false;
            }
        }
        return true;
    }

    static bool IsAreaFreePlayer(const AABB& testAABB)
    {
        //auto player = std::dynamic_pointer_cast<Player>(ActorManager::GetActorByName("actor"));
        //if (!player)
        //{
        //    _ASSERT("player is nullptr in BuildManager! ");
        //}
        //auto shapeLeft = std::dynamic_pointer_cast<BoxComponet>(player->GetSceneComponentByName("boxHitLeftComponent"));
        //auto shapeRight = std::dynamic_pointer_cast<BoxComponet>(player->GetSceneComponentByName("boxHitRightComponent"));
        //if (!shapeLeft && !shapeRight)
        //{
        //    _ASSERT("player left and right boxes is nullptr in BuildManager! ");
        //}
        //if (ColliderAABBVsAABB(shapeLeft->GetAABB(), testAABB) && ColliderAABBVsAABB(shapeRight->GetAABB(), testAABB))
        //{
        //    return false;
        //}
        return true;
    }

    static AABB TransformAABB(const AABB& localAABB, const DirectX::XMMATRIX& worldMatrix)
    {
        using namespace DirectX;
        XMFLOAT3 corners[8];
        localAABB.GetCorners(corners);


        AABB result;
        result.min = { FLT_MAX, FLT_MAX, FLT_MAX };
        result.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        for (int i = 0; i < 8; ++i)
        {
            XMVECTOR p = XMLoadFloat3(&corners[i]);
            p = XMVector3Transform(p, worldMatrix);
            XMFLOAT3 transformed;
            XMStoreFloat3(&transformed, p);

            result.min.x = std::min<float>(result.min.x, transformed.x);
            result.min.y = std::min<float>(result.min.y, transformed.y);
            result.min.z = std::min<float>(result.min.z, transformed.z);
            result.max.x = std::max<float>(result.max.x, transformed.x);
            result.max.y = std::max<float>(result.max.y, transformed.y);
            result.max.z = std::max<float>(result.max.z, transformed.z);
        }

        return result;
    }
};


//namespace SpawnHelper
//{
//    using PositionGenerator = std::function<DirectX::XMFLOAT3()>;
//
//    template<typename ActorT>
//    bool TrySpawnWithValidation(int maxTries, PositionGenerator generator, std::function<void(std::shared_ptr<ActorT>)> onSuccess)
//    {
//        for (int i = 0; i < maxTries; i++)
//        {
//            DirectX::XMFLOAT3 pos = generator();
//            auto actor = ActorManager::CreateAndRegisterActor<ActorT>("spawned", false);
//            actor->SetTempPosition(pos);
//            actor->Initialize();
//            actor->PostInitialize();
//
//            auto shape = actor->GetComponent<ShapeComponent>();
//            if (shape)
//            {
//                if (SpawnValidator::IsAreaFree(shape->GetAABB()))
//                {
//                    //SpawnValidator::RegisterAABB(shape->GetAABB());
//                    if (auto model = actor->skeltalMeshComponent)
//                    {
//                        model->SetIsVisible(true);
//                        if (onSuccess)
//                        {
//                            onSuccess(actor);
//                        }
//                        return true; //成功したのでこの area のループをやめる
//                    }
//                }
//            }
//            actor->SetValid(false);
//        }
//        return false;
//    }
//
//}

#endif //SPAWN_VALIDATOR_H