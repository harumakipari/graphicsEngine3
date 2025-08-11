#pragma once
#include <memory>
#include "Core/Actor.h"
#include "Graphics/Resource/GeometricPrimitive.h"
#include "Physics/CollisionMesh.h"

#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/CollisionComponent.h"
#include "Animation/AnimationController.h"

class Character :public Actor
{
public:
    Character() = default;
    virtual ~Character() = default;

    Character(const std::string& modelName) :Actor(modelName)
    {
    }


    virtual void Update(float deltaTime)override
    {
        if (animationController_)
        {
            animationController_->OnUpdate(deltaTime);
        }
    };

    void SetAnimationController(std::shared_ptr<AnimationController> controller)
    {
        animationController_ = controller;
    }

    std::shared_ptr<AnimationController> GetAnimationController()
    {
        return animationController_;
    }

    // �g�p
    void PlayAnimation(const std::string& name, bool loop = true, bool blend = true, float blendTime = 0.3f)
    {
        if (animationController_)
        {
            animationController_->SetAnimationClip(name, loop, blend, blendTime);
        }
    }

    void StopAnimation()
    {
        if (animationController_)
        {
            animationController_->Stop();
        }
    }

    virtual void LateUpdate(float elapsedTime) {};

    virtual void Move(float elapsedTime) {}

    void SetAnimationIndex(size_t index) { animationIndex = index; }

    size_t GetAnimationIndex()const override { return animationIndex; }

    // �A�j���[�V�����̍Đ��{����ύX����֐�
    void SetAnimationRate(float animationRate)
    {
        if (animationController_)
        {
            animationController_->SetAnimationRate(animationRate);
        }
    }

    int GetHp() const { return hp; }

    //�i�s�����̒P�ʃx�N�g�����擾����
    const DirectX::XMFLOAT3& GetForward()
    {
        // Z�������̒P�ʕ����x�N�g���@�f�t�H���g
        DirectX::XMVECTOR DefaultForward = DirectX::XMVectorSet(0, 0, 1, 0);
        //player�̉�]�l�ɂ���č�����]�s��
        DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&GetQuaternionRotation()));
        //DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(GetQuaternionRotation().x, GetQuaternionRotation().y, GetQuaternionRotation().z);
        // �f�t�H���g�̃x�N�g���ɉ�]�s���K������
        DirectX::XMVECTOR TransformedForward = DirectX::XMVector3TransformNormal(DefaultForward, RotationMatrix);
        //���K��
        TransformedForward = DirectX::XMVector3Normalize(TransformedForward);

        DirectX::XMStoreFloat3(&front, TransformedForward);
        return front;
    }



    //���փ��C�L���X�g���΂�
    bool CheckCollisionFloor(const CollisionMesh* collisionMesh, DirectX::XMFLOAT4X4 transform)
    {
        //DirectX::XMFLOAT4 intersection;
        std::string mesh;
        std::string material;

        //�X�e�[�W���փ��C�L���X�g���΂�
        DirectX::XMFLOAT3 rayPosition{ GetPosition().x,GetPosition().y + 1.0f,GetPosition().z };
        //DirectX::XMFLOAT3 rayDirection{ 0,velocity.y,0};   //��̒���
        DirectX::XMFLOAT3 rayDirection{ 0, -1, 0 };   //��̒���
        /*DirectX::XMFLOAT3 intersectionPosition;*/
        DirectX::XMFLOAT3 intersectionNormal;
        std::string intersectionMesh;
        std::string intersectionMaterial;

        if (collisionMesh->Raycast(rayPosition, rayDirection, transform, intersectStagePosition, intersectionNormal, intersectionMesh, intersectionMaterial))
        {
            float playerHeight = 0.0f;
            using namespace DirectX;
            DirectX::XMFLOAT3 pos = GetPosition();
            float d0 = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pos) - XMLoadFloat3(&rayPosition)));
            float d1 = XMVectorGetX(XMVector3Length(XMLoadFloat3(&intersectStagePosition) - XMLoadFloat3(&rayPosition)));
            if (d0 + playerHeight > d1)
            {
                XMFLOAT3 outPosition = GetPosition();
                float d = (d0 + playerHeight) - d1;
                outPosition.x -= d * rayDirection.x;
                outPosition.y -= d * rayDirection.y;
                outPosition.z -= d * rayDirection.z;
                SetPosition(outPosition);
                // Reflection
                velocity.y = 0.0f;
                //isGround = true;
                return true;
            }
        }
        return false;
    }

    // ���͂��I���ɂ��邩
    virtual bool CanMove() { return true; }

    //�i�s�����Ƀ��C���΂�
    bool RaycastForward(const CollisionMesh* collisionMesh, DirectX::XMFLOAT4X4 transform)
    {
#if 0
        //DirectX::XMFLOAT4 intersection;
        std::string mesh;
        std::string material;

        //�i�s�����ɂփ��C�L���X�g���΂�
        DirectX::XMFLOAT3 rayPosition{ position.x,position.y + height * 0.5f,position.z };   //�w��̐^�񒆂���
        //DirectX::XMFLOAT3 rayDirection = GetForward();   //���C�̌���
        DirectX::XMFLOAT3 rayDirection = { velocity.x,0.0f,velocity.z };   //���C�̒���
        DirectX::XMFLOAT3 intersectionPosition;
        DirectX::XMFLOAT3 intersectionNormal;
        std::string intersectionMesh;
        std::string intersectionMaterial;

        if (fabs(velocity.x - FLT_EPSILON) < 0.0f && fabs(velocity.z - FLT_EPSILON) < 0.0f)
        {// velocity.x velocity.z���ǂ���� 0 ��������
            return false;
        }
        DirectX::XMVECTOR RayDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&rayDirection));
        DirectX::XMStoreFloat3(&rayDirection, RayDirection);

        //���C�̒�����K�؂Ȓl�ɕύX����
        rayDirection.x *= 1.5f;
        rayDirection.z *= 1.5f;

        if (collisionMesh->Raycast(rayPosition, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
        {
            float playerHeight = 0.0f;
            using namespace DirectX;
            float d0 = XMVectorGetX(XMVector3Length(XMLoadFloat3(&position) - XMLoadFloat3(&rayPosition)));
            float d1 = XMVectorGetX(XMVector3Length(XMLoadFloat3(&intersectionPosition) - XMLoadFloat3(&rayPosition)));
            if (d0 + playerHeight > d1)
            {
                XMFLOAT3 outPosition = position;
                float d = (d0 + playerHeight) - d1;
                outPosition.x -= d * rayDirection.x;
                //outPosition.y -= d * rayDirection.y;
                outPosition.z -= d * rayDirection.z;
                position = outPosition;
                // Reflection
                if (velocity.y < 0.0f)
                {
                    XMStoreFloat3(&velocity, XMVector3Reflect(XMLoadFloat3(&velocity), XMLoadFloat3(&intersectionNormal)));
                }

                //velocity.x = velocity.y = velocity.z = 0.0f;
                return true;
            }
        }
#endif
        return false;
    }
public:
    //����
    float height = 0.0f;    //m�P��

    //�d��
    float mass = 0.0f;      //kg�P��

    //���a
    float radius = 0.0f;    //m�P��

    //���x
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };

    //�n�ʂ̏�ɂ��邩
    bool isGround = false;

    //�d��
    float gravity = -9.8f;
protected:
    size_t animationIndex = 0;

    int hp = 0;

    //�O�����x�N�g��
    DirectX::XMFLOAT3 front{ 0.0f,0.0f,1.0f/*,0.0f*/ };

    //�ő��]�l
    float maxTurningSpeed = 360.0f;

    //��]���x�@degree�
    float turningSpeed = 0.0f;

    //���C���΂����Ƃ��ɃX�e�[�W�Ɠ�������W
    DirectX::XMFLOAT3  intersectStagePosition{ 0.0f,0.0f,0.0f };

    // �A�j���[�V�����R���g���[���[
    std::shared_ptr<AnimationController> animationController_;

private:

};