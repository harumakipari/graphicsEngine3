#ifndef RIGID_BODY_COMPONENT_H
#define RIGID_BODY_COMPONENT_H

// C++ �W�����C�u����
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>

// �����C�u����
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>
#include <PxShape.h>
#include <geometry/PxConvexMeshGeometry.h>
#include <geometry/PxGeometry.h>
#include <geometry/PxConvexMesh.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// �v���W�F�N�g�̑��̃w�b�_
#include "Physics/Collider.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsHelper.h"
#include "Components/Transform/Transform.h"
#include "Graphics/Resource/InterleavedGltfModel.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Base/Component.h"

class CollisionComponent;
class ShapeComponent;
class Actor;

// XMFLOAT3 �p�n�b�V������r
struct XMFLOAT3Hasher
{
    size_t operator()(const DirectX::XMFLOAT3& v)const
    {
        // �n�b�V���֐�
        size_t hx = std::hash<float>()(v.x);
        size_t hy = std::hash<float>()(v.y);
        size_t hz = std::hash<float>()(v.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

struct XMFLOAT3Equal
{
    bool operator()(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) const {
        constexpr float EPS = 1e-6f;
        return
            fabs(a.x - b.x) < EPS &&
            fabs(a.y - b.y) < EPS &&
            fabs(a.z - b.z) < EPS;
    }
};

// ShapeComponent �� �A�^�b�`����p�� component
class RigidBodyComponent : public Component
{
public:
    //RigidBodyComponent() = default;

    RigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner) : Component(name, owner) {}

    virtual ~RigidBodyComponent() = default;

    virtual void Initialize() {};
    virtual void UpdateComponentToWorld(UpdateTransformFlags update_transform_flags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None) {}


    virtual void Initialize(physx::PxPhysics* physics) = 0;

    virtual void Tick(float deltaTime) = 0;

    virtual void AddToScene(physx::PxScene* scene) = 0;

    virtual void SetTransform(const Transform& worldTransform) = 0;

    virtual void SetCollisionFilter(uint32_t layer, uint32_t mask) = 0;

    virtual void SetGravity(bool useGravity) {}

    virtual void SetKinematic(bool isKinematic) {}

    virtual void Destroy() override = 0;
protected:
    uint32_t layer_ = 0xFFFFFFFF;
    uint32_t mask_ = 0xFFFFFFFF;
};


class SingleRigidBodyComponent :public RigidBodyComponent
{
public:
    SingleRigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner, ShapeComponent* shape)
        : RigidBodyComponent(name, owner), shapeComponent_(shape), material_(Physics::Instance().GetMaterial()) {
    }
    //SingleRigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner, ShapeComponent* shape)
    //    : RigidBodyComponent(name, owner), shapeComponent_(shape), material_(Physics::Instance().GetDefaultMaterial()) {
    //}

    virtual void Initialize(physx::PxPhysics* physics) override;

    virtual void Tick(float deltaTime) override;

    virtual void AddToScene(physx::PxScene* scene) override
    {
        if (pxActor_)
        {
            scene->addActor(*pxActor_);
            isInScene_ = true;
        }
    }

    virtual void SetTransform(const Transform& worldTransform) override
    {
        if (pxActor_)
        {
            pxActor_->setGlobalPose(PhysicsHelper::ToPxTransform(worldTransform));
        }
    }

    virtual void SetCollisionFilter(uint32_t layer, uint32_t mask) override
    {
        layer_ = layer;
        mask_ = mask;
        if (pxShape_)
        {
            physx::PxFilterData filterData(layer, mask, 0, 0);
            pxShape_->setSimulationFilterData(filterData);
            pxShape_->setQueryFilterData(filterData);
        }
    }

    void SetGravity(bool isUseGravity)override
    {
        if (pxActor_)
        {
            pxActor_->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !isUseGravity);
        }
    }

    void SetIntialVelocity(const DirectX::XMFLOAT3& velocity)
    {
        if (pxActor_)
        {
            pxActor_->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
        }
    }

    // �r���Ŏ��ʂ�ݒ肷��֐�
    void SetMass(float newMass)
    {
        //if (!isKinematic_)
        //{// �L�l�}�e�B�b�N����Ȃ�������
        //    return;
        //}
        if (pxActor_)
        {
            physx::PxRigidBodyExt::updateMassAndInertia(*pxActor_, newMass);
        }
        mass_ = newMass;
    }

    // �����蔻��𖳌��ɂ���֐�
    void DisableCollision()
    {
        if (!pxShape_)
        {
            return;
        }
        pxShape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        pxShape_->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
    }

    // �����蔻���L���ɂ���֐�
    void EnableCollision()
    {
        if (!pxShape_)
        {
            return;
        }
        pxShape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        pxShape_->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    }

    // ���I�ɋ��̓����蔻��̔��a��傫������֐�
    void ResizeSphere(float newRadius);

    // ���I�ɃJ�v�Z���̔����傫������֐�
    void ResizeCapsule(float newRadius, float newHeight);

    // ���I�Ƀ{�b�N�X�̔����傫������֐�
    void ResizeBox(float newExtentX, float newExtentY, float newExtentZ);

    void SetKinematic(bool isKinematic)override
    {
        if (pxActor_)
        {
            pxActor_->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
            //shapeComponent_->SetKinematic(isKinematic); // ShapeComonent�����X�V����
        }
        isKinematic_ = isKinematic;
    }

    void SetTrigger(bool isTrigger)
    {
        isTrigger_ = isTrigger;
    }

    virtual void Destroy() override
    {
        if (pxActor_ && isInScene_)
        {
            // deferred �ɕύX
            //Physics::EnqueueDefferfOperations({ DefferdPhysicsOperation::Type::RemoveRigidActor, pxActor_ });
            Physics::Instance().GetScene()->removeActor(*pxActor_);
            isInScene_ = false;
        }

        isDestroyed_ = true;
    }

    void AddImpulse(const DirectX::XMFLOAT3& impulse)
    {
        using namespace physx;
        if (!isKinematic_)
        {// �_�C�i�~�b�N�̎�
            if (pxActor_)
            {
                PxVec3 impulsePx(impulse.x, impulse.y, impulse.z);

                // �u�ԓI�ȑ��x�ω���^����
                pxActor_->addForce(impulsePx, PxForceMode::eIMPULSE);
            }
        }
        //else
        //{// �L�l�}�e�B�b�N�̎�
        //    if (pxActor_)
        //    {
        //        PxTransform current = pxActor_->getGlobalPose();

        //    }

        //}
    }

private:
    physx::PxRigidDynamic* pxActor_ = nullptr;
    physx::PxShape* pxShape_ = nullptr;
    ShapeComponent* shapeComponent_ = nullptr;
    physx::PxMaterial* material_ = nullptr;

    float mass_ = 1.0f;
    bool isDestroyed_ = false;
    bool isInScene_ = false;
    bool isKinematic_ = true;
    bool isTrigger_ = false;

    // ���I�ɔ��a��ύX�������� position �𑫂�����
    float currentRadius_ = 0.0f;

};


// node ���� rigidBody (ConvexMesh)�@���쐬���鎞 
class MultiRigidBodyComponent :public RigidBodyComponent
{
public:
    MultiRigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner, MeshComponent* meshComponent, CollisionComponent* collisionComponent)
        : RigidBodyComponent(name, owner), meshComponent_(meshComponent), collisionComponent_(collisionComponent) {
    }

    void Initialize(physx::PxPhysics* physics)override;

    void Tick(float deltaTime)override;

    void AddToScene(physx::PxScene* scene) override
    {
        if (isAddedToScene_)
        {
            return;
        }
        for (auto* pxBody : rigidBodies_)
        {
            scene->addActor(*pxBody);
        }
        isAddedToScene_ = true;
    }

    void SetTransform(const Transform& worldTransform) {}

    void SetCollisionFilter(uint32_t layer, uint32_t mask)
    {
        layer_ = layer;
        mask_ = mask;
        // pxShape �̔z��ɃZ�b�g���Ă���������
    }

    const std::vector<InterleavedGltfModel::Node>& GetAnimatedNodes() const
    {
        return animatedNodes_;
    }

    void SetKinematic(bool isKinematic)
    {
        for (physx::PxRigidActor* actor : rigidBodies_)
        {
            // RigidDynamic �݂̂� Kinematic ��ݒ�\
            if (physx::PxRigidDynamic* dynActor = actor->is<physx::PxRigidDynamic>())
            {
                dynActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
                if (isKinematic)
                {
                    dynActor->setLinearVelocity({ 0, 0, 0 });
                    dynActor->setAngularVelocity({ 0, 0, 0 });
                }
                else
                {
                    dynActor->wakeUp();
                }
            }
        }
    }

    bool IsKinematic(size_t index) const
    {
        if (index >= rigidBodies_.size())
        {
            return false;
        }
        auto* dyn = rigidBodies_[index]->is<physx::PxRigidDynamic>();
        return dyn ? dyn->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC : false;
    }

    //virtual void Destroy() override
    //{
    //    for (auto pxActor : rigidBodies_)
    //    {
    //        if (pxActor && isAddedToScene_)
    //        {
    //            Physics::Instance().GetScene()->removeActor(*pxActor);
    //            isAddedToScene_ = false;
    //        }
    //        isDestroyed_ = true;
    //    }
    //}

    // �����蔻��𖳌��ɂ���֐�
    void DisableCollision()
    {
        for (physx::PxShape* pxShape : pxShapes_)
        {
            if (pxShape)
            {
                pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
                pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
            }
        }
    }


    virtual void Destroy() override;

private:
    void ComputeGlobalTransformRecursive(std::vector<InterleavedGltfModel::Node>& nodes, int nodeIndex, const DirectX::XMFLOAT4X4& parentGlobal)
    {
        InterleavedGltfModel::Node& node = nodes[nodeIndex];

        // ���[�J���s�� SRT
        //DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation));
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z);
        DirectX::XMMATRIX localMat = S * R * T;

        // �e�s�� * ���[�J���s�� �Ŏ����̃O���[�o���s��
        DirectX::XMMATRIX parentMat = DirectX::XMLoadFloat4x4(&parentGlobal);
        DirectX::XMMATRIX globalMat = parentMat * localMat;
        DirectX::XMStoreFloat4x4(&node.globalTransform, globalMat);

        // �q�m�[�h�����l�ɏ���
        for (int childIdx : node.children)
        {
            ComputeGlobalTransformRecursive(nodes, childIdx, node.globalTransform);
        }
    }

    void ComputeGlobalTransforms(std::vector<InterleavedGltfModel::Node>& nodes)
    {
        // ���[�g��T��
        std::vector<bool> hasParent(nodes.size(), false);
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            for (int c : nodes[i].children)
            {
                hasParent[c] = true;
            }
        }
        // Identity �s���e�Ƃ��āA�e���[�g����ċA
        DirectX::XMFLOAT4X4 identity;
        DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (!hasParent[i])
            {
                ComputeGlobalTransformRecursive(nodes, int(i), identity);
            }
        }
    }

    // mesh ����� vertices �����
    std::vector<DirectX::XMFLOAT3> ReturnPhysxVertices(const InterleavedGltfModel::Mesh& mesh)
    {
#if 0
        std::vector<DirectX::XMFLOAT3> physxVertices;

        for (auto& primitive : mesh.primitives)
        {
            auto& p = primitive;
            //OutputDebugStringA(("index buffer size: " + std::to_string(p.cachedIndices.size()) + "\n").c_str());
            size_t vertexCount = p.cachedVertices.size();
            //int offset = static_cast<int>(p.cachedVertices.size());
            if (primitive.indexBufferView.format == DXGI_FORMAT_R32_UINT)
            {// TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
                const uint32_t* indices = reinterpret_cast<const uint32_t*>(p.cachedIndices.data());
                size_t indexCount = p.cachedIndices.size() / sizeof(uint32_t);

                for (size_t i = 0; i < indexCount; i++)
                {
                    uint32_t index = indices[i];
                    if (index < vertexCount)
                    {
                        physxVertices.push_back(p.cachedVertices[index].position);
                    }
                }
                //uint32_t* data = reinterpret_cast<uint32_t*>(p.cachedIndices.data());
                //for (int i = 0; i < p.cachedIndices.size() / 4; ++i)
                //{
                //    //data[i] += offset;
                //}
            }
            else if (primitive.indexBufferView.format == DXGI_FORMAT_R16_UINT)
            {// TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                const uint16_t* data = reinterpret_cast<const uint16_t*>(p.cachedIndices.data());
                size_t indexCount = p.cachedIndices.size() / sizeof(uint16_t);

                for (size_t i = 0; i < indexCount; i++)
                {
                    uint16_t index = data[i];
                    if (index < vertexCount)
                    {
                        physxVertices.push_back(p.cachedVertices[index].position);
                    }
                }
                //for (int i = 0; i < p.cachedIndices.size() / 2; ++i)
                //{
                //    data[i] += offset;
                //}
            }
            else
            {
                _ASSERT(L"���̃t�H�[�}�b�g�ɂ͑Ή����Ă��Ȃ��ł��B");
            }


            //for(size_t index=0;)
            //physxVertices.emplace_back(p.cachedVertices[data[i]]);s

        }
        //for (auto& primitive : mesh.primitives)
        //{
        //    if (primitive.cachedVertices.size() < 4 || primitive.cachedVertices.size() > 256)
        //    {
        //        int i = 0;
        //    }
        //}
        return physxVertices;

#else
        //bool isMeter = /*meshComponent_->model->isModelInMeters;*/true;
        //float unitScale = isMeter ? 1.0f : 0.01f;

        std::unordered_set<DirectX::XMFLOAT3, XMFLOAT3Hasher, XMFLOAT3Equal> uniquePositions;

        for (auto& primitive : mesh.primitives)
        {
            for (auto& vertex : primitive.cachedVertices)
            {
                DirectX::XMFLOAT3 scaledPos =
                {
                    vertex.position.x /** unitScale*/,
                    vertex.position.y /** unitScale*/,
                    vertex.position.z /** unitScale*/,
                };

                uniquePositions.insert(scaledPos);
            }
        }

        // vector �ɃR�s�[���ĕԂ�
        return std::vector<DirectX::XMFLOAT3>(uniquePositions.begin(), uniquePositions.end());

#endif // 0
    }

#if 0
    // Vertices �� Data ���� ConvexMesh ���쐬����
    physx::PxConvexMesh* ToPxConvexMesh(physx::PxPhysics* physics, const std::vector<InterleavedGltfModel::Mesh::Vertex>& vertices)
    {
        using namespace physx;

        // ���_���`�F�b�N�i�����I�Ɂj
        if (vertices.size() < 4 || vertices.size() > 256)
        {
            OutputDebugStringA("ConvexMesh cooking skipped: ���_���� 4 ���� or 256 ����\n");
            return nullptr;
        }

        //std::set<DirectX::XMFLOAT3> uniqueVerts;
        //for (const auto& v : vertices) uniqueVerts.insert(v.position);
        //if (uniqueVerts.size() < 4) {
        //    OutputDebugStringA("ConvexMesh skipped: �������j�[�N���_�� < 4\n");
        //    return nullptr;
        //}

        physx::PxTolerancesScale tolerancesScale;
        physx::PxCookingParams cookingParams(tolerancesScale);
        cookingParams.convexMeshCookingType = physx::PxConvexMeshCookingType::Enum::eQUICKHULL;
        cookingParams.gaussMapLimit = 256;
        physx::PxConvexMeshDesc pxMeshDesc;

        pxMeshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
        pxMeshDesc.points.stride = sizeof(InterleavedGltfModel::Mesh::Vertex);
        //pxMeshDesc.points.data = &vertices[0];
        pxMeshDesc.points.data = vertices.data();
        //pxMeshDesc.flags = ::PxConvexFlag::eCOMPUTE_CONVEX;
        pxMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
        physx::PxConvexMesh* convexMesh = nullptr;
        physx::PxDefaultMemoryOutputStream writeBuffer;
        if (!PxCookConvexMesh(cookingParams, pxMeshDesc, writeBuffer))
        {
            _ASSERT(L" PxCookConvexMesh �����s���܂����I");
        }
        physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        convexMesh = physics->createConvexMesh(readBuffer);
        return convexMesh;
    }
#else
    // Vertices �� Data ���� ConvexMesh ���쐬����
    physx::PxConvexMesh* ToPxConvexMesh(physx::PxPhysics* physics, const std::vector<DirectX::XMFLOAT3>& vertices)
    {
        using namespace physx;

        // ���_���`�F�b�N�i�����I�Ɂj
        if (vertices.size() < 4 || vertices.size() > 256)
        {
            OutputDebugStringA("ConvexMesh cooking skipped: ���_���� 4 ���� or 256 ����\n");
            return nullptr;
        }

        //std::set<DirectX::XMFLOAT3> uniqueVerts;
        //for (const auto& v : vertices) uniqueVerts.insert(v.position);
        //if (uniqueVerts.size() < 4) {
        //    OutputDebugStringA("ConvexMesh skipped: �������j�[�N���_�� < 4\n");
        //    return nullptr;
        //}

        physx::PxTolerancesScale tolerancesScale;
        physx::PxCookingParams cookingParams(tolerancesScale);
        cookingParams.convexMeshCookingType = physx::PxConvexMeshCookingType::Enum::eQUICKHULL;
        cookingParams.gaussMapLimit = 256;
        physx::PxConvexMeshDesc pxMeshDesc;

        pxMeshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
        //pxMeshDesc.points.stride = sizeof(InterleavedGltfModel::Mesh::Vertex);
        pxMeshDesc.points.stride = sizeof(DirectX::XMFLOAT3);
        //pxMeshDesc.points.data = &vertices[0];
        pxMeshDesc.points.data = vertices.data();
        //pxMeshDesc.flags = ::PxConvexFlag::eCOMPUTE_CONVEX;
        pxMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
        physx::PxConvexMesh* convexMesh = nullptr;
        physx::PxDefaultMemoryOutputStream writeBuffer;
        if (!PxCookConvexMesh(cookingParams, pxMeshDesc, writeBuffer))
        {
            _ASSERT(L" PxCookConvexMesh �����s���܂����I");
        }
        physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        convexMesh = physics->createConvexMesh(readBuffer);
        return convexMesh;
    }

#endif // 0

    // rigidBodies �̐ڑ�
    void CreateFixedJoints(MeshComponent* meshComponent)
    {
        using namespace physx;
        PxPhysics* pxPhysics = Physics::Instance().GetPhysics();
        auto& model = meshComponent->model;

        for (size_t parentIndex = 0; parentIndex < rigidBodies_.size(); ++parentIndex)
        {
            PxRigidActor* parentBody = rigidBodies_[parentIndex];
            if (!parentBody)
            {// ��Ȃ�X�L�b�v
                continue;
            }

            auto nodes = model->GetNodes();
            auto node = nodes[parentIndex];
            for (int childIndex : node.children)
            {
                if (childIndex < 0 || static_cast<size_t>(childIndex) >= rigidBodies_.size())
                {
                    continue;
                }

                PxRigidActor* childBody = rigidBodies_[childIndex];
                if (!childBody)
                {
                    continue;
                }

                PxFixedJoint* joint = PxFixedJointCreate(*pxPhysics, parentBody, PxTransform(PxIdentity), childBody, PxTransform(PxIdentity));
                joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, true);
            }
        }
    }

    bool IsAddedToScene() const
    {
        return isAddedToScene_;
    }


private:
    std::vector<physx::PxShape*> pxShapes_;
    std::vector<physx::PxRigidActor*> rigidBodies_;
    // �X�V���� nodeIndex �� rigidBody ��R�Â��Ă�������
    std::unordered_map<size_t, physx::PxRigidActor*> nodeIndexToRigidBody_;
    // �`�掞�ɕK�v�� nodes
    std::vector<InterleavedGltfModel::Node> animatedNodes_;
    MeshComponent* meshComponent_ = nullptr;
    // shape �� userData �ɓo�^����
    CollisionComponent* collisionComponent_ = nullptr;
    // �O�̃{�[���Ƃ̃W���C���g���쐬����̂Ɏg�p����
    physx::PxRigidDynamic* previousBody_ = nullptr;
    DirectX::XMFLOAT4X4 previousNodeTransform;

    bool isAddedToScene_ = false;
    bool isDestroyed_ = false;
};

// �ÓI�ȃX�e�[�W�p RigidBody
class TriangleMeshRigidBodyComponent :public RigidBodyComponent
{
public:
    //TriangleMeshRigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner, MeshComponent* meshComponent)
    //    : RigidBodyComponent(name, owner), owner_(meshComponent), material_(Physics::Instance().GetDefaultMaterial()) {
    //}
    TriangleMeshRigidBodyComponent(const std::string& name, std::shared_ptr<Actor> owner, MeshComponent* meshComponent)
        : RigidBodyComponent(name, owner), owner_(meshComponent), material_(Physics::Instance().GetMaterial()) {
    }

    void Initialize(physx::PxPhysics* physics) override;

    void Tick(float deltaTime) override {}

    void AddToScene(physx::PxScene* scene)override
    {
        if (pxActor_)
        {
            scene->addActor(*pxActor_);
        }
    }

    void SetTransform(const Transform& worldTransform) override
    {
        if (pxActor_)
        {
            pxActor_->setGlobalPose(PhysicsHelper::ToPxTransform(worldTransform));
        }
    }

    void SetCollisionFilter(uint32_t layer, uint32_t mask)
    {
        layer_ = layer;
        mask_ = mask;
        if (pxShape_)
        {
            physx::PxFilterData filterData(layer, mask, 0, 0);
            pxShape_->setSimulationFilterData(filterData);
            pxShape_->setQueryFilterData(filterData);
        }
    }

    virtual void Destroy()override
    {
        if (pxActor_)
        {
            pxActor_->release();
            pxActor_ = nullptr;
        }

        pxShape_ = nullptr; // PxShape �̓A�N�^�[�����L���Ă���̂� release �s�v
    }
private:
    MeshComponent* owner_ = nullptr;
    physx::PxRigidStatic* pxActor_ = nullptr;
    physx::PxShape* pxShape_ = nullptr;
    physx::PxMaterial* material_ = nullptr;
};

#endif //RIGID_BODY_COMPONENT_H