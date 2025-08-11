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

    // 使用
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

    // アニメーションの再生倍率を変更する関数
    void SetAnimationRate(float animationRate)
    {
        if (animationController_)
        {
            animationController_->SetAnimationRate(animationRate);
        }
    }

    int GetHp() const { return hp; }

    //進行方向の単位ベクトルを取得する
    const DirectX::XMFLOAT3& GetForward()
    {
        // Z軸方向の単位方向ベクトル　デフォルト
        DirectX::XMVECTOR DefaultForward = DirectX::XMVectorSet(0, 0, 1, 0);
        //playerの回転値によって作られる回転行列
        DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&GetQuaternionRotation()));
        //DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(GetQuaternionRotation().x, GetQuaternionRotation().y, GetQuaternionRotation().z);
        // デフォルトのベクトルに回転行列を適応する
        DirectX::XMVECTOR TransformedForward = DirectX::XMVector3TransformNormal(DefaultForward, RotationMatrix);
        //正規化
        TransformedForward = DirectX::XMVector3Normalize(TransformedForward);

        DirectX::XMStoreFloat3(&front, TransformedForward);
        return front;
    }



    //床へレイキャストを飛ばす
    bool CheckCollisionFloor(const CollisionMesh* collisionMesh, DirectX::XMFLOAT4X4 transform)
    {
        //DirectX::XMFLOAT4 intersection;
        std::string mesh;
        std::string material;

        //ステージ床へレイキャストを飛ばす
        DirectX::XMFLOAT3 rayPosition{ GetPosition().x,GetPosition().y + 1.0f,GetPosition().z };
        //DirectX::XMFLOAT3 rayDirection{ 0,velocity.y,0};   //例の長さ
        DirectX::XMFLOAT3 rayDirection{ 0, -1, 0 };   //例の長さ
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

    // 入力をオンにするか
    virtual bool CanMove() { return true; }

    //進行方向にレイを飛ばす
    bool RaycastForward(const CollisionMesh* collisionMesh, DirectX::XMFLOAT4X4 transform)
    {
#if 0
        //DirectX::XMFLOAT4 intersection;
        std::string mesh;
        std::string material;

        //進行方向にへレイキャストを飛ばす
        DirectX::XMFLOAT3 rayPosition{ position.x,position.y + height * 0.5f,position.z };   //背丈の真ん中から
        //DirectX::XMFLOAT3 rayDirection = GetForward();   //レイの向き
        DirectX::XMFLOAT3 rayDirection = { velocity.x,0.0f,velocity.z };   //レイの長さ
        DirectX::XMFLOAT3 intersectionPosition;
        DirectX::XMFLOAT3 intersectionNormal;
        std::string intersectionMesh;
        std::string intersectionMaterial;

        if (fabs(velocity.x - FLT_EPSILON) < 0.0f && fabs(velocity.z - FLT_EPSILON) < 0.0f)
        {// velocity.x velocity.zがどちらも 0 だったら
            return false;
        }
        DirectX::XMVECTOR RayDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&rayDirection));
        DirectX::XMStoreFloat3(&rayDirection, RayDirection);

        //レイの長さを適切な値に変更する
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
    //高さ
    float height = 0.0f;    //m単位

    //重さ
    float mass = 0.0f;      //kg単位

    //半径
    float radius = 0.0f;    //m単位

    //速度
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };

    //地面の上にいるか
    bool isGround = false;

    //重力
    float gravity = -9.8f;
protected:
    size_t animationIndex = 0;

    int hp = 0;

    //前方向ベクトル
    DirectX::XMFLOAT3 front{ 0.0f,0.0f,1.0f/*,0.0f*/ };

    //最大回転値
    float maxTurningSpeed = 360.0f;

    //回転速度　degree基準
    float turningSpeed = 0.0f;

    //レイを飛ばしたときにステージと当たる座標
    DirectX::XMFLOAT3  intersectStagePosition{ 0.0f,0.0f,0.0f };

    // アニメーションコントローラー
    std::shared_ptr<AnimationController> animationController_;

private:

};