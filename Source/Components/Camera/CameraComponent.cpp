#include "CameraComponent.h"

#include "Core/Actor.h"

// ビュー行列取得
const DirectX::XMFLOAT4X4& CameraComponent::GetView()
{
#if 1
    using namespace DirectX;
    // クォータニオンから回転行列
    XMMATRIX rotationMatrix{};
    XMVECTOR eye{};
    //XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rotationMatrix);
    if (auto parent = attachParent_.lock())
    {
        rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&parent->GetRelativeRotation()));
        DirectX::XMFLOAT3 e = parent->GetRelativeLocation();
        //e = attachParent_.lock()->GetLocalPosition();
        //rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&attachParent_.lock()->GetLocalRotation()));
        //DirectX::XMFLOAT3 e = attachParent_.lock()->GetWorldPosition();
        //e = attachParent_.lock()->GetLocalPosition();
        eye = XMLoadFloat3(&e);
    }
    else
    {
        rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&GetRelativeRotation()));
        DirectX::XMFLOAT3 pos = GetRelativeLocation();
        eye = XMLoadFloat3(&pos);
    }
    // 前方向ベクトルを回転
    XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rotationMatrix);
    XMVECTOR up = /*XMVector3TransformNormal(*/XMVectorSet(0, 1, 0, 0)/*, rotationMatrix)*/;


    XMVECTOR target = eye + forward;
    
    if (customTarget) 
    {
        target = XMLoadFloat3(&_target);
    }
    
    //カメラシェイク処理
    if (handler.GetSequenceCount() > 0)
    {
        XMVECTOR right = XMVector3Cross(up, forward);

        float x = ((rand() / static_cast<float>(RAND_MAX)) - 0.5f) * power;
        float y = ((rand() / static_cast<float>(RAND_MAX)) - 0.5f) * power;
        target += (right * x) + (up * y);
    }

    XMMATRIX ViewMatrix = XMMatrixLookAtLH(eye, target, up);

    XMStoreFloat4x4(&view, ViewMatrix);
    return view;
#else
    using namespace DirectX;

    XMMATRIX mat = XMLoadFloat4x4(&GetWorldTransform());

    // カメラ位置と向きからビュー行列作成
    XMVECTOR eyePos = XMVector3TransformCoord(XMVectorZero(), mat);
    XMVECTOR lookDir = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), mat); // Z+方向
    XMVECTOR lookAt = XMVectorAdd(eyePos, lookDir);
    XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), mat);

    XMStoreFloat4x4(&view, XMMatrixLookAtLH(eyePos, lookAt, up));
    return view;
#endif
}

void DebugCameraComponent::HandleKeyboardInput(float deltaTime)
{
    using namespace DirectX;
    DirectX::XMFLOAT4 rotaion = GetComponentRotation();
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
    DirectX::XMVECTOR forward = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), rotationMatrix);
    //DirectX::XMVECTOR right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), rotationMatrix);
    //DirectX::XMVECTOR up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), rotationMatrix);
    DirectX::XMVECTOR right = rotationMatrix.r[0];
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

    DirectX::XMVECTOR move = DirectX::XMVectorZero();

    if (InputSystem::GetInputState("W")) { move += forward; }
    if (InputSystem::GetInputState("S")) { move -= forward; }
    if (InputSystem::GetInputState("D")) { move += right; }
    if (InputSystem::GetInputState("A")) { move -= right; }
    //
    if (InputSystem::GetInputState("E")) { move += up; }
    if (InputSystem::GetInputState("Q")) { move -= up; }

    if (InputSystem::GetInputState("Shift")) { move = DirectX::XMVectorScale(move, 2.5f); }

    move = DirectX::XMVectorScale(move, moveSpeed * deltaTime);

    //DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&positionLocal);
    //pos += move;
    //DirectX::XMStoreFloat3(&positionLocal, pos);
    DirectX::XMFLOAT3 position = GetComponentLocation();
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos += move;
    DirectX::XMFLOAT3 positionLocal{};
    DirectX::XMStoreFloat3(&positionLocal, pos);

    GetOwner()->SetPosition(positionLocal);
    //SetWorldLocationDirect(positionLocal);
}
