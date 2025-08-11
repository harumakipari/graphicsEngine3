#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

// C++ �W�����C�u����
#include <string>

// �����C�u����
#include <DirectXMath.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

// �v���W�F�N�g�̑��̃w�b�_
#include "Components/Base/SceneComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Utils/EasingHandler.h"

class SpringArmComponent :public SceneComponent
{
public:
    SpringArmComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner)
    {
        armLength = 5.0f;
        targetOffset = { 0.0f,1.0f,0.0f };
    }

    void Tick(float deltaTime)override
    {
        using namespace DirectX;

        if (auto parent = attachParent_.lock())
        {
            // 1. �e�̃��[���h�ʒu�E��]�擾
            //XMFLOAT3 parentWorldPos = parent->GetWorldPosition();
            XMFLOAT3 parentWorldPos = parent->GetRelativeLocation();
            XMVECTOR parentPosVec = XMLoadFloat3(&parentWorldPos);
            XMVECTOR parentRotQuat = XMLoadFloat4(&parent->GetRelativeRotation());
            //XMVECTOR parentRotQuat = XMLoadFloat4(&parent->GetLocalRotation());


            // 
            XMVECTOR localRot = DirectX::XMLoadFloat4(&GetRelativeRotation());
            //XMVECTOR localRot = XMQuaternionRotationRollPitchYaw(
            //    XMConvertToRadians(angleLocal.x),
            //    XMConvertToRadians(angleLocal.y),
            //    XMConvertToRadians(angleLocal.z));

            // �e�̉�]�ƍ���
            XMVECTOR finalRot = XMQuaternionMultiply(localRot, parentRotQuat);

            XMMATRIX rotMatrix = XMMatrixRotationQuaternion(finalRot);



            // �e�̊p�x���m�F���Ĕz�u����J����
            XMVECTOR forward = XMVector3TransformNormal(
                XMVectorSet(0, 0, 1, 0),
                XMMatrixRotationQuaternion(finalRot)
            );

            forward = XMVectorSet(0, 0, 1, 0);  // ���������L�����N�^�[�̉�]�𖳎����Č��ɔz�u����J����

            XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), XMMatrixRotationQuaternion(finalRot));


            XMVECTOR backward = XMVectorScale(forward, -armLength);
            XMVECTOR upward = XMVectorScale(up, targetOffset.y);

            // 3. �ʒu�v�Z
            XMVECTOR camPos = parentPosVec + backward + upward;
            XMFLOAT3 camPosF3;
            XMStoreFloat3(&camPosF3, camPos);

            // 4. ���[���h�ʒu�E��]���f
            //SetWorldPosition(camPosF3);
            //rotationLocal = parent->GetLocalRotation();


            //SetWorldLocationDirect(camPosF3);
            SetRelativeLocationDirect(camPosF3);
            //positionLocal = camPosF3;
            DirectX::XMFLOAT4 rotationLocal;
            XMStoreFloat4(&rotationLocal, finalRot);    // �e�̊p�x���m�F���Ĕz�u����J����
            SetWorldRotationDirect(rotationLocal);
            rotationLocal = { 0.0f,0.0f,0.0f,1.0f };    // ���������L�����N�^�[�̉�]�𖳎����Č��ɔz�u����J����
            SetWorldRotationDirect(rotationLocal);
        }
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  armSpring").c_str()))
        {
            //ImGui::DragFloat3("Position", &positionLocal.x, 0.1f);
            ImGui::DragFloat("armLength", &armLength, 0.1f);
            ImGui::TreePop();
        }
#endif
    }


private:
    float armLength;
    bool enableLag;
    float lagSpeed;
    DirectX::XMFLOAT3 targetOffset;
};

class CameraComponent :public SceneComponent
{
public:
    CameraComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner)
    {
        // �f�t�H���g�p�����[�^
        fovY = DirectX::XMConvertToRadians(45); // 45�x
        aspectRatio = Graphics::GetScreenWidth() / Graphics::GetScreenHeight();
        nearZ = 0.1f;
        farZ = 1000.0f;
    }

    void Tick(float deltaTime)override
    {
        //if (GetAsyncKeyState('F') & 0x8000) {
        //    Shake();
        //}

        handler.Update(power, deltaTime);
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  camera").c_str()))
        {
            //ImGui::DragFloat3("Position", &positionLocal.x, 0.1f);
            ImGui::DragFloat("fovY", &fovY, 0.1f);
            ImGui::DragFloat("nearZ", &nearZ, 0.1f);
            ImGui::DragFloat("farZ", &farZ, 0.1f);
            ImGui::TreePop();
        }
#endif
    }


    // �p�[�X�y�N�e�B�u�ݒ�
    void SetPerspective(float fovY, float aspect, float nearZ, float farZ)
    {
        this->fovY = fovY;
        this->aspectRatio = aspect;
        this->nearZ = nearZ;
        this->farZ = farZ;
    }

    // �r���[�s��擾
    const DirectX::XMFLOAT4X4& GetView();

    //�@�v���W�F�N�V�����s��擾
    const DirectX::XMFLOAT4X4& GetProjection()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ));
        return projection;
    }

    //�J�����V�F�C�N
    void Shake(float power = 0.02f, float time = 0.2f)
    {
        handler.Clear();
        handler.SetEasing(EaseType::Linear, power, 0.f, time);
    }

    //�t�H�[�J�X���蓮�ݒ�ł���悤�ɂ���p
    bool customTarget = false;
    DirectX::XMFLOAT3 _target{};

private:
    float fovY;
    float aspectRatio;
    float nearZ;
    float farZ;

    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};

    EasingHandler handler;
    float power;
};




class DebugCameraComponent :public CameraComponent
{
public:
    DebugCameraComponent(const std::string& name,std::shared_ptr<Actor> owner) :CameraComponent(name, owner) {}

    void Tick(float deltaTime)override
    {
        HandleKeyboardInput(deltaTime);
        HandleMouseInput(deltaTime);
    }


private:
    float moveSpeed = 5.0f;
    float rotateSpeed = 1.0f;

    float yaw = 0.0f;
    float pitch = 0.0f;

    void HandleKeyboardInput(float deltaTime);

    void HandleMouseInput(float deltaTime)
    {
        if (InputSystem::GetInputState("MouseRight"))
        {
            int deltaX, deltaY;
            InputSystem::GetMouseDelta(deltaX, deltaY);

#if 1
            //DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationLocal));
            DirectX::XMFLOAT4 rotaion = GetComponentRotation();
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
            yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
            pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
            pitch = std::clamp(pitch, DirectX::XMConvertToRadians(-89.0f), DirectX::XMConvertToRadians(89.0f));
            DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
            DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), yaw);
            DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
            //DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
            DirectX::XMFLOAT4 r = GetComponentRotation();
            DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&r), Q));
            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotationLocal), Q));
#else
            DirectX::XMVECTOR rot;
            {
                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
                pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
                DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
            }
            {
                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot);
                yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
                DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(R.r[1], yaw);
                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(rot, quatYaw));
            }
#endif // 0

            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
            //rot= DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), rot));
            //R = DirectX::XMMatrixRotationQuaternion(rot);
            ////R.r[2] = {};
            //rot = DirectX::XMQuaternionRotationMatrix(R);
            DirectX::XMFLOAT4 rotationLocal{};
            DirectX::XMStoreFloat4(&rotationLocal, rot);
            SetWorldRotationDirect(rotationLocal);
        }
    }
};


#endif //CAMERA_COMPONENT_H