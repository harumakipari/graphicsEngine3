#include <algorithm>
#include "Engine/Utility/Win32Utils.h"
#include "Physics.h"
#include "CollisionEvent.h"
#include "Graphics/Core/Graphics.h"

#include "Core/Actor.h"

#include "Components/CollisionShape/CollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"


// ������
void Physics::Initialize()
{
    // ��Ր���
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
    // �����V�X�e������
    {
        pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), true, pxPvd);
        _ASSERT_EXPR(pxPhysics != nullptr, "Failed PxCreatePhysics");

        PxInitExtensions(*pxPhysics, pxPvd);
    }
    // �f�B�X�p�b�`���[����
    {
        pxDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        _ASSERT_EXPR(pxDispatcher != nullptr, "Failed PxDefaultCpuDispatcherCreate");
    }
    // �V�[������
    {
        physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
        pxSceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        pxSceneDesc.cpuDispatcher = pxDispatcher;
        pxSceneDesc.filterShader = SimulationFilterShader;	// NOTE:�G�Փˌ��o�t�B���^�����O
        pxSceneDesc.simulationEventCallback = this;
        //pxSceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
        //pxSceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;   // ActiveActors �𑗂�
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
        //pxSceneDesc.flags |= physx::PxSceneFlag::eENABLE_VISUALIZATION;   // �������̂��̂�L��
                //pxSceneDesc.visualizationScale = 1.0f;

        pxScene = pxPhysics->createScene(pxSceneDesc);
        _ASSERT_EXPR(pxScene != nullptr, "Failed pxPhysics->createScene");
    }

    // PVD�V�[���N���C�A���g�ݒ�
    {
        physx::PxPvdSceneClient* pxPvdSceneClient = pxScene->getScenePvdClient();
        if (pxPvdSceneClient != nullptr)
        {
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pxPvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    // �R���g���[���[�}�l�[�W���[����
    {
        pxControllerManager = PxCreateControllerManager(*pxScene);
        _ASSERT_EXPR(pxControllerManager != nullptr, "Failed PxCreateControllerManager");
        pxControllerManager->setDebugRenderingFlags(physx::PxControllerDebugRenderFlag::eALL);
    }

    // �}�e���A������
    {
        pxMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        _ASSERT_EXPR(pxMaterial != nullptr, "Failed pxPhysics->createMaterial");
    }
}

// �I����
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

// sumilate ��Ɏ��s
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
                op.actor_->release(); // �����ň��S�ɍ폜�����
            }
            break;
        default:
            break;
        }
    }
    defferfOps_.clear();
}

// �X�V����
void Physics::Update(float elapsedTime)
{
    //static bool frag = true;
    //// �����V�~�����[�V��������
    //if (GetAsyncKeyState('N') & 0x8000)
    //{
    //    frag = !frag;
    //}
    //if (frag)
    //{

#if 1

    pxScene->simulate(elapsedTime);//simulate������J�n�������Ă������}
    pxScene->fetchResults(true);//	�v�Z���I���܂ő҂�
    PostSimulate();

#endif // 0

    //ExecuteDefferdOperations();
    //--------------------------
    // NOTE:�L�L�l�}�e�B�b�N�I�u�W�F�N�g���m�̏Փˏ���
    //--------------------------
#if 0
    {
        // �V�[���̒�����_�C�i�~�b�N�I�u�W�F�N�g�����W
        physx::PxActorTypeFlags pxActorTypeFlags = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
        physx::PxU32 pxNumActors = pxScene->getNbActors(pxActorTypeFlags);

        if (pxNumActors > 0)
        {
            std::vector<physx::PxRigidActor*> pxActors(pxNumActors);
            pxScene->getActors(pxActorTypeFlags, reinterpret_cast<physx::PxActor**>(pxActors.data()), pxNumActors);

            // ���W�����A�N�^�[�𑍓�����ŏՓˏ���
            physx::PxShape* pxShapesA[128] = { nullptr };
            physx::PxShape* pxShapesB[128] = { nullptr };
            for (physx::PxU32 pxActorIndexA = 0; pxActorIndexA < pxNumActors; ++pxActorIndexA)
            {
                // �L�l�}�e�B�b�N�I�u�W�F�N�g�ȊO�͑ΏۊO
                physx::PxRigidActor* pxActorA = pxActors.at(pxActorIndexA);
                physx::PxRigidDynamic* pxRigidBodyA = pxActorA->is<physx::PxRigidDynamic>();


                //physx::PxTransform pxPose = pxActorA->getGlobalPose();
                //OutputDebugStringA(("PhysX Actor Pos: " + std::to_string(pxPose.p.x) + ", " + std::to_string(pxPose.p.y) + ", " + std::to_string(pxPose.p.z) + "\n").c_str());


                if (!pxRigidBodyA->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
                {//�L�l�}�e�B�b�N����Ȃ�������continue
                    continue;
                }

                // �A�N�^�[�ɂ������Ă���`������W
                const physx::PxU32 pxNumShapesA = pxActorA->getShapes(pxShapesA, _countof(pxShapesA));
                for (physx::PxU32 pxShapeIndexA = 0; pxShapeIndexA < pxNumShapesA; ++pxShapeIndexA)
                {
                    // �g���K�[�ݒ�̌`��͑ΏۊO
                    physx::PxShape* pxShapeA = pxShapesA[pxShapeIndexA];
                    if (pxShapeA->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
                    {//�g���K�[�I�u�W�F�N�g�͉����o����������Ȃ�����continue
                        continue;
                    }
                    // �O�p�`���b�V���Ȃǂ̕��G�ȃW�I���g���͑ΏۊO
                    const physx::PxGeometry& pxGeometryA = pxShapeA->getGeometry();
                    if (pxGeometryA.getType() == physx::PxGeometryType::eTRIANGLEMESH)
                    {//sphere��capsele��box�����z�肵�ĂȂ�����trinagleMesh�͑ΏۊO�߂����Ꮘ���d��convex�͂ł���
                        continue;
                    }

                    // �A�N�^�[�ƌ`��̏�񂩂�p�����v�Z
                    //�J�v�Z���Ȃǂ����E�̂ǂ��ɂ��邩
                    physx::PxTransform pxShapeTransformA = physx::PxShapeExt::getGlobalPose(*pxShapeA, *pxActorA);

                    // �ΏۂƂȂ�A�N�^�[�ƏՓ˔��������
                    for (physx::PxU32 pxActorIndexB = pxActorIndexA + 1; pxActorIndexB < pxNumActors; ++pxActorIndexB)
                    {//for���Q�d�ɂȂ邩��,+���Ă���
                        // �L�l�}�e�B�b�N�I�u�W�F�N�g�ȊO�͑ΏۊO
                        physx::PxRigidActor* pxActorB = pxActors.at(pxActorIndexB);
                        physx::PxRigidDynamic* pxRigidBodyB = pxActorB->is<physx::PxRigidDynamic>();
                        if (!pxRigidBodyB->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
                        {
                            continue;
                        }
                        // �����̃A�N�^�[�̎��ʂ��[���̏ꍇ�͏Փˏ��������Ȃ�
                        physx::PxF32 pxMassA = pxRigidBodyA->getMass();
                        physx::PxF32 pxMassB = pxRigidBodyB->getMass();
                        if (pxMassA == 0.0f && pxMassB == 0.0f)
                        {//����0��continue���̐��0���Z��h������
                            continue;
                        }

                        // �ΏۂƂȂ�A�N�^�[�ɂ������Ă���`������W
                        const physx::PxU32 pxNumShapesB = pxActorB->getShapes(pxShapesB, _countof(pxShapesB));
                        for (physx::PxU32 pxShapeIndexB = 0; pxShapeIndexB < pxNumShapesB; ++pxShapeIndexB)
                        {
                            // �g���K�[�ݒ�̌`��͑ΏۊO
                            physx::PxShape* pxShapeB = pxShapesB[pxShapeIndexB];
                            if (pxShapeB->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
                            {
                                continue;
                            }
                            // �O�p�`���b�V���Ȃǂ̕��G�ȃW�I���g���͑ΏۊO
                            const physx::PxGeometry& pxGeometryB = pxShapeB->getGeometry();
                            if (pxGeometryB.getType() == physx::PxGeometryType::eTRIANGLEMESH)
                            {
                                continue;
                            }

                            // �A�N�^�[�ƌ`��̏�񂩂�p�����v�Z
                            physx::PxTransform pxShapeTransformB = physx::PxShapeExt::getGlobalPose(*pxShapeB, *pxActorB);

                            // �Փˉ����o������
                            {
                                // �Q�̌`�󂪏d�Ȃ��Ă���ꍇ�A�����o�������Ƃ߂荞�ݗʂ��̌v�Z������
                                physx::PxVec3 pxDirection;
                                physx::PxF32 pxDepth;
                                bool intersect = physx::PxGeometryQuery::computePenetration(pxDirection, pxDepth, pxGeometryA, pxShapeTransformA, pxGeometryB, pxShapeTransformB);
                                if (intersect)											//�ǂ��������ɂǂꂭ�炢�߂荞��ł��邩
                                {
                                    // �Q�̃A�N�^�[�̎��ʂ��牟���o������v�Z����		2��mass���牟���o���̊��������߂�
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
                                    // �e�A�N�^�[�ɑ΂��ĉ����o���ʂ��v�Z
                                    physx::PxF32 pxDepthA = pxDepth * pxRateA;
                                    physx::PxF32 pxDepthB = pxDepth * pxRateB;

                                    // �����o������
                                    physx::PxTransform pxTransformA = pxRigidBodyA->getGlobalPose();
                                    physx::PxTransform pxTransformB = pxRigidBodyB->getGlobalPose();

                                    pxTransformA.p += pxDirection * pxDepthA;
                                    pxTransformB.p -= pxDirection * pxDepthB;

                                    pxRigidBodyA->setKinematicTarget(pxTransformA);
                                    pxRigidBodyA->setGlobalPose(pxTransformA);//�����ɂ��̏ꏊ�ɕς��@�u�Ԉړ��@�{����simulate�̎��ɓ����I�ɍs���邯�ǁA
                                    //�����simulate�̒���ɍs���Ă��邩�瑼�ɉe�����Ȃ�

                                    pxRigidBodyB->setKinematicTarget(pxTransformB);
                                    pxRigidBodyB->setGlobalPose(pxTransformB);

                                    auto actorA = static_cast<Actor*>(pxRigidBodyA->userData);
                                    auto actorB = static_cast<Actor*>(pxRigidBodyB->userData);
                                    if (actorA)
                                    {
                                        Transform tA = PhysicsHelper::FromPxTransform(pxTransformA);
                                        actorA->rootComponent_->SetPhysicalTransform(tA); // �����o���ꂽ Transform ��ۑ�
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
                                    //// �Փ˒ʒm
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

// ���C�L���X�g
bool Physics::RayCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, HitResult& result)
{
    //--------------------------
    // NOTE:�A�t�B���^�����O�ݒ�
    //--------------------------
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N
    //OutputDebugStringA(("Ray Origin: " + std::to_string(origin.x) + ", " + std::to_string(origin.y) + ", " + std::to_string(origin.z) + "\n").c_str());
    //OutputDebugStringA(("Ray Dir: " + std::to_string(direction.x) + ", " + std::to_string(direction.y) + ", " + std::to_string(direction.z) + "\n").c_str());

    //--------------------------
    // NOTE:�@���C�L���X�g
    //--------------------------
    physx::PxVec3 pxOrigin(origin.x, origin.y, origin.z);
    physx::PxVec3 pxDirection(direction.x, direction.y, direction.z);
    physx::PxRaycastBuffer/*N<1>*/ pxRaycastBuffer;
    bool hit = pxScene->raycast(
        pxOrigin, pxDirection, distance,
        pxRaycastBuffer,
        physx::PxHitFlag::eDEFAULT,
        pxQueryFilterData,		// NOTE:�A�t�B���^�����O�ݒ�
        &Physics::Instance());
    if (hit && pxRaycastBuffer.hasBlock)
    {
        //--------------------------
        // NOTE:�C�q�b�g���
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

// �X�t�B�A�L���X�g
bool Physics::SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, HitResult& result)
{
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N

    //--------------------------
    // NOTE:�C�V�F�C�v�L���X�g
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

// �X�t�B�A�L���X�g
bool Physics::SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, RaycastHit2& result)
{
    physx::PxQueryFilterData pxQueryFilterData(
        physx::PxQueryFlag::eDYNAMIC |
        physx::PxQueryFlag::eSTATIC |
        physx::PxQueryFlag::ePREFILTER |
        physx::PxQueryFlag::ePOSTFILTER
    );
    pxQueryFilterData.data.word0 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N
    pxQueryFilterData.data.word1 = 0xFFFFFFFF;	// NOTE:�D���C���[�}�X�N

    //--------------------------
    // NOTE:�C�V�F�C�v�L���X�g
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
    // NOTE:�B�t�B���^�����O����
    //--------------------------
    physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
    if ((shapeFilterData.word0 & filterData.word0) == 0)	// NOTE:�E���C���[�}�X�N����
    {
        return physx::PxQueryHitType::eNONE;
    }

    return physx::PxQueryHitType::eTOUCH;
}

physx::PxQueryHitType::Enum Physics::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor)
{
    //--------------------------
    // NOTE:�B�t�B���^�����O����
    //--------------------------
    return physx::PxQueryHitType::eBLOCK;
}

// �Փ˂��Ă��� �ǂ��炩�� Dynamic�iPxRigidDynamic�j
// �����Ƃ� Simulation Shape�i��eSIMULATION_SHAPE�t���O���L���� PxShape�j
// �����Ƃ� Trigger �ł͂Ȃ��i��Trigger ���m or Trigger �Е����� onTrigger() �̑Ώہj
// kinematic �� static ���m�͌Ă΂�Ȃ��@����̏ꍇ player �� stage �͌Ă΂�Ȃ�
void Physics::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    using namespace physx;
    //--------------------------
    // �Փ˃C�x���g���o
    //--------------------------
    for (physx::PxU32 i = 0; i < nbPairs; i++)
    {
        const physx::PxContactPair& pair = pairs[i];

        if (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0 || pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1)
        {
            continue;	// �폜���ꂽ Shape �ƏՓ˂͖���
        }

        physx::PxShape* shapeA = pair.shapes[0];
        physx::PxShape* shapeB = pair.shapes[1];
        if (!shapeA || !shapeB)
        {
            continue;
        }

        // userData���g���āA������CollisionComponent���擾
        CollisionComponent* compA = static_cast<CollisionComponent*>(shapeA->userData);
        CollisionComponent* compB = static_cast<CollisionComponent*>(shapeB->userData);
        //if (!compA || !compB)
        //{
        //    continue;
        //}


        // �����ŏՓ˂����Q�� Actor ���擾
        physx::PxActor* pxActorA = pairHeader.actors[0];
        physx::PxActor* pxActorB = pairHeader.actors[1];
        //physx::PxActor* pxActorA = pair.shapes[0]->getActor();
        //physx::PxActor* pxActorB = pair.shapes[1]->getActor();
        if (!pxActorA || !pxActorB)
        {
            continue;
        }
        // userData���g���āA������Actor���擾
        Actor* ownerA = static_cast<Actor*>(pxActorA->userData);
        Actor* ownerB = static_cast<Actor*>(pxActorB->userData);

        // �Փ˓_�̎擾
        PxContactPairPoint contactPoints[16];
        PxU32 contactCount = pair.extractContacts(contactPoints, 16);

        for (PxU32 j = 0; j < contactCount; j++)
        {
            const PxContactPairPoint& point = contactPoints[j];

            // �Փˈʒu
            PxVec3 hitPos = point.position;

            // �Փ˖@�� B ���� A �֌���
            PxVec3 normal = point.normal;

            // A ���󂯎���� �Փ˗� 
            PxVec3 impulse = point.impulse;

            // �Փ˂̐[��
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

            if (ownerA)// ��{��������delete���Ă���
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
            // ���߂ĐڐG�����Ƃ�
            //for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            //{
            //	const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

            //	auto a = static_cast<CollisionEvent*>(pxActorA->userData);
            //	if (a != nullptr)
            //	{
            //		// NOTE:�K�Փ˃C�x���g�ʒm
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
            //		// NOTE:�K�Փ˃C�x���g�ʒm
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
            // �ڐG���Ă�����
            //for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            //{
            //	const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

            //	auto a = static_cast<CollisionEvent*>(pxActorA->userData);
            //	if (a != nullptr)
            //	{
            //		// NOTE:�K�Փ˃C�x���g�ʒm
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
            //		// NOTE:�K�Փ˃C�x���g�ʒm
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
            // �ڐG���Ă����Ԃ��痣�ꂽ�Ƃ�
            for (physx::PxU32 j = 0; j < pxContactCount; ++j)
            {
                const physx::PxContactPairPoint& pxContactPoint = pxContactPoints[j];

                auto a = static_cast<CollisionEvent*>(pxActorA->userData);
                if (a != nullptr)
                {
                    // NOTE:�K�Փ˃C�x���g�ʒm
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
                    // NOTE:�K�Փ˃C�x���g�ʒm
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
    // NOTE:�H�g���K�[�C�x���g���o
    //--------------------------
    for (physx::PxU32 i = 0; i < count; i++)
    {
        const physx::PxTriggerPair& pair = pairs[i];

        // �폜���ꂽ Shape �̏����͖���
        if (pair.flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
            continue;

        // userData ���玩���� CollisionComponent ���擾
        CollisionComponent* triggerComp = static_cast<CollisionComponent*>(pair.triggerShape->userData);
        CollisionComponent* otherComp = static_cast<CollisionComponent*>(pair.otherShape->userData);

        if (!triggerComp || !otherComp)
            continue;

        //// Trigger �� Enter / Exit �𔻒�
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
            // ���߂ĐڐG�����Ƃ�						//TODO:02�����ł�userData��collisionEvent�̂͂�������
            auto a = static_cast<CollisionEvent*>(pair.triggerActor->userData);
            if (a != nullptr)
            {
                // NOTE:�K�Փ˃C�x���g�ʒm
                CollisionTrigger trigger;
                trigger.pxActor = pair.otherActor;
                a->OnTriggerTouchFound(trigger);//OnTriggerEnter
            }
            auto b = static_cast<CollisionEvent*>(pair.otherActor->userData);
            if (b != nullptr)
            {
                // NOTE:�K�Փ˃C�x���g�ʒm
                CollisionTrigger trigger;
                trigger.pxActor = pair.triggerActor;
                b->OnTriggerTouchFound(trigger);
            }
        }
        if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            // �ڐG���Ă����Ԃ��痣�ꂽ�Ƃ�
            auto a = static_cast<CollisionEvent*>(pair.triggerActor->userData);
            if (a != nullptr)
            {
                // NOTE:�K�Փ˃C�x���g�ʒm
                CollisionTrigger trigger;
                trigger.pxActor = pair.otherActor;
                a->OnTriggerTouchLost(trigger);
            }
            auto b = static_cast<CollisionEvent*>(pair.otherActor->userData);
            if (b != nullptr)
            {
                // NOTE:�K�Փ˃C�x���g�ʒm
                CollisionTrigger trigger;
                trigger.pxActor = pair.triggerActor;
                b->OnTriggerTouchLost(trigger);
            }
        }

#endif // 0
    }
}

//--------------------------
// NOTE:�G�Փˌ��o�t�B���^�����O
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
    // ���C���[�}�X�N���g�p���ďՓ˃t�B���^�����O����
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