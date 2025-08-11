#ifndef PHYSICS_UTILITY_H
#define PHYSICS_UTILITY_H

// C++ 標準ライブラリ
#include <string>

// 他ライブラリ
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>


// プロジェクトの他のヘッダ
#include "Physics/Physics.h"
#include "Components/CollisionShape/ShapeComponent.h"

class Actor;

struct RaycastHit
{
    Actor* actor = nullptr;     // 衝突した相手
    ShapeComponent* component = nullptr;    // 衝突したコンポーネント
    float distance = 0.0f;      // 距離
    DirectX::XMFLOAT3 hitPoint;     // ヒット距離
    DirectX::XMFLOAT3 normal;       // 法線
};

class PhysicsTest :
    public physx::PxQueryFilterCallback		// NOTE:③フィルタリングインターフェースの継承
    , public physx::PxSimulationEventCallback	// NOTE:⑦衝突イベントインターフェース継承
{
private:
    std::vector<DirectX::XMFLOAT3> points;
    PhysicsTest()
    {
        OutputDebugStringA("PhysicsTest constructor\n");
    }
    virtual ~PhysicsTest() = default;

public:
    static PhysicsTest& Instance()
    {
        static PhysicsTest instance;
        return instance;
    }

    void Finalize();

    void DebugShapePosition();

    bool RayCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, _Out_ RaycastHit& rayCastHit)
    {
        using namespace physx;

        points.clear();
        DirectX::XMFLOAT3 start = origin;
        end.x = origin.x + direction.x * distance;
        end.y = origin.y + direction.y * distance;
        end.z = origin.z + direction.z * distance;
        points.push_back(start);
        points.push_back(end);

        PxQueryFilterData filterData;
        filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
        filterData.data.word0 = 0xFFFFFFFF;  // 全レイヤーを対象にする（マスク全ビットON）
        filterData.data.word1 = 0xFFFFFFFF;

        PxRaycastBuffer raycastBuffer;

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

        // Raycastコールバックなしでシンプルに呼ぶ（フィルターコールバックを外す）
        bool hit = Physics::Instance().GetScene()->raycast(
            PxVec3(origin.x, origin.y, origin.z),
            PxVec3(direction.x, direction.y, direction.z),
            distance,
            raycastBuffer,
            hitFlags,
            filterData,
            nullptr);  // コールバックなし

        if (hit && raycastBuffer.hasBlock)
        {
            auto& block = raycastBuffer.block;

            rayCastHit.hitPoint = DirectX::XMFLOAT3(block.position.x, block.position.y, block.position.z);
            rayCastHit.normal = DirectX::XMFLOAT3(block.normal.x, block.normal.y, block.normal.z);
            rayCastHit.distance = block.distance;

            if (block.actor && block.actor->userData)
                rayCastHit.actor = static_cast<Actor*>(block.actor->userData);

            if (block.shape && block.shape->userData)
                rayCastHit.component = static_cast<ShapeComponent*>(block.shape->userData);

            return true;
        }

        return false;
    }

    // スフィアキャスト
    bool SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, _Out_ RaycastHit& rayCastHit, uint32_t myLayer, uint32_t wantHitRayer)
    {
        physx::PxQueryFilterData pxQueryFilterData(
            physx::PxQueryFlag::eDYNAMIC |
            physx::PxQueryFlag::eSTATIC |
            physx::PxQueryFlag::ePREFILTER |
            physx::PxQueryFlag::ePOSTFILTER
        );
        pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
        pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
        pxQueryFilterData.data.word0 = (myLayer);	// NOTE:⑤レイヤーマスク
        pxQueryFilterData.data.word1 = (wantHitRayer);	// NOTE:⑤レイヤーマスク

        //--------------------------
        // NOTE:④シェイプキャスト
        //--------------------------
        physx::PxSphereGeometry pxGeometry(radius);
        physx::PxSweepBuffer pxSweepBuffer;
        //physx::PxSweepBufferN<1> pxSweepBuffer;
        physx::PxTransform pxTransform(
            physx::PxVec3(origin.x, origin.y, origin.z),
            physx::PxQuat(0, 0, 0, 1));
        physx::PxHitFlags hitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;
        bool hit = Physics::Instance().GetScene()->sweep(pxGeometry,
            physx::PxTransform(origin.x, origin.y, origin.z),
            physx::PxVec3(direction.x, direction.y, direction.z),
            distance,
            pxSweepBuffer,
            //physx::PxHitFlag::eDEFAULT,
            hitFlags,
            pxQueryFilterData,
            this);
        if (hit && pxSweepBuffer.hasBlock)
        {
            const physx::PxVec3& p = pxSweepBuffer.block.position;
            //OutputDebugStringA(("Hit Position: " + std::to_string(p.x) + "," + std::to_string(p.y) + "," + std::to_string(p.z) + "\n").c_str());
            const physx::PxVec3& n = pxSweepBuffer.block.normal;

            rayCastHit.hitPoint = DirectX::XMFLOAT3(p.x, p.y, p.z);
            rayCastHit.normal = DirectX::XMFLOAT3(n.x, n.y, n.z);
            rayCastHit.distance = pxSweepBuffer.block.distance;
            if (pxSweepBuffer.block.actor && pxSweepBuffer.block.actor->userData)
                rayCastHit.actor = static_cast<Actor*>(pxSweepBuffer.block.actor->userData);

            if (pxSweepBuffer.block.shape && pxSweepBuffer.block.shape->userData)
                rayCastHit.component = static_cast<ShapeComponent*>(pxSweepBuffer.block.shape->userData);

            distance = rayCastHit.distance;
        }
        distance += radius;

        points.clear();
        DirectX::XMFLOAT3 start = origin;
        end.x = origin.x + direction.x * distance;
        end.y = origin.y + direction.y * distance;
        end.z = origin.z + direction.z * distance;
        points.push_back(start);
        points.push_back(end);

        //Capsule& capsule = capsules.emplace_back();
        //capsule.radius = radius;
        //capsule.height = (std::max)(distance - radius * 2, 0.0f);
        //capsule.color = line.color;

        if (direction.x == 0.0f && direction.y == 1.0f && direction.z == 0.0f)
        {
            DirectX::XMMATRIX Transform = DirectX::XMMatrixTranslation(
                origin.x + direction.x * distance * 0.5f,
                origin.y + direction.y * distance * 0.5f,
                origin.z + direction.z * distance * 0.5f);
            //DirectX::XMStoreFloat4x4(&capsule.transform, Transform);
        }
        else if (direction.x == 0.0f && direction.y == -1.0f && direction.z == 0.0f)
        {
            DirectX::XMMATRIX Transform = DirectX::XMMatrixTranslation(
                origin.x + direction.x * distance * 0.5f,
                origin.y + direction.y * distance * 0.5f,
                origin.z + direction.z * distance * 0.5f);
            //DirectX::XMStoreFloat4x4(&capsule.transform, Transform);
        }
        else
        {
            DirectX::XMVECTOR Origin = DirectX::XMLoadFloat3(&origin);
            DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&direction);
            DirectX::XMVECTOR Up = DirectX::XMVectorSet(0, 1, 0, 0);
            DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, Front);
            Up = DirectX::XMVector3Cross(Front, Right);
            DirectX::XMVECTOR Position = DirectX::XMVectorAdd(Origin, DirectX::XMVectorScale(Front, distance * 0.5f));
            Position = DirectX::XMVectorSetW(Position, 1.0f);
            DirectX::XMMATRIX Transform(Right, Up, Front, Position);
            DirectX::XMMATRIX Offset = DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2);
            //DirectX::XMStoreFloat4x4(&capsule.transform, Offset * Transform);
        }

        return hit;
    }


    void DebugRender(ID3D11DeviceContext* immediateContext)
    {
        ShapeRenderer::Instance().DrawSegment(immediateContext, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), points, ShapeRenderer::Type::Segment);

    }
    DirectX::XMFLOAT3 end{};

    physx::PxQueryHitType::Enum preFilter(
        const physx::PxFilterData& filterData,
        const physx::PxShape* shape,
        const physx::PxRigidActor* actor,
        physx::PxHitFlags& queryFlags)override
    {
        //OutputDebugStringA("=== preFilter CALLED ===\n");
        //return physx::PxQueryHitType::eBLOCK;
        const physx::PxFilterData& targetData = shape->getQueryFilterData();
        char msg[128];
        sprintf_s(msg, "Ray: word0=0x%X word1=0x%X | Shape: word0=0x%X word1=0x%X\n",
            filterData.word0, filterData.word1,
            targetData.word0, targetData.word1);
        //OutputDebugStringA(msg);

        // word0 = 自分のレイヤー、word1 = 当たりたいレイヤー
        if ((filterData.word0 & targetData.word1) == 0 ||
            (targetData.word0 & filterData.word1) == 0)
        {
            return physx::PxQueryHitType::eNONE; // 当たらない
        }

        return physx::PxQueryHitType::eBLOCK; // 当たる
    }
    // この下を overrideしないと 抽象クラスになる
        //--------------------------
    // NOTE:③フィルタリングインターフェース関数
    //--------------------------
    //physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override
    //{
    //    OutputDebugStringA("=== preFilter CALLED ===\n");
    //    physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
    //    physx::PxFilterData rayFilterData = filterData;

    //    char msg[256];
    //    sprintf_s(msg, "shape.word0=%u, ray.word0=%u, result=%s\n",
    //        shapeFilterData.word0, rayFilterData.word0,
    //        ((shapeFilterData.word0 & rayFilterData.word0) == 0 ? "NO HIT" : "HIT"));

    //    OutputDebugStringA(msg);

    //    return physx::PxQueryHitType::eTOUCH;

    //    //--------------------------
    //    // NOTE:③フィルタリング処理
    //    //--------------------------
    //    //physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
    //    //if ((shapeFilterData.word0 & filterData.word0) == 0)	// NOTE:⑥レイヤーマスク判定
    //    if ((shapeFilterData.word0 & filterData.word0) == 0)	// NOTE:⑥レイヤーマスク判定
    //    {
    //        return physx::PxQueryHitType::eNONE;
    //    }

    //    return physx::PxQueryHitType::eTOUCH;

    //}
    physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override
    {
        //--------------------------
        // NOTE:③フィルタリング処理
        //--------------------------
        return physx::PxQueryHitType::eBLOCK;
    }

    //--------------------------
    // NOTE:⑦衝突イベントインターフェース関数
    //--------------------------
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
    void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override {};
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {};
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override {}
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {};

};

#endif //PHYSICS_UTILITY_H
