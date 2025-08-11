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
    Actor* actor = nullptr;     // �Փ˂�������
    ShapeComponent* component = nullptr;    // �Փ˂����R���|�[�l���g
    float distance = 0.0f;      // ����
    DirectX::XMFLOAT3 hitPoint;     // �q�b�g����
    DirectX::XMFLOAT3 normal;       // �@��
};
// �t�B�W�N�X
class Physics
    : public physx::PxQueryFilterCallback		// NOTE:�B�t�B���^�����O�C���^�[�t�F�[�X�̌p��
    , public physx::PxSimulationEventCallback	// NOTE:�F�Փ˃C�x���g�C���^�[�t�F�[�X�p��
{
private:
    Physics() = default;
    ~Physics() = default;

public:
    // �C���X�^���X�擾
    static Physics& Instance()
    {
        static Physics instance;
        return instance;
    }

    // ������
    void Initialize();

    // �I����
    void Finalize();

    // �X�V����
    void Update(float elapsedTime);

    // �t�B�W�N�X�擾
    physx::PxPhysics* GetPhysics() { return pxPhysics; }

    // �`������֐�
    physx::PxShape* CreateShape(const physx::PxGeometry& geometry) { return pxPhysics->createShape(geometry, *pxMaterial); }

    // �V�[���擾
    physx::PxScene* GetScene() { return pxScene; }

    // �R���g���[���[�}�l�[�W���[�擾
    physx::PxControllerManager* GetControllerManager() { return pxControllerManager; }

    // �}�e���A���擾
    physx::PxMaterial* GetMaterial() { return pxMaterial; }

    // �f�t�H���g�̃}�e���A���擾
    physx::PxMaterial* GetDefaultMaterial()
    {
        physx::PxMaterial* defaultMaterial_ = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f); /* static friction, dynamic friction, restitution*/
        return defaultMaterial_;
    }

    // ���C�L���X�g
    bool RayCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, HitResult& result);

    // �X�t�B�A�L���X�g
    bool SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, HitResult& result);
    bool SphereCast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float distance, float radius, RaycastHit2& result);
    // simulate ��ł��鏈����ǉ�����
    static void EnqueueDefferfOperations(const DefferdPhysicsOperation& op)
    {
        defferfOps_.push_back(op);
    }

    // sumilate ��Ɏ��s
    void ExecuteDefferdOperations();

protected:
    //--------------------------
    // NOTE:�B�t�B���^�����O�C���^�[�t�F�[�X�֐�
    //--------------------------
    physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
    physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override;

    //--------------------------
    // NOTE:�F�Փ˃C�x���g�C���^�[�t�F�[�X�֐�
    //--------------------------
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
    void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override {};
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {};

private:
    //--------------------------
    // NOTE:�G�Փˌ��o�t�B���^�����O
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
