#ifndef TRANSFORM_H
#define TRANSFORM_H

// �����C�u����
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>

// �v���W�F�N�g�̑��̃w�b�_
#include "Math/MathHelper.h"

struct alignas(16) Transform
{
    DirectX::XMVECTOR translation_;
    DirectX::XMVECTOR rotation_;	// �N�H�[�^�j�I��
    DirectX::XMVECTOR scale_;

    Transform() : translation_(DirectX::XMVectorZero()), rotation_(DirectX::XMQuaternionIdentity()), scale_(DirectX::XMVectorSet(1, 1, 1, 0))
    {
    }
    Transform(DirectX::XMFLOAT3 translation, DirectX::XMFLOAT3 euler_rotation, DirectX::XMFLOAT3 scale) :
        translation_(DirectX::XMLoadFloat3(&translation)),
        rotation_(DirectX::XMQuaternionRotationRollPitchYaw(euler_rotation.x, euler_rotation.y, euler_rotation.z)),
        scale_(DirectX::XMLoadFloat3(&scale))
    {
    }
    Transform(DirectX::XMFLOAT3 translation, DirectX::XMFLOAT4 rotation/*quaternion*/, DirectX::XMFLOAT3 scale) :
        translation_(DirectX::XMLoadFloat3(&translation)),
        rotation_(DirectX::XMLoadFloat4(&rotation)),
        scale_(DirectX::XMLoadFloat3(&scale))
    {
    }
    Transform(DirectX::XMVECTOR translation, DirectX::XMVECTOR rotation/*quaternion*/, DirectX::XMVECTOR scale) :
        translation_(translation),
        rotation_(rotation),
        scale_(scale)
    {
    }
    Transform(DirectX::XMMATRIX matrix)
    {
        DirectX::XMMatrixDecompose(&scale_, &rotation_, &translation_, matrix);
    }

    DirectX::XMFLOAT3 GetTranslation() const
    {
        DirectX::XMFLOAT3 translation;
        DirectX::XMStoreFloat3(&translation, translation_);
        return translation;
    }
    DirectX::XMFLOAT3 GetLocation() const
    {
        return GetTranslation();
    }
    void SetTranslation(DirectX::XMFLOAT3 translation)
    {
        translation_ = DirectX::XMLoadFloat3(&translation);
    }
    DirectX::XMFLOAT3 GetScale() const
    {
        DirectX::XMFLOAT3 scale;
        DirectX::XMStoreFloat3(&scale, scale_);
        return scale;
    }
    void SetScale(DirectX::XMFLOAT3 scale)
    {
        scale_ = DirectX::XMLoadFloat3(&scale);
    }

    DirectX::XMFLOAT4 GetRotation() const
    {
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, rotation_);
        return rotation;
    }
    void SetRotation(DirectX::XMFLOAT4 rotation) 
    {
        rotation_ = DirectX::XMLoadFloat4(&rotation);
    }
    DirectX::XMFLOAT3 GetEulerRotation() const  
    {
        DirectX::XMFLOAT4 rotationQuaternion;
        DirectX::XMStoreFloat4(&rotationQuaternion, rotation_);
        return MathHelper::QuaternionToEuler(rotationQuaternion);
    }
    void SetEularRotation(const DirectX::XMFLOAT3& eulerRotation)
    {
        rotation_ = DirectX::XMQuaternionRotationRollPitchYaw(eulerRotation.x, eulerRotation.y, eulerRotation.z);
    }

    // �l��Nan�������Ă��Ȃ����m�F
    bool ContainsNanOrInfinite() const
    {
        return MathHelper::VectorContainsNanOrInfinite(translation_) || MathHelper::VectorContainsNanOrInfinite(scale_) || MathHelper::VectorContainsNanOrInfinite(rotation_);
    }

    // transform�@�� matrix �ɕϊ�
    DirectX::XMMATRIX ToMatrix() const
    {
        _ASSERT_EXPR(!ContainsNanOrInfinite(), L"Invalid transform : one or more components (translation, rotation, or scale) contain NaN.");
        return DirectX::XMMatrixScalingFromVector(scale_) * DirectX::XMMatrixRotationQuaternion(rotation_) * DirectX::XMMatrixTranslationFromVector(translation_);
    }

    // transform�@�� worldTransform �ɕϊ�
    DirectX::XMFLOAT4X4 ToWorldTransform() const 
    {
        DirectX::XMFLOAT4X4 worldTransform{};
        _ASSERT_EXPR(!ContainsNanOrInfinite(), L"Invalid transform : one or more components (translation, rotation, or scale) contain NaN.");
        DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixScalingFromVector(scale_) * DirectX::XMMatrixRotationQuaternion(rotation_) * DirectX::XMMatrixTranslationFromVector(translation_));
        return worldTransform;

    }

    // Transform ���m���|���Z����
    Transform operator*(const Transform& rhs) const
    {
        Transform composed;
        // �p���̊|���Z���@transform�Ɋi�[����
        DirectX::XMMatrixDecompose(&composed.scale_, &composed.rotation_, &composed.translation_, ToMatrix() * rhs.ToMatrix());
        return composed;
    }

    // �����̎p�����玩�g�̑��ΓI�ȍ��W���擾����
    //Transform GetRelativeTransform(const Transform& other) const
    //{
    //    
    //}

    // ����Transform�Ɣ�r
    bool Equals(const Transform& other, float tolerance) const
    {
        DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(tolerance);
        return DirectX::XMVector3NearEqual(translation_, other.translation_, epsilon)
            && DirectX::XMVector4NearEqual(rotation_, other.rotation_, epsilon)
            && DirectX::XMVector3NearEqual(scale_, other.scale_, epsilon);
    }

};

#endif //TRANSFORM_H