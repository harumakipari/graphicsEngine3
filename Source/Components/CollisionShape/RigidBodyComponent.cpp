#include "RigidBodyComponent.h"

#include <PxPhysicsAPI.h>
#include "Physics/PhysicsHelper.h"
#include "ShapeComponent.h"
#include "Core/Actor.h"

void SingleRigidBodyComponent::Initialize(physx::PxPhysics* physics)
{
    using namespace physx;
    auto info = shapeComponent_->GetPhysicsShapeInfo();

    DirectX::XMFLOAT3 pos = shapeComponent_->GetOwner()->GetPosition();
    DirectX::XMFLOAT4 rot = shapeComponent_->GetComponentRotation();
    PxVec3 pxPosition(pos.x, pos.y, pos.z);
    PxQuat pxRotation(rot.x, rot.y, rot.z, rot.w);
    // physx �̌��_���グ�邽��
    //pos.y += shapeComponent_->GetModelHeight();
    // PxTransform ���쐬���Ďg�p����
    PxTransform transform(pxPosition, pxRotation);

    pxActor_ = physics->createRigidDynamic(transform);
    pxActor_->userData = shapeComponent_->GetOwner();    // Actor �ւ̃|�C���^

    // ��O������ true �ɂ��邱�Ƃň�ӓI�Ȑ�p�� shape �ɂ���
    pxShape_ = physics->createShape(info.geometry.any(), *material_, true);

    pxShape_->userData = shapeComponent_;    // ShapeComponent �ւ̃|�C���^
    if (shapeComponent_->GetCollisionType() == "Capsule")
    {
        // �`��̃��[�J���p���𒲐�
        physx::PxTransform pxShapeTransform = pxShape_->getLocalPose();
        pxShapeTransform.q = physx::PxQuat(physx::PxPiDivTwo, physx::PxVec3(0.0f, 1.0f, 0.0f));	//90�x��]������
        pxShape_->setLocalPose(pxShapeTransform);	//�V�[���ɐݒ�(�J�v�Z���𗧂����邽��)�ʏ�͉�����
    }
    else
    {
        // ��ɕ␳����
        pxShape_->setLocalPose(physx::PxTransform(physx::PxVec3(0.0f, shapeComponent_->GetModelHeight(), 0.0f)));
    }
    physx::PxFilterData filterData;
    filterData.word0 = layer_;
    filterData.word1 = mask_;

    pxShape_->setSimulationFilterData(filterData);
    pxShape_->setQueryFilterData(filterData);

    pxShape_->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    //if (isTrigger_)
    //{
    //    pxShape_->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    //}
    //else
    {// �f�t�H���g
        pxShape_->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    }
    //pxActor_->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    pxActor_->attachShape(*pxShape_);

    pxActor_->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, shapeComponent_->IsKinematic());

    PxRigidBodyExt::updateMassAndInertia(*pxActor_, mass_);
}

void SingleRigidBodyComponent::Tick(float deltaTime)
{
    using namespace physx;

    if (!pxActor_)
    {
        return;
    }

    if (shapeComponent_->IsKinematic())
    {
        Transform t = shapeComponent_->GetComponentWorldTransform();
        //Transform t = shapeComponent_->GetOwner()->rootComponent_->GetFinalWorldTransform();

        if (shapeComponent_->GetCollisionType() == "Capsule")
        {// ShapeComponent ���J�v�Z���̎�
            ShapeComponent::CapsuleAxis axis = shapeComponent_->GetCapusleAxis();
            if (axis == ShapeComponent::CapsuleAxis::y)
            {// physx �� z �����@�㎲������
                DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), DirectX::XMConvertToRadians(90.0f));
                DirectX::XMFLOAT4 rotation = t.GetRotation();
                DirectX::XMVECTOR qOriginal = DirectX::XMLoadFloat4(&rotation);
                DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(q, qOriginal);
                DirectX::XMStoreFloat4(&rotation, qResult);
                //t.SetRotation(rotation);
            }
            else if (axis == ShapeComponent::CapsuleAxis::x)
            {// �Q�[������ x��
                DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), DirectX::XMConvertToRadians(90.0f));
                DirectX::XMFLOAT4 rotation = t.GetRotation();
                DirectX::XMVECTOR qOriginal = DirectX::XMLoadFloat4(&rotation);
                DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(q, qOriginal);
                DirectX::XMStoreFloat4(&rotation, qResult);
                //t.SetRotation(rotation);
                PxTransform pxT = PhysicsHelper::ToPxTransform(t);
                // �����蔻�肪�n�ʂ̉��ɍs���̂�h��
                pxT.p.x += shapeComponent_->GetModelHeight();
                pxActor_->setKinematicTarget(pxT);
                //PhysicsHelper::DebugPrintCollisionInfo("capsule", shapeComponent_->GetPhysicsShapeInfo().geometry.any(), pxT);
                return;
            }
            else
            {// �Q�[������ z��
                //DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), DirectX::XMConvertToRadians(90.0f));
                //DirectX::XMFLOAT4 rotation = t.GetRotation();
                //DirectX::XMVECTOR qOriginal = DirectX::XMLoadFloat4(&rotation);
                //DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(q, qOriginal);
                //DirectX::XMStoreFloat4(&rotation, qResult);
                //t.SetRotation(rotation);
            }
        }

        PxTransform pxT = PhysicsHelper::ToPxTransform(t);
        // �����蔻�肪�n�ʂ̉��ɍs���̂�h��
        //pxT.p.y += shapeComponent_->GetModelHeight();
        pxActor_->setKinematicTarget(pxT);
    }
    else
    {// �_�C�i�~�b�N�̎�
        // ��ɕ␳����

        PxTransform pxT = pxActor_->getGlobalPose();
        // �����蔻�肪�n�ʂ̉��ɍs���̂�h��
        if (shapeComponent_->GetCollisionType() == "Sphere")
        {// ���I�ɔ��a��ύX�������Ɏg�p
            //pxShape_->setLocalPose(physx::PxTransform(physx::PxVec3(0.0f, currentRadius_, 0.0f)));
            pxT.p.y += currentRadius_;
        }
        GetOwner()->SetPosition({ pxT.p.x,pxT.p.y,pxT.p.z });
        GetOwner()->SetQuaternionRotation({ pxT.q.x,pxT.q.y,pxT.q.z ,pxT.q.w });
        //shapeComponent_->SetWorldLocationDirect({ pxT.p.x,pxT.p.y,pxT.p.z });
        //shapeComponent_->SetWorldRotationDirect({ pxT.q.x,pxT.q.y,pxT.q.z ,pxT.q.w });
    }
}

// ���I�ɋ��̓����蔻��̔��a��傫������֐�
void SingleRigidBodyComponent::ResizeSphere(float newRadius)
{
    if (!pxShape_)
    {
        return;
    }

    if (shapeComponent_->GetCollisionType() != "Sphere")
    {
        return;
    }

    // Geometry���擾
    physx::PxGeometryHolder holder = pxShape_->getGeometry();

    // Sphere�ɕϊ����Ĕ��a��ύX
    physx::PxSphereGeometry sphere = holder.sphere();
    sphere.radius = newRadius;

    // ���I�� position ����ɏグ�邽��
    currentRadius_ = newRadius;

    // Geometry���Đݒ�
    pxShape_->setGeometry(sphere);
}

// ���I�ɃJ�v�Z���̔����傫������֐�
void SingleRigidBodyComponent::ResizeCapsule(float newRadius, float newHeight)
{
    if (!pxShape_)
    {
        return;
    }

    if (shapeComponent_->GetCollisionType() != "Capsule")
    {
        return;
    }

    // Geometry���擾
    physx::PxGeometryHolder holder = pxShape_->getGeometry();

    // Capsule �ɕϊ����Ĕ��a��ύX
    physx::PxCapsuleGeometry capsule = holder.capsule();
    capsule.radius = newRadius;
    capsule.halfHeight = newHeight * 0.5f;

    // Geometry���Đݒ�
    pxShape_->setGeometry(capsule);
}

// ���I�Ƀ{�b�N�X�̔����傫������֐�
void SingleRigidBodyComponent::ResizeBox(float newExtentX, float newExtentY, float newExtentZ)
{
    if (!pxShape_)
    {
        return;
    }

    if (shapeComponent_->GetCollisionType() != "Box")
    {
        return;
    }

    // Geometry���擾
    physx::PxGeometryHolder holder = pxShape_->getGeometry();

    // Box �ɕϊ����Ĕ��a��ύX
    physx::PxBoxGeometry box = holder.box();
    box.halfExtents.x = newExtentX;
    box.halfExtents.y = newExtentY;
    box.halfExtents.z = newExtentZ;

    // Geometry���Đݒ�
    pxShape_->setGeometry(box);
}

void MultiRigidBodyComponent::Initialize(physx::PxPhysics* physics)
{
#if 1
    using namespace physx;
    auto& model = meshComponent_->model;

    animatedNodes_ = model->GetNodes();

    for (size_t i = 0; i < animatedNodes_.size(); ++i)
    {
        //std::stringstream ss;
        //ss << "Node " << i << " (" << animatedNodes_[i].name << ") children: ";
        //for (auto c : animatedNodes_[i].children)
        //    ss << c << " ";
        //OutputDebugStringA(ss.str().c_str());
    }

    ComputeGlobalTransforms(animatedNodes_);


    //// owner_ �� position �� rotaion ���擾
    //// owner_ ��Transform�i���[���h��Ԋ�j��PhysX Transform�ɕϊ�
#if 1
    //DirectX::XMFLOAT3 pos = meshComponent_->GetOwner()->GetPosition();
    DirectX::XMFLOAT3 pos = meshComponent_->GetRelativeLocation();
    DirectX::XMFLOAT4 rot = meshComponent_->GetOwner()->GetQuaternionRotation();
    //PxVec3 pxPosition(pos.x, pos.y, pos.z);
    PxVec3 pxPosition(pos.x, pos.y, pos.z);
    PxQuat pxRotation(rot.x, rot.y, rot.z, rot.w);
    // PxTransform ���쐬���Ďg�p����
    PxTransform ownerTransform(pxPosition, pxRotation);

#endif // 0

    // �e Node ���� mesh �������Ă���m�[�h�݂̂���������
    for (size_t nodeIndex = 0; nodeIndex < animatedNodes_.size(); ++nodeIndex)
    {
        const auto& node = animatedNodes_[nodeIndex];

        if (node.mesh < 0)
        {// mesh�@�������Ȃ� node �̓X�L�b�v
            continue;
        }

        auto& mesh = model->meshes[node.mesh];
        PxTransform nodeTransform = PhysicsHelper::ToPxTransform(node.globalTransform);
        PxTransform worldPx = ownerTransform * nodeTransform;

        // �m�[�h�̃��[���h�ϊ��� PhysX �ϊ���
        //PxTransform nodePx = PhysicsHelper::ToPxTransform(node.globalTransform);
        //PxTransform worldPx = ownerTransform * nodePx;

        // RigidDynamic ���쐬
        PxRigidDynamic* currentBody = physics->createRigidDynamic(worldPx);
        currentBody->userData = meshComponent_->GetOwner();

        // �����Ń��O���o��
        {
        //    char buf[256];
        //    sprintf_s(buf, "[Init] Node %zu: worldPx.p = (%f, %f, %f)\n",
        //        nodeIndex,
        //        worldPx.p.x, worldPx.p.y, worldPx.p.z);
        //    OutputDebugStringA(buf);
        }

        std::vector<DirectX::XMFLOAT3> physicsVertices = ReturnPhysxVertices(mesh);
        PxConvexMesh* convexMesh = ToPxConvexMesh(physics, physicsVertices);
        //PxConvexMesh* convexMesh = ToPxConvexMesh(physics, mesh.primitives[0].cachedVertices);
        bool isMeter = meshComponent_->model->isModelInMeters;
        float unitScale = isMeter ? 1.0f : 0.01f;

        //const DirectX::XMFLOAT3 scale = meshComponent_->GetComponentScale();
        const DirectX::XMFLOAT3 scale = { unitScale,unitScale,unitScale };
        PxConvexMeshGeometry geometry(convexMesh, PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z)));
        PxMaterial& material = *Physics::Instance().GetMaterial();
        //PxMaterial& mat = *Physics::Instance().GetMaterial();
        //mat.setStaticFriction(1.0f);
        //mat.setDynamicFriction(1.0f);
        //mat.setRestitution(0.0f); // �o�E���h���Ȃ��悤��
        if (!geometry.isValid())
        {
            OutputDebugStringA("PxConvexMeshGeometry is invalid.\n");
            continue;
        }
        if (!convexMesh)
        {
            OutputDebugStringA("ToPxConvexMesh failed: convexMesh is null.\n");
            continue;
        }

        //if (primitive.cachedVertices.size() < 4 || primitive.cachedVertices.size() > 256)
        //if (mesh.primitives[0].cachedVertices.size() < 4 || mesh.primitives[0].cachedVertices.size() > 256)
        //{
        //    OutputDebugStringA("ConvexMesh creation skipped due to invalid vertex count.\n");
        //    continue;
        //}
        if (!geometry.isValid())
        {
            OutputDebugStringA("PxConvexMeshGeometry is invalid.\n");
            continue;
        }
        PxShape* pxShape_ = physics->createShape(geometry, material,true); // ��p shape ���쐬����
        //pxShape_->userData = meshComponent_;   // MeshComponent �ւ̃|�C���^
        pxShape_->userData = collisionComponent_;   // MeshComponent �ւ̃|�C���^

        physx::PxFilterData filterData;
        filterData.word0 = layer_;
        filterData.word1 = mask_;

        pxShape_->setSimulationFilterData(filterData);
        pxShape_->setQueryFilterData(filterData);
        pxShape_->setLocalPose(PxTransform(PxIdentity));
        pxShape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        pxShape_->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        pxShape_->setFlag(PxShapeFlag::eVISUALIZATION, true); // ���ꂪ�Ȃ��ƕ\������Ȃ�
        // nodeTransform�i���[�J����ԁj�����[���h��Ԃɕϊ��iownerTransform�ƍ����j
        PxTransform worldTrnasform = ownerTransform * nodeTransform;

        // RigidDynamic ���쐬
        //PxRigidDynamic* curretBody/* pxActor_ */ = physics->createRigidDynamic(worldTrnasform);
        //curretBody->userData = meshComponent_->GetOwner();     // Actor �ւ̃|�C���^

        // �d�͂𖳌�������
        //curretBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

        // �ŏ��L�l�}�e�B�b�N�Ƃ��ēo�^����
        //curretBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        currentBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);

        //currentBody->setActorFlag(PxActorFlag::eVISUALIZATION, true);
        //currentBody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
        //PxReal pxMass = 0.0f;
        //PxMat33 inertia;
        //PxVec3 centerOfMass;
        //convexMesh->getMassInformation(pxMass, inertia, centerOfMass);
        //float volumeScale = pxMass;       // �̐ς��擾
        //float baseMass = 10.0f; // �����
        //float mass = baseMass * volumeScale;
        //currentBody->setMass(mass);
        currentBody->setMass(20.0f);  
        currentBody->setMassSpaceInertiaTensor(PxVec3(1.0f,1.0f,1.0f));  // Y���̉�]��}����
        currentBody->setLinearDamping(0.9f);
        //currentBody->setLinearDamping(0.3f);
        currentBody->setAngularDamping(0.8f);

        currentBody->attachShape(*pxShape_);
        // ����͂��̎��_�� addActor�@����� �������Z�̋����ɍ����x���邩���߂�
        //Physics::Instance().GetScene()->addActor(*curretBody);

        nodeIndexToRigidBody_[nodeIndex] = currentBody;
        rigidBodies_.push_back(currentBody);
        pxShapes_.push_back(pxShape_);
        //}
    }
#else
    using namespace physx;
    auto& model = owner_->model;

    // owner �̃��[���hTransform�i���_�Ɖ�]�j���擾
    DirectX::XMFLOAT3 pos = owner_->GetOwner()->GetPosition();
    DirectX::XMFLOAT4 rot = owner_->GetOwner()->GetQuaternionRotation();
    PxTransform ownerTransform(PxVec3(pos.x, pos.y, pos.z), PxQuat(rot.x, rot.y, rot.z, rot.w));

    // �X�P�[���i1�����擾�A�m�[�h�P�ʂɕύX��������΂��̎擾�ʒu���H�v�j
    const DirectX::XMFLOAT3 scale = owner_->GetComponentScale();
    PxVec3 pxScale(scale.x, scale.y, scale.z);

    // �O��̍��́ETransform��ێ�
    PxRigidDynamic* previousBody = nullptr;
    DirectX::XMFLOAT4X4 previousNodeTransform;

    animatedNodes_ = model->nodes;

    for (size_t nodeIndex = 0; nodeIndex < animatedNodes_.size(); ++nodeIndex)
    {
        const auto& node = animatedNodes_[nodeIndex];

        if (node.mesh < 0)
        {
            continue; // ���b�V���������Ȃ��m�[�h�̓X�L�b�v
        }

        const auto& mesh = model->meshes[node.mesh];
        PxMaterial& material = *Physics::Instance().GetMaterial();

        // �m�[�h�̃��[���hTransform���擾
        PxTransform nodeTransform = PhysicsHelper::ToPxTransform(node.globalTransform);
        PxTransform worldTransform = ownerTransform * nodeTransform;

        // ���̂��쐬�iprimitive�����������Ă�1�ɂ܂Ƃ߂�j
        PxRigidDynamic* currentBody = physics->createRigidDynamic(worldTransform);
        currentBody->userData = owner_->GetOwner(); // Actor�|�C���^�Ȃ�

        for (const auto& primitive : mesh.primitives)
        {
            PxConvexMesh* convexMesh = ToPxConvexMesh(physics, primitive.cachedVertices);
            PxConvexMeshGeometry geometry(convexMesh, PxMeshScale(pxScale));

            PxShape* shape = physics->createShape(geometry, material);
            shape->userData = owner_;

            PxFilterData filterData;
            filterData.word0 = layer_;
            filterData.word1 = mask_;
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);

            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

            currentBody->attachShape(*shape);
        }

        // �V�[���ɍ��̂�ǉ�
        Physics::Instance().GetScene()->addActor(*currentBody);

        // �o�^
        nodeIndexToRigidBody_[nodeIndex] = currentBody;
        rigidBodies_.push_back(currentBody);

        // �O�̍��̂�����ꍇ�A�W���C���g�Őڑ�
        if (previousBody)
        {
            // Transform A�i�O�j�� B�i���j�� PxTransform �ɕϊ�
            PxTransform a = PhysicsHelper::ToPxTransform(previousNodeTransform);
            PxTransform b = PhysicsHelper::ToPxTransform(node.globalTransform);

            // A��Ԃł�B�̑���Transform
            PxTransform relativeToA = a.getInverse() * b;

            // FixedJoint �쐬
            PxFixedJoint* joint = PxFixedJointCreate(*physics,
                previousBody, PxTransform(PxIdentity),
                currentBody, relativeToA);
            joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, true);
        }

        // ���̃W���C���g�p�ɕۑ�
        previousBody = currentBody;
        previousNodeTransform = node.globalTransform;
    }
#endif
}

void MultiRigidBodyComponent::Destroy() 
{
    using namespace physx;
#if 0
    for (physx::PxRigidActor* actor : rigidBodies_)
    {
        if (actor)
        {
            if (physx::PxScene* scene = actor->getScene())
            {// actor �� scene �ɑ����Ă��鎞����
                scene->removeActor(*actor);
            }

            actor->release();
        }
    }

#endif // 0
    for (auto actor : rigidBodies_)
    {
        if (auto scene = actor->getScene())
        {
            scene->removeActor(*actor);
        }


        PxU32 nb = actor->getNbShapes();
        std::vector<PxShape*> shapes(nb);
        actor->getShapes(shapes.data(), nb);

        for (PxShape* shape : shapes)
        {
            PxGeometryHolder holder = shape->getGeometry();
            if (holder.getType() == PxGeometryType::eCONVEXMESH) 
            {
                auto geom = holder.convexMesh(); // PxConvexMeshGeometry
                geom.convexMesh->release();
            }

            //if (Actor* ownerA = static_cast<Actor*>(actor->userData))
            //{
            //    char b[256];
            //    sprintf_s(b, "Actor Name :%s is destroy\n", ownerA->GetName());
            //    OutputDebugStringA(b);
            //}
            actor->detachShape(*shape);
            shape->release();
        }
        actor->release();

    }

    rigidBodies_.clear();


    pxShapes_.clear();
    nodeIndexToRigidBody_.clear();
    animatedNodes_.clear();
    isAddedToScene_ = false;
}



void MultiRigidBodyComponent::Tick(float deltaTime)
{
    if (animatedNodes_.empty())
    {
        return;
    }

    // owner_ �� worldTransform ���擾
    DirectX::XMFLOAT4X4 ownerTransform = meshComponent_->GetOwner()->GetWorldTransform();
    DirectX::XMMATRIX OwnerMatrix = DirectX::XMLoadFloat4x4(&ownerTransform);

    static bool frag = true;
    if (GetAsyncKeyState('N') & 0x8000)
    {
        frag = !frag;
    }

    using namespace physx;
#if 1
    for (const auto& [nodeIndex, body] : nodeIndexToRigidBody_)
    {

        if (!frag)
        {
            // ���I���̂��`�F�b�N���L���X�g
            physx::PxRigidDynamic* dynamicBody = body->is<physx::PxRigidDynamic>();
            if (dynamicBody)
            {
                dynamicBody->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
                dynamicBody->setAngularVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
            }
        }
        else
        {
#if 0
            DirectX::XMFLOAT3 pos = animatedNodes_[nodeIndex].translation;
            DirectX::XMFLOAT4 rot = animatedNodes_[nodeIndex].rotation;
            DirectX::XMFLOAT3 scale = animatedNodes_[nodeIndex].scale;
#else
            const PxTransform& pxT = body->getGlobalPose();
            DirectX::XMFLOAT3 pos = { pxT.p.x, pxT.p.y, pxT.p.z };
            DirectX::XMFLOAT4 rot = { pxT.q.x, pxT.q.y, pxT.q.z, pxT.q.w };
            //DirectX::XMFLOAT3 scale = meshComponent_->GetComponentScale();
            //DirectX::XMFLOAT3 scale = animatedNodes_[nodeIndex].scale;
            DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
#endif
            //DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
            DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rot));
            DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

            DirectX::XMMATRIX LocalMatrix = S * R * T;

            // �e�� Transform ����Z���čŏI�� Transform �ɂ���
            DirectX::XMMATRIX M = LocalMatrix/* * OwnerMatrix*/;

            DirectX::XMStoreFloat4x4(&animatedNodes_[nodeIndex].globalTransform, M);
        }
    }

#endif // 0

    //for (const auto& [nodeIndex, body] : nodeIndexToRigidBody_)
    //{
    //    // ���I���̂��`�F�b�N���L���X�g
    //    physx::PxRigidDynamic* dynamicBody = body->is<physx::PxRigidDynamic>();
    //    if (dynamicBody)
    //    {
    //        const PxTransform& pxT = dynamicBody->getGlobalPose();

    //        // PxTransform -> DirectX::XMFLOAT4x4 �ϊ�
    //        DirectX::XMMATRIX M =
    //            DirectX::XMMatrixScaling(animatedNodes_[nodeIndex].scale.x,
    //                animatedNodes_[nodeIndex].scale.y,
    //                animatedNodes_[nodeIndex].scale.z) *
    //            DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(pxT.q.x, pxT.q.y, pxT.q.z, pxT.q.w)) *
    //            DirectX::XMMatrixTranslation(pxT.p.x, pxT.p.y, pxT.p.z);

    //        DirectX::XMStoreFloat4x4(&animatedNodes_[nodeIndex].globalTransform, M);
    //    }
    //}
}

void TriangleMeshRigidBodyComponent::Initialize(physx::PxPhysics* physics)
{
    using namespace physx;

    auto& model = owner_->model; //batchMeshes

    //���f�������[�g���P�ʂ� cm�P�ʂ̎���false�ɂ���
    bool isMeter = model->isModelInMeters;

    float unitScale = isMeter ? 1.0f : 0.01f;

    PxTolerancesScale tolerancesScale;
    PxCookingParams cookingParams(tolerancesScale);
    cookingParams.convexMeshCookingType = PxConvexMeshCookingType::Enum::eQUICKHULL;
    cookingParams.gaussMapLimit = 256;

    //�@���_��������ɍ��v
    size_t totalVertexCount = 0;
    for (const auto& mesh : model->batchMeshes)
    {
        totalVertexCount += mesh.cachedVertices.size();
    }
    // ���_���ɉ����ĕ��򂷂�
    bool use32BitIndex = (totalVertexCount >= 65536);

    // ���_�ƃC���f�b�N�X���\�z
    std::vector<PxVec3> vertices;
    std::vector<PxU32> indices32;
    std::vector<PxU16> indices16;

    PxU32 vertexOffset = 0;

    for (const auto& mesh : model->batchMeshes)
    {
        for (const auto& vertex : mesh.cachedVertices)
        {
            vertices.emplace_back(vertex.position.x * unitScale, vertex.position.y * unitScale, vertex.position.z * unitScale);
        }

        if (use32BitIndex)
        {
            for (const auto& index : mesh.cachedIndices)
            {
                indices32.push_back(index + vertexOffset);
            }
        }
        else
        {
            for (const auto& index : mesh.cachedIndices)
            {
                indices16.push_back(static_cast<PxU16>(index + vertexOffset));
            }
        }

        // ���_���I�t�Z�b�g���݂ŉ��Z
        vertexOffset += static_cast<PxU32>(mesh.cachedVertices.size());
    }

    PxTriangleMeshDesc pxMeshDesc;
    pxMeshDesc.points.count = static_cast<PxU32>(vertices.size());
    pxMeshDesc.points.stride = sizeof(PxVec3);
    pxMeshDesc.points.data = vertices.data();

    if (use32BitIndex)
    {
        pxMeshDesc.triangles.count = static_cast<PxU32>(indices32.size() / 3);
        pxMeshDesc.triangles.stride = 3 * sizeof(PxU32);
        pxMeshDesc.triangles.data = indices32.data();
    }
    else
    {
        pxMeshDesc.triangles.count = static_cast<PxU32>(indices16.size() / 3);
        pxMeshDesc.triangles.stride = 3 * sizeof(PxU16);
        pxMeshDesc.triangles.data = indices16.data();
    }
    pxMeshDesc.flags = PxMeshFlag::e16_BIT_INDICES;

    PxTriangleMesh* triangleMesh = nullptr;
    PxDefaultMemoryOutputStream writeBuffer;
    if (!PxCookTriangleMesh(cookingParams, pxMeshDesc, writeBuffer))
    {
        _ASSERT(L" PxCookTriangleMesh failed.");
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    triangleMesh = physics->createTriangleMesh(readBuffer);

    PxTriangleMeshGeometry geometry(triangleMesh);
    pxShape_ = physics->createShape(geometry, *material_, true);
    pxShape_->userData = owner_;   // MeshComponent �ւ̃|�C���^

    // �Փ˃t�B���^
    PxFilterData filterData(layer_, mask_, 0, 0);
    pxShape_->setSimulationFilterData(filterData);
    pxShape_->setQueryFilterData(filterData);

    pxActor_ = physics->createRigidStatic(PxTransform(PxIdentity));
    pxActor_->userData = owner_->GetOwner();     // Actor �ւ̃|�C���^
    pxActor_->attachShape(*pxShape_);
}