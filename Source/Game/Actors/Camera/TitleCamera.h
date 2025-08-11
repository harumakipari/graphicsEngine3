#ifndef TITLE_CAMERA_H
#define TITLE_CAMERA_H

#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Components/Camera/CameraComponent.h"
#include "Game/Actors/Camera/Camera.h"

class TitleCamera :public Camera
{
public:
    //引数付きコンストラクタ
    TitleCamera(std::string actorName) :Camera(actorName) {}

    virtual ~TitleCamera() = default;
    //std::shared_ptr<CameraComponent> mainCameraComponent;


    void Initialize()override
    {
        mainCameraComponent = this->NewSceneComponent<class CameraComponent>("mainCamera");
        mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(35), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
        SetPosition(DirectX::XMFLOAT3(-0.13f, 1.2f, -4.3f));
        SetEulerRotation(DirectX::XMFLOAT3(1.4f, 20.6f, 0.0f));
    };

    //更新処理
    void Update(float deltaTime)override
    {
        using namespace DirectX;
#if 0

        XMFLOAT3 forward = enemy->GetForward();
        DirectX::XMFLOAT3 focus = enemy->bossJointComponent->GetComponentWorldTransform().GetLocation();
        XMVECTOR Focus = XMLoadFloat3(&focus);

        XMVECTOR Forward = XMVector3Normalize(XMLoadFloat3(&forward));
        XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
        XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Forward, Up));

        XMVECTOR Eye = Focus + (Right * -easeX) + (Forward * easeZ);
        XMFLOAT3 eye;
        XMStoreFloat3(&eye, Eye);

        if (abs(focus.x - eye.x) < 0.001f && abs(focus.z - eye.z) < 0.001f)
        {
            Eye += Forward * distance;
            XMStoreFloat3(&eye, Eye);
        }
        eye.y = easeY;

        mainCameraComponent->customTarget = true;
        mainCameraComponent->_target = focus;
        SetPosition(eye);
#else
        //if (isClickButton)
        //{
        //    XMFLOAT3 playerPos = { 0.0f, 0.4f, 0.0f };
        //    XMFLOAT3 enemyPos = { 6.7f, 0.0f, 5.6f };

        //    XMFLOAT3 center;
        //    center.x = (playerPos.x + enemyPos.x) * 0.5f;
        //    center.y = (playerPos.y + enemyPos.y) * 0.5f;
        //    center.z = (playerPos.z + enemyPos.z) * 0.5f;
        //    SetTarget(center);
        //    center.y += offset.y;  // 必要なら高さも調整
        //    center.x += offset.x;
        //    center.z += offset.z;

        //    XMVECTOR p = XMLoadFloat3(&playerPos);
        //    XMVECTOR e = XMLoadFloat3(&enemyPos);
        //    XMVECTOR toEnemy = XMVector3Normalize(e - p);

        //    XMVECTOR up = XMVectorSet(0, 1, 0, 0);

        //    XMVECTOR right = XMVector3Normalize(XMVector3Cross(toEnemy, up));

        //    XMVECTOR eyeVec = XMLoadFloat3(&center) - right * distance;
        //    XMFLOAT3 eye;
        //    XMStoreFloat3(&eye, eyeVec);
        //    eye.x += offset2.x;
        //    eye.y += offset2.y;
        //    eye.z += offset2.z;
        //    SetPosition(eye);

        //    XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&center) - eyeVec);
        //    XMMATRIX lookAt = XMMatrixLookToLH(eyeVec, dir, up);
        //    XMVECTOR rotQuat = XMQuaternionRotationMatrix(lookAt);
        //    XMFLOAT4 q;
        //    XMStoreFloat4(&q, rotQuat);
        //    SetQuaternionRotation(q);

        //    mainCameraComponent->customTarget = true;
        //    mainCameraComponent->_target = center;
        //}

        //SetEulerRotation(rotation);

        //DirectX::XMFLOAT3 eye = GetPosition();
        //DirectX::XMVECTOR backDir = XMVectorSet(0, 0, -1, 0); // 後ろ向き
        //DirectX::XMMATRIX Rot = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&GetQuaternionRotation()));
        //backDir = XMVector3TransformNormal(backDir, Rot);
        //backDir = XMVector3Normalize(backDir);

        //DirectX::XMVECTOR targetVec = XMLoadFloat3(&target);
        //DirectX::XMVECTOR eyePos = XMVectorMultiplyAdd(backDir, XMVectorReplicate(distance), targetVec);

        //DirectX::XMStoreFloat3(&eye, eyePos);
        //SetPosition(eye);
#endif // 0
    }

    void OnClick()
    {
        isClickButton = true;
    }

    void SetTarget(DirectX::XMFLOAT3 target)
    {
        this->target = target;
    }

    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI
        ImGui::DragFloat3("offset", &offset.x, 0.3f);
        ImGui::DragFloat3("offset2", &offset2.x, 0.3f);
        ImGui::DragFloat3("cameraTarget", &target.x, 0.3f);
        ImGui::DragFloat("distance", &distance, 0.3f);
#endif
    }

private:
    DirectX::XMFLOAT3 target = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 offset = { 0.0f,1.2f,0.0f };
    DirectX::XMFLOAT3 offset2 = { 0.0f,1.2f,0.0f };
    float distance = 10.0f;
    bool isClickButton = false;
};

#endif // !TITLE_CAMERA_H
