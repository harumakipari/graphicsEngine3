#include <algorithm>
#include "Engine/Utility/Win32Utils.h"
#include "Physics.h"
#include "CollisionEvent.h"
#include "Graphics/Core/Graphics.h"

#include "Core/Actor.h"

#include "Components/CollisionShape/CollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"


// 初期化
void Physics::Initialize()
{
    // 基盤生成
    {
        pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
        _ASSERT_EXPR(pxFoundation != nullptr, "Failed PxCreateFoundation");
    }
    // PVD
    {
        pxPvd = physx::PxCreatePvd(*pxFoundation);
        _ASSERT_EXPR(pxPvd != nullptr, "Failed PxCreatePvd");

        physx::PxPvdTransport* pxPvdTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        _ASSERT_EXPR(pxPvdTransport != nullptr, "Failed PxDefaultPvdSocketTransportCreate");

        pxPvd->connect(*pxPvdTransport, physx::PxPvdInstrumentationFlag::eALL);
    }
    // 物理システム生成
    {
        pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), true, pxPvd);
        _ASSERT_EXPR(pxPhysics != nullptr, "Failed PxCreatePhysics");

        PxInitExtensions(*pxPhysics, pxPvd);
    }
    // ディスパッチャー生成
    {
        pxDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        _ASSERT_EXPR(pxDispatcher != nullptr, "Failed PxDefaultCpuDispatcherCreate");
    }
    // シーン生成
    {
        physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
        pxSceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        pxSceneDesc.cpuDispatcher = pxDispatcher;
        pxSceneDesc.filterShader = SimulationFilterShader;	// NOTE:⑧衝突検出フィルタリング
        pxSceneDesc.simulationEventCallback = this;
        //pxSceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
        //pxSceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;   // ActiveActors を送る
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_VISUALIZATION;   // 可視化そのものを有効
                //pxSceneDesc.visualizationScale = 1.0f;

        pxScene = pxPhysics->createScene(pxSceneDesc);
        _ASSERT_EXPR(pxScene != nullptr, "Failed pxPhysics->createScene");
    }

    // PVDシーンクライアント設定
    {
        physx::PxPvdSceneClient* pxPvdSceneClient = pxScene->getScenePvdClient();
        if (pxPvdSceneClient != nullptr)
        {
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    // コントローラーマネージャー生成
    {
        pxControllerManager = PxCreateControllerManager(*pxScene);
        _ASSERT_EXPR(pxControllerManager != nullptr, "Failed PxCreateControllerManager");
        pxControllerManager->setDebugRenderingFlags(physx::PxControllerDebugRenderFlag::eALL);
    }

    // マテリアル生成
    {
        pxMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        _ASSERT_EXPR(pxMaterial != nullptr, "Failed pxPhysics->createMaterial");
    }
}

// 終了化
void Physics::Finalize()
{
    PxCloseExtensions();

    PX_RELEASE(pxControllerManager);
    PX_RELEASE(pxScene);
    PX_RELEASE(pxDispatcher);
    PX_RELEASE(pxPhysics);

    if (pxPvd != nullptr)
    {
        physx::PxPvdTransport* pxPvdTransport = pxPvd->getTransport();
        pxPvdTransport->disconnect();
        PX_RELEASE(pxPvd);
        PX_RELEASE(pxPvdTransport);
    }

    PX_RELEASE(pxFoundation);
}

// sumilate 後に実行
void Physics::ExecuteDefferdOperations()
{
    for (auto& op : defferfOps_)
    {
        switch (op.type_)
        {
        case DefferdPhysicsOperation::Type::DisableCollision:
            op.target_->DisableCollision();
            break;
        case DefferdPhysicsOperation::Type::AddScene:
            op.target_->AddToScene();
            break;
        case DefferdPhysicsOperation::Type::SetKinematicFalse:
            op.target_->SetKinematic(false);
            break;
        case DefferdPhysicsOperation::Type::SetActive:
            op.target_->SetActive(true);
            break;
        case DefferdPhysicsOperation::Type::DestroyComponent:
            op.target_->GetActor()->ScheduleDestroyComponentByName(op.target_->name());
            break;
        case DefferdPhysicsOperation::Type::RemoveRigidActor:
            if (op.actor_)
            {
                op.actor_->release(); // ここで安全に削除される
            }
            break;
        default:
            break;
        }
    }
    defferfOps_.clear();
}

// 更新処理
void Physics::Update(float elapsedTime)
{
    //static bool frag = true;
    //// 物理シミュレーション処理
    //if (GetAsyncKeyState('N') & 0x8000)
    //{
    //    frag = !frag;
    //}
    //if (frag)
    //{

#if 1

    pxScene->simulate(elapsedTime);//simulate今から開始するよっていう合図
    pxScene->fetchResults(true);//	計算が終わるまで待つ
    PostSimulate();

#endif // 0

    //ExecuteDefferdOperations();
    //--------------------------
    // NOTE:⑬キネマティックオブジェクト同士の衝突処理
    //--------------------------
#if 0
    {
        // シーンの中からダイナミックオブジェクトを収集
        physx::PxActorTypeFlags pxActorTypeFlags = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
        physx::PxU32 pxNumActors = pxScene->getNbActors(pxActorTypeFlags);

        if (pxNumActors > 0)
        {
            std::vector<physx::PxRigidActor*> pxActors(pxNumActors);
            pxScene->getActors(pxActorTypeFlags, reinterpret_cast<physx::PxActor**>(pxActors.data()), pxNumActors);

            // 収集したアクターを総当たりで衝突処理
            physx::PxShape* pxShapesA[128] = { nullptr };
            physx::PxShape* pxShapesB[128] = { nullptr };
            for (physx::PxU32 pxActorIndexA = 0; pxActorIndexA < pxNumActors; ++pxActorIndexA)
            {
                // キネマティックオブジェクト以外は対象外
                physx::PxRigidActor* pxActorA = pxActors.at(pxActorIndexA);
                physx::PxRigidDynamic* pxRigidBodyA = pxActorA->is<physx::PxRigidDynamic>();


                //physx::PxTransform pxPose = pxActorA->getGlobalPose();
                //OutputDebugStringA(("PhysX Actor Pos: " + std::to_string(pxPose.p.x) + ", " + std::to_string(pxPose.p.y) + ", " + std::to_string(pxPose.p.z) + "\n").c_str());


                if (!pxRigidBodyA->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
                {//キネマティックじゃなかったらcontinue
                    continue;
                }

                // アクターにくっついている形状を収集
                const physx::PxU32 pxNumShapesA = pxActorA->getShapes(pxShapesA, _countof(pxShapesA));
                for (physx::PxU32 pxShapeIndexA = 0; pxShapeIndexA < pxNumShapesA; ++pxShapeIndexA)
                {
                    // トリガー設定の形状は対象外
                    physx::PxShape* pxShapeA = pxShapesA[pxShapeIndexA];
                    if (pxShapeA->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
                    {//トリガーオブジェクトは押し出し処理いらないからcontinue
                        continue;
                    }
                    // 三角形メッシュなどの複雑なジオメトリは対象外
                    const physx::PxGeometry& pxGeometryA = pxShapeA->getGeometry();
                    if (pxGeometryA.getType() == physx::PxGeometryType::eTRIANGLEMESH)
                    {//sphereとcapseleとboxしか想定してないからtrinagleMeshは対象外めっちゃ処理重いconvexはできる
                        continue;
                    }

                    // アクターと形状の情報から姿勢を計算
                    //カプセルなどが世界のどこにいるか
                    physx::PxTransform pxShapeTransformA = physx::PxShapeExt::getGlobalPose(*pxShapeA, *pxActorA);

                    // 対象となるアクターと衝突判定をする
                    for (physx::PxU32 pxActorIndexB = pxActorIndexA + 1; pxActorIndexB < pxNumActors; ++pxActorIndexB)
                    {//forが２重になるから,+しておく
                        // キネマティックオブジェクト以外は対象外
                        physx::PxRigidActor* pxActorB = pxActors.at(pxActorIndexB);
                        physx::PxRigidDynamic* pxRigidBodyB = pxActorB->is<physx::PxRigidDynamic>();
                        if (!pxRigidBodyB->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
                        {
                            continue;
                        }
                        // 両方のアクターの質量がゼロの場合は衝突処理をしない
                        physx::PxF32 pxMassA = pxRigidBodyA->getMass();
                        physx::PxF32 pxMassB = pxRigidBodyB->getMass();
                        if (pxMassA == 0.0f && pxMassB == 0.0f)
                        {//質量0はcontinueこの先の0除算を防ぐため
                            continue;
                        }

                        // 対象となるアクターにくっついている形状を収集
                        const physx::PxU32 pxNumShapesB = pxActorB->getShapes(pxShapesB, _countof(pxShapesB));
                        for (physx::PxU32 pxShapeIndexB = 0; pxShapeIndexB < pxNumShapesB; ++pxShapeIndexB)
                        {
                            // トリガー設定の形状は対象外
                            physx::PxShape* pxShapeB = pxShapesB[pxShapeIndexB];
                            if (pxShapeB->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
                            {
                                continue;
                            }
                            // 三角形メッシュなどの複雑なジオメトリは対象外
                            const physx::PxGeometry& pxGeometryB = pxShapeB->getGeometry();
                            if (pxGeometryB.getType() == physx::PxGeometryType::eTRIANGLEMESH)
                            {
                                continue;
                            }

                            // アクターと形状の情報から姿勢を計算
                            physx::PxTransform pxShapeTransformB = physx::PxShapeExt::getGlobalPose(*pxShapeB, *pxActorB);

                            // 衝突押し出し処理
                            {
                                // ２つの形状が重なっている場合、押し出し方向とめり込み量をの計算をする
                                physx::PxVec3 pxDirection;
                                physx::PxF32 pxDepth;
                                bool intersect = physx::PxGeometryQuery::computePenetration(pxDirection, pxDepth, pxGeometryA, pxShapeTransformA, pxGeometryB, pxShapeTransformB);
                                if (intersect)											//どっち方向にどれくらいめり込んでいるか
                                {
                                    // ２つのアクターの質量から押し出し具合を計算する		2つのmassから押し出しの割合を求める
                                    physx::PxF32 pxRateA, pxRateB;
                                    if (pxMassA == 0.0f)
                                    {
                                        pxRateA = 0.0f;
                                        pxRateB = 1.0f;
                                    }
                                    else if (pxMassB == 0.0f)
                                    {
                                        pxRateA = 1.0f;
                                        pxRateB = 0.0f;
                                    }
                                    else
                                    {
                                        pxRateA = pxMassB / (pxMassA + pxMassB);
                                        pxRateB = 1.0f - pxRateA;
                                    }
                                    // 各アクターに対して押し出し量を計算
                                    physx::PxF32 pxDepthA = pxDepth * pxRateA;
                                    physx::PxF32 pxDepthB = pxDepth * pxRateB;

                                    // 押し出し処理
                                    physx::PxTransform pxTransformA = pxRigidBodyA->getGlobalPose();
                                    physx::PxTransform pxTransformB = pxRigidBodyB->getGlobalPose();

                                    pxTransformA.p += pxDirection * pxDepthA;
                                    pxTransformB.p -= pxDirection * pxDepthB;

                                    pxRigidBodyA->setKinematicTarget(pxTransformA);
                                    pxRigidBodyA->setGlobalPose(pxTransformA);//すぐにその場所に変わる　瞬間移動　本来はsimulateの時に内部的に行われるけど、
                                    //今回はsimulateの直後に行っているから他に影響がない

                                    pxRigidBodyB->setKinematicTarget(pxTransformB);
                                    pxRigidBodyB->setGlobalPose(pxTransformB);

                                    auto actorA = static_cast<Actor*>(pxRigidBodyA->userData);
                                    auto actorB = static_cast<Actor*>(pxRigidBodyB->userData);
                                    if (actorA)
                                    {
                                        Transform tA = PhysicsHelper::FromPxTransform(pxTransformA);
                                        actorA->rootComponent_->SetPhysicalTransform(tA); // 押し出された Transform を保存
                                        //actorA->SetPosition(tA.GetLocation());
                                        //actorA->SetQuaternionRotation(tA.GetRotation());
                                    }
                                    if (actorB)
                                    {
                                        Transform tB = PhysicsHelper::FromPxTransform(pxTransformB);
                                        actorB->rootComponent_->SetPhysicalTransform(tB);
                                        //actorB->SetPosition(tB.GetLocation());
                                        //actorB->SetQuaternionRotation(tB.GetRotation());
                                    }
                                    //// 衝突通知
                                    //auto objA = static_cast<CollisionEvent*>(pxActorA->userData);
                                    //if (objA != nullptr)
                                    //{
                                    //    CollisionHit hit;
                                    //    hit.pxActor = pxActorB;
                                    //    hit.pxNormal = pxDirection;
                                    //    hit.pxDepth = pxDepth;
                                    //    objA->OnCollisionHit(hit);
                                    //}
                                    //auto objB = static_cast<CollisionEvent*>(pxActorB->userData);
                                    //if (objB != nullptr)
                                    //{
                                    //    CollisionHit hit;
                                    //    hit.pxActor = pxActorA;
                                    //    hit.pxNormal = -pxDirection;
                                    //    hit.pxDepth = pxDepth;
                                    //    objB->OnCollisionHit(hit);
                                    //}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

// レイキャスト
bool Physics::RayCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, HitResult& result)
{
    //--------------------------
    // NOTE:②フィルタリング設定
    //--------------------------
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
    //OutputDebugStringA(("Ray Origin: " + std::to_string(origin.x) + ", " + std::to_string(origin.y) + ", " + std::to_string(origin.z) + "\n").c_str());
    //OutputDebugStringA(("Ray Dir: " + std::to_string(direction.x) + ", " + std::to_string(direction.y) + ", " + std::to_string(direction.z) + "\n").c_str());

    //--------------------------
    // NOTE:①レイキャスト
    //--------------------------
    physx::PxVec3 pxOrigin(origin.x, origin.y, origin.z);
    physx::PxVec3 pxDirection(direction.x, direction.y, direction.z);
    physx::PxRaycastBuffer/*N<1>*/ pxRaycastBuffer;
    bool hit = pxScene->raycast(
        pxOrigin, pxDirection, distance,
        pxRaycastBuffer,
        physx::PxHitFlag::eDEFAULT,
        pxQueryFilterData,		// NOTE:②フィルタリング設定
        &Physics::Instance());
    if (hit && pxRaycastBuffer.hasBlock)
    {
        //--------------------------
        // NOTE:④ヒット情報
        //--------------------------
        const physx::PxVec3& p = pxRaycastBuffer.block.position;
        const physx::PxVec3& n = pxRaycastBuffer.block.normal;
        result.position = DirectX::XMFLOAT3(p.x, p.y, p.z);
        result.normal = DirectX::XMFLOAT3(n.x, n.y, n.z);
        result.distance = pxRaycastBuffer.block.distance;

        distance = result.distance;
    }

    return hit;
}

// スフィアキャスト
bool Physics::SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, HitResult& result)
{
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク

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
    bool hit = pxScene->sweep(pxGeometry,
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

        result.position = DirectX::XMFLOAT3(p.x, p.y, p.z);
        result.normal = DirectX::XMFLOAT3(n.x, n.y, n.z);
        result.distance = pxSweepBuffer.block.distance;

        distance = result.distance;
    }
    distance += radius;

    return hit;
}

// スフィアキャスト
bool Physics::SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, RaycastHit2& result)
{
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:⑤レイヤーマスク

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
    bool hit = pxScene->sweep(pxGeometry,
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

        result.hitPoint = DirectX::XMFLOAT3(p.x, p.y, p.z);
        result.normal = DirectX::XMFLOAT3(n.x, n.y, n.z);
        result.distance = pxSweepBuffer.block.distance;
        if (pxSweepBuffer.block.actor && pxSweepBuffer.block.actor->userData)
            result.actor = static_cast<Actor*>(pxSweepBuffer.block.actor->userData);

        if (pxSweepBuffer.block.shape && pxSweepBuffer.block.shape->userData)
            result.component = static_cast<ShapeComponent*>(pxSweepBuffer.block.shape->userData);

        distance = result.distance;
    }
    distance += radius;

    return hit;
}

physx::PxQueryHitType::Enum Physics::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
    //OutputDebugStringA("=== preFilter CALLED ===\n");

    //--------------------------
    // NOTE:③フィルタリング処理
    //--------------------------
    physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
    if ((shapeFilterData.word0 & filterData.word0) == 0)	// NOTE:⑥レイヤーマスク判定
    {
        return physx::PxQueryHitType::eNONE;
    }

    return physx::PxQueryHitType::eTOUCH;
}

physx::PxQueryHitType::Enum Physics::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor)
{
    //--------------------------
    // NOTE:③フィルタリング処理
    //--------------------------
    return physx::PxQueryHitType::eBLOCK;
}

// 衝突している どちらかが Dynamic（PxRigidDynamic）
// 両方とも Simulation Shape（＝eSIMULATION_SHAPEフラグが有効な PxShape）
// 両方とも Trigger ではない（＝Trigger 同士 or Trigger 片方だと onTrigger() の対象）
// kinematic と static 同士は呼ばれない　今回の場合 player と stage は呼ばれない
void Physics::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    using namespace physx;
    //--------------------------
    // 衝突イベント検出
    //--------------------------
    for (physx::PxU32 i = 0; i < nbPairs; i++)
    {
        const physx::PxContactPair& pair = pairs[i];

        if (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0 || pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1)
        {
            continue;	// 削除された Shape と衝突は無視
        }

        physx::PxShape* shapeA = pair.shapes[0];
        physx::PxShape* shapeB = pair.shapes[1];
        if (!shapeA || !shapeB)
        {
            continue;
        }

        // userDataを使って、自分のCollisionComponentを取得
        CollisionComponent* compA = static_cast<CollisionComponent*>(shapeA->userData);
        CollisionComponent* compB = static_cast<CollisionComponent*>(shapeB->userData);
        //if (!compA || !compB)
        //{
        //    continue;
        //}


        // ここで衝突した２つの Actor を取得
        physx::PxActor* pxActorA = pairHeader.actors[0];
        physx::PxActor* pxActorB = pairHeader.actors[1];
        //physx::PxActor* pxActorA = pair.shapes[0]->getActor();
        //physx::PxActor* pxActorB = pair.shapes[1]->getActor();
        if (!pxActorA || !pxActorB)
        {
            continue;
        }
        // userDataを使って、自分のActorを取得
        Actor* ownerA = static_cast<Actor*>(pxActorA->userData);
        Actor* ownerB = static_cast<Actor*>(pxActorB->userData);

        // 衝突点の取得
        PxContactPairPoint contactPoints[16];
        PxU32 contactCount = pair.extractContacts(contactPoints, 16);

        for (PxU32 j = 0; j < contactCount; j++)
        {
            const PxContactPairPoint& point = contactPoints[j];

            // 衝突位置
            PxVec3 hitPos = point.position;

            // 衝突法線 B から A へ向く
            PxVec3 normal = point.normal;

            // A が受け取った 衝突力 
            PxVec3 impulse = point.impulse;

            // 衝突の深さ
            float separate = point.separation;


            //if (compA && compB)
            //{
            //    compA->OnCollisionEnter(compB/*,*/
            //       /* DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z),
            //        DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
            //        DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z)*/);
            //    compB->OnCollisionEnter(compA/*,
            //        DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z),
            //        DirectX::XMFLOAT3(-normal.x, -normal.y, -normal.z),
            //        DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z)*/);
            //}
            if (ownerA)
            {
                if (compA && compB)
                {
                    //compA->OnCollisionEnter(compB,
                    //    DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z),
                    //    DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
                    //    DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z));

                    ownerA->NotifyHit(compA, compB, ownerB, DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z));
                }
            }
            if (ownerB)
            {
                if (compA && compB)
                {
                    ownerB->NotifyHit(compB, compA, ownerA, DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z), DirectX::XMFLOAT3(-normal.x, -normal.y, -normal.z), DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z));
                }
            }

            if (ownerA)// 基本こっちでdeleteしている
            {
                ownerA->NotifyHit(ownerB, DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z));
            }
            if (ownerB)
            {
                ownerB->NotifyHit(ownerA, DirectX::XMFLOAT3(hitPos.x, hitPos.y, hitPos.z), DirectX::XMFLOAT3(-normal.x, -normal.y, -normal.z), DirectX::XMFLOAT3(impulse.x, impulse.y, impulse.z));
            }
            //std::string msg = ownerA->GetName() + " hit " + ownerB->GetName() + "\n";
            //OutputDebugStringA(msg.c_str());

            //char buffer[256];
            //sprintf_s(buffer, sizeof(buffer), "HitPos: (%.2f, %.2f, %.2f)\n", hitPos.x, hitPos.y, hitPos.z);
            //OutputDebugStringA(buffer);

            //sprintf_s(buffer, sizeof(buffer), "Impulse: (%.2f, %.2f, %.2f)\n", impulse.x, impulse.y, impulse.z);
            //OutputDebugStringA(buffer);
        }

        // pxActorA 
        //if (auto* dynamicA = pxActorA->is<physx::PxRigidDynamic>())
        //{
        //    if (dynamicA->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY)
        //    {
        //        gravityEnableList_.push_back(dynamicA);
        //        //dynamicA->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
        //        //dynamicA->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
        //    }
        //}
        //// pxActorB 
        //if (auto* dynamicB = pxActorB->is<physx::PxRigidDynamic>())
        //{
        //    if (dynamicB->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY)
        //    {
        //        gravityEnableList_.push_back(dynamicB);
        //        //dynamicB->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
        //    }
        //}

        physx::PxContactPairPoint pxContactPoints[32];
        physx::PxU32 pxContactCount = pair.extractContacts(pxContactPoints, _countof(pxContactPoints));

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            // 初めて接触したとき
            //for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            //{
            //	const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

            //	auto a = static_cast<CollisionEvent*>(pxActorA->userData);
            //	if (a != nullptr)
            //	{
            //		// NOTE:⑫衝突イベント通知
            //		CollisionContact contact;
            //		contact.pxActor = pxActorB;
            //		contact.pxNormal = pxContactPoint.normal;
            //		contact.pxPoint = pxContactPoint.position;
            //		contact.pxDepth = pxContactPoint.separation;
            //		a->OnContactTouchFound(contact);
            //	}
            //	auto b = static_cast<CollisionEvent*>(pxActorB->userData);
            //	if (b != nullptr)
            //	{
            //		// NOTE:⑫衝突イベント通知
            //		CollisionContact contact;
            //		contact.pxActor = pxActorA;
            //		contact.pxNormal = -pxContactPoint.normal;
            //		contact.pxPoint = pxContactPoint.position;
            //		contact.pxDepth = pxContactPoint.separation;
            //		b->OnContactTouchFound(contact);
            //	}
            //}
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            // 接触している状態
            //for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            //{
            //	const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

            //	auto a = static_cast<CollisionEvent*>(pxActorA->userData);
            //	if (a != nullptr)
            //	{
            //		// NOTE:⑫衝突イベント通知
            //		CollisionContact contact;
            //		contact.pxActor = pxActorB;
            //		contact.pxNormal = pxContactPoint.normal;
            //		contact.pxPoint = pxContactPoint.position;
            //		contact.pxDepth = pxContactPoint.separation;
            //		a->OnContactTouchPersists(contact);
            //	}
            //	auto b = static_cast<CollisionEvent*>(pxActorB->userData);
            //	if (b != nullptr)
            //	{
            //		// NOTE:⑫衝突イベント通知
            //		CollisionContact contact;
            //		contact.pxActor = pxActorA;
            //		contact.pxNormal = -pxContactPoint.normal;
            //		contact.pxPoint = pxContactPoint.position;
            //		contact.pxDepth = pxContactPoint.separation;
            //		b->OnContactTouchPersists(contact);
            //	}
            //}
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            // 接触している状態から離れたとき
            for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            {
                const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

                auto a = static_cast<CollisionEvent*>(pxActorA->userData);
                if (a != nullptr)
                {
                    // NOTE:⑫衝突イベント通知
                    CollisionContact contact;
                    contact.pxActor = pxActorB;
                    contact.pxNormal = pxContactPoint.normal;
                    contact.pxPoint = pxContactPoint.position;
                    contact.pxDepth = pxContactPoint.separation;
                    a->OnContactTouchLost(contact);
                }
                auto b = static_cast<CollisionEvent*>(pxActorB->userData);
                if (b != nullptr)
                {
                    // NOTE:⑫衝突イベント通知
                    CollisionContact contact;
                    contact.pxActor = pxActorA;
                    contact.pxNormal = -pxContactPoint.normal;
                    contact.pxPoint = pxContactPoint.position;
                    contact.pxDepth = pxContactPoint.separation;
                    b->OnContactTouchLost(contact);
                }
            }
        }
    }
}

void Physics::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
    //--------------------------
    // NOTE:⑨トリガーイベント検出
    //--------------------------
    for (physx::PxU32 i = 0; i < count; i++)
    {
        const physx::PxTriggerPair& pair = pairs[i];

        // 削除された Shape の処理は無視
        if (pair.flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
            continue;

        // userData から自分の CollisionComponent を取得
        CollisionComponent* triggerComp = static_cast<CollisionComponent*>(pair.triggerShape->userData);
        CollisionComponent* otherComp = static_cast<CollisionComponent*>(pair.otherShape->userData);

        if (!triggerComp || !otherComp)
            continue;

        //// Trigger の Enter / Exit を判定
        //if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        //{
        //    triggerComp->OnCollisionEnter(otherComp);
        //}
        //else if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        //{
        //    //triggerComp->OnTriggerExit(otherComp);
        //}
#if 0

        if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            // 初めて接触したとき						//TODO:02ここでのuserDataはcollisionEventのはずだから
            auto a = static_cast<CollisionEvent*>(pair.triggerActor->userData);
            if (a != nullptr)
            {
                // NOTE:⑫衝突イベント通知
                CollisionTrigger trigger;
                trigger.pxActor = pair.otherActor;
                a->OnTriggerTouchFound(trigger);//OnTriggerEnter
            }
            auto b = static_cast<CollisionEvent*>(pair.otherActor->userData);
            if (b != nullptr)
            {
                // NOTE:⑫衝突イベント通知
                CollisionTrigger trigger;
                trigger.pxActor = pair.triggerActor;
                b->OnTriggerTouchFound(trigger);
            }
        }
        if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            // 接触している状態から離れたとき
            auto a = static_cast<CollisionEvent*>(pair.triggerActor->userData);
            if (a != nullptr)
            {
                // NOTE:⑫衝突イベント通知
                CollisionTrigger trigger;
                trigger.pxActor = pair.otherActor;
                a->OnTriggerTouchLost(trigger);
            }
            auto b = static_cast<CollisionEvent*>(pair.otherActor->userData);
            if (b != nullptr)
            {
                // NOTE:⑫衝突イベント通知
                CollisionTrigger trigger;
                trigger.pxActor = pair.triggerActor;
                b->OnTriggerTouchLost(trigger);
            }
        }

#endif // 0
    }
}

//--------------------------
// NOTE:⑧衝突検出フィルタリング
//--------------------------
physx::PxFilterFlags Physics::SimulationFilterShader(
    physx::PxFilterObjectAttributes	attributes0, physx::PxFilterData filterData0,
    physx::PxFilterObjectAttributes	attributes1, physx::PxFilterData	filterData1,
    physx::PxPairFlags& pairFlags,
    const void* constantBlock, physx::PxU32 constantBlockSize)
{

    if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
    {
        pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
        return physx::PxFilterFlag::eDEFAULT;
    }
    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
    pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    return physx::PxFilterFlag::eDEFAULT;
    // レイヤーマスクを使用して衝突フィルタリングする
    if ((filterData0.word1 & filterData1.word0) == 0 || (filterData1.word1 & filterData0.word0) == 0)
    {
        return physx::PxFilterFlag::eSUPPRESS;
    }

    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
    pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST
        | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
    return physx::PxFilterFlag::eNOTIFY;

    //pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
    //pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    //return physx::PxFilterFlag::eDEFAULT;
}