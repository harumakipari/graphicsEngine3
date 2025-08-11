#ifndef PHYSICS_HELPER_H
#define PHYSICS_HELPER_H

#include <string>

// �����C�u����
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>

// �v���W�F�N�g�̑��̃w�b�_
#include "Components/Transform/Transform.h"

namespace PhysicsHelper
{
    // �ϊ�: Transform -> PxTransform
    inline physx::PxTransform ToPxTransform(const Transform& t)
    {
        using namespace physx;
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, t.translation_);

        DirectX::XMFLOAT4 rot;
        DirectX::XMStoreFloat4(&rot, t.rotation_);

        return PxTransform(PxVec3(pos.x, pos.y, pos.z), PxQuat(rot.x, rot.y, rot.z, rot.w));
    }

    // �ϊ�: PxTransform -> Transform
    inline Transform FromPxTransform(const physx::PxTransform& p)
    {
        // �ʒu�Ɖ�]���Z�b�g�A�X�P�[���͕����G���W���ł͈���Ȃ����� (1,1,1) ���g�p
        return Transform(
            DirectX::XMVectorSet(p.p.x, p.p.y, p.p.z, 0.0f),
            DirectX::XMVectorSet(p.q.x, p.q.y, p.q.z, 0.0f),
            DirectX::XMVectorSet(1.0f, 1.0f, 1.0, 0.0f)
        );
    }

    // �ϊ�: DirectX::XMFLOAT4X4 -> PxTransform
    inline physx::PxTransform ToPxTransform(const DirectX::XMFLOAT4X4& mat)
    {
        using namespace DirectX;

        XMMATRIX Mat = XMLoadFloat4x4(&mat);

        XMVECTOR scale;
        XMVECTOR rotationQuat;
        XMVECTOR translation;

        // Matrix ���� XmVector ���쐬
        XMMatrixDecompose(&scale, &rotationQuat, &translation, Mat);

        XMFLOAT3 pos;
        XMFLOAT4 rot;
        XMStoreFloat3(&pos, translation);
        XMStoreFloat4(&rot, rotationQuat);

        return physx::PxTransform(
            physx::PxVec3(pos.x, pos.y, pos.z),
            physx::PxQuat(rot.x, rot.y, rot.z, rot.w)
        );
    }

    inline void DebugPrintCollisionInfo(const std::string& name, const physx::PxGeometryHolder& geometry, const physx::PxTransform& transform)
    {
        using namespace physx;
        // �`��^�C�v�擾
        const char* shapeType = "Unknown";
        switch (geometry.getType())
        {
        case PxGeometryType::eSPHERE:        shapeType = "Sphere"; break;
        case PxGeometryType::ePLANE:         shapeType = "Plane"; break;
        case PxGeometryType::eCAPSULE:       shapeType = "Capsule"; break;
        case PxGeometryType::eBOX:           shapeType = "Box"; break;
        case PxGeometryType::eCONVEXMESH:    shapeType = "ConvexMesh"; break;
        case PxGeometryType::eTRIANGLEMESH:  shapeType = "TriangleMesh"; break;
        case PxGeometryType::eHEIGHTFIELD:   shapeType = "HeightField"; break;
        default:                             break;
        }

        // �ʒu�Ɖ�]
        const PxVec3& p = transform.p;
        const PxQuat& q = transform.q;

        // �o��
        char buffer[512];
        sprintf_s(buffer, sizeof(buffer),
            "[%s] Type: %s | Pos: (%.2f, %.2f, %.2f) | Rot (quat): (%.2f, %.2f, %.2f, %.2f)\n",
            name.c_str(), shapeType,
            p.x, p.y, p.z,
            q.x, q.y, q.z, q.w
        );
        OutputDebugStringA(buffer);
    }

}
#endif //PHYSICS_HELPER_H
