#pragma once

#include <vector>
#include <set>
#include <map>
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>
#include <memory>

#include "Graphics/Renderer/ShapeRenderer.h"
#include "DefferdPhysicsOperation.h"
struct HitResult
{
    DirectX::XMFLOAT3	position;
    DirectX::XMFLOAT3	normal;
    float				distance;
};


class Actor;
class ShapeComponent;
struct RaycastHit2
{
    Actor* actor = nullptr;     // 衝突した相手
    ShapeComponent* component = nullptr;    // 衝突したコンポーネント
    float distance = 0.0f;      // 距離
    DirectX::XMFLOAT3 hitPoint;     // ヒット距離
    DirectX::XMFLOAT3 normal;       // 法線
};
// フィジクス
class Physics
    : public physx::PxQueryFilterCallback		// NOTE:③フィルタリングインターフェースの継承
    , public physx::PxSimulationEventCallback	// NOTE:⑦衝突イベントインターフェース継承
{
private:
    Physics() = default;
    ~Physics() = default;

public:
    // インスタンス取得
    static Physics& Instance()
    {
        static Physics instance;
        return instance;
    }

    // 初期化
    void Initialize();

    // 終了化
    void Finalize();

    // 更新処理
    void Update(float elapsedTime);

    // フィジクス取得
    physx::PxPhysics* GetPhysics() { return pxPhysics; }

    // 形状を作る関数
    physx::PxShape* CreateShape(const physx::PxGeometry& geometry) { return pxPhysics->createShape(geometry, *pxMaterial); }

    // シーン取得
    physx::PxScene* GetScene() { return pxScene; }

    // コントローラーマネージャー取得
    physx::PxControllerManager* GetControllerManager() { return pxControllerManager; }

    // マテリアル取得
    physx::PxMaterial* GetMaterial() { return pxMaterial; }

    // デフォルトのマテリアル取得
    physx::PxMaterial* GetDefaultMaterial()
    {
        physx::PxMaterial* defaultMaterial_ = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f); /* static friction, dynamic friction, restitution*/
        return defaultMaterial_;
    }

    // レイキャスト
    bool RayCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, HitResult& result);

    // スフィアキャスト
    bool SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, HitResult& result);
    bool SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, RaycastHit2& result);
    // simulate 後でする処理を追加する
    static void EnqueueDefferfOperations(const DefferdPhysicsOperation& op)
    {
        defferfOps_.push_back(op);
    }

    // sumilate 後に実行
    void ExecuteDefferdOperations();

protected:
    //--------------------------
    // NOTE:③フィルタリングインターフェース関数
    //--------------------------
    physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
    physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override;

    //--------------------------
    // NOTE:⑦衝突イベントインターフェース関数
    //--------------------------
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
    void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override {};
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {};

private:
    //--------------------------
    // NOTE:⑧衝突検出フィルタリング
    //--------------------------
    static physx::PxFilterFlags SimulationFilterShader(
        physx::PxFilterObjectAttributes	attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes	attributes1, physx::PxFilterData	filterData1,
        physx::PxPairFlags& pairFlags,
        const void* constantBlock, physx::PxU32 constantBlockSize);

    void PostSimulate()
    {
        for (auto* dyn : gravityEnableList_)
        {
            if (dyn && dyn->is<physx::PxRigidDynamic>())
            {
                dyn->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
            }
        }
        gravityEnableList_.clear();
    }
private:

    physx::PxDefaultAllocator			pxAllocator;
    physx::PxDefaultErrorCallback		pxErrorCallback;
    physx::PxFoundation* pxFoundation = nullptr;
    physx::PxPhysics* pxPhysics = nullptr;
    physx::PxDefaultCpuDispatcher* pxDispatcher = nullptr;
    physx::PxScene* pxScene = nullptr;
    physx::PxControllerManager* pxControllerManager = nullptr;

    physx::PxMaterial* pxMaterial = nullptr;

    physx::PxPvd* pxPvd = nullptr;

    struct ContactData
    {
        physx::PxVec3					normal;
        physx::PxF32					depth;
    };

    std::vector<physx::PxRigidDynamic*> gravityEnableList_;

    static inline std::vector<DefferdPhysicsOperation> defferfOps_;
};
