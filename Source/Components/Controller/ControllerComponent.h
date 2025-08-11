#ifndef CONTROLLER_COMPONENT_H
#define CONTROLLER_COMPONENT_H

// C++ 標準ライブラリ
#include <memory>
#include <unordered_map>
#include <string>

// 他ライブラリ
#include <DirectXMath.h>

// プロジェクトの他のヘッダ
#include "Components/Base/Component.h"
#include "Components/Base/SceneComponent.h"
#include "Engine/Input/GamePad.h"
#include "Engine/Input/InputSystem.h"

class Actor;


class InputComponent :public SceneComponent
{
public:
    InputComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    void Tick(float deltaTime)override
    {
        static DirectX::XMFLOAT3 position{ 0.0f,0.0f,0.0f };
        static DirectX::XMFLOAT3 angle{ 0.0f,0.0f,0.0f };

        DirectX::XMFLOAT3 inputDir = { 0.0f,0.0f,0.0f };

        if (!CameraManager::IsUseDebug())
        {
            if (InputSystem::GetInputState("W"))
            {
                inputDir.z += 1.0f;
                //position.z += 2.0f * deltaTime;
            }
            if (InputSystem::GetInputState("S"))
            {
                inputDir.z -= 1.0f;
                //position.z -= 2.0f * deltaTime;
            }
            if (InputSystem::GetInputState("D"))
            {
                inputDir.x += 1.0f;
                //position.x += 2.0f * deltaTime;
            }
            if (InputSystem::GetInputState("A"))
            {
                inputDir.x -= 1.0f;
                //position.x -= 2.0f * deltaTime;
            }
        }

        moveInput_ = inputDir;

        //attachParent_.lock()->SetLocalPosition(position);

        for (const std::pair<GamePad::Button, ButtonBinding>& binding : buttonToActionMap_)
        {
            if (pad.ButtonState(binding.first, binding.second.triggerMode))
            {
                Trigger(binding.second.actionName);
            }
        }
    }

    void SetMoveInput(const DirectX::XMFLOAT3& input) { moveInput_ = input; }

    const DirectX::XMFLOAT3& GetMoveInput() const { return moveInput_; }

    // アクション名で起こるイベントを設定
    //使用例
    // inputComponent->BindAction("Jump",[](){/*　処理　*/ });
    void BindAction(const std::string& action, std::function<void()> callback)
    {
        bindings_[action] = callback;
    }

    // ボタンとアクションを紐づける
    //使用例
    // inputComponent->BindActionAndButton(GamePad::Button::x,"Jump",TriggerMode::fallingEdge);
    void BindActionAndButton(GamePad::Button button, const std::string& action, TriggerMode triggerMode)
    {
        buttonToActionMap_[button] = { action,triggerMode };
    }

    void Trigger(const std::string& action)
    {
        auto it = bindings_.find(action);
        if (it != bindings_.end())
        {
            it->second();
        }
    }

    // [←]:-1   [→]:+1
    float GetTumbStateLx()
    {
        return pad.ThumbStateLx();
    }
    // [↑]:+1  [↓]:-1
    float GetTumbStateLy()
    {
        return pad.ThumbStateLy();
    }
    // [a]:-1   [d]:+1
    float GetThumbStateRx()
    {
        return pad.ThumbStateRx();
    }
    // [w]:+1  [s]:-1
    float GetThumbStateRy()
    {
        return pad.ThumbStateRy();
    }


private:
    struct ButtonBinding
    {
        std::string actionName;
        TriggerMode triggerMode;
    };

    // 入力によって起こるイベントを登録する
    std::unordered_map<std::string, std::function<void()>> bindings_;

    // キー入力によって起こるアクションを設定する
    std::unordered_map<GamePad::Button, ButtonBinding> buttonToActionMap_;

    GamePad pad;

    DirectX::XMFLOAT3 moveInput_ = { 0.0f,0.0f,0.0f };

};

class MovementComponent :public SceneComponent
{
public:
    MovementComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    void Tick(float deltaTime)override;

    void SetSpeed(float s) { speed_ = s; }

    float GetSpeed() const { return speed_; }

    void SetInputDelta(const DirectX::XMFLOAT3& delta) { inputDelta_ = delta; }

    const DirectX::XMFLOAT3& GetInputDelta() const { return inputDelta_; }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  speed").c_str()))
        {
            ImGui::DragFloat("speed", &speed_);
            ImGui::TreePop();
        }
#endif
    }

private:
    float speed_ = 5.0f;    // m/s
    DirectX::XMFLOAT3 velocity_ = { 0.0f,0.0f,0.0f };

    DirectX::XMFLOAT3 inputDelta_; // 今フレームの移動量
};

class MovementComponentOutInput :public SceneComponent
{
public:
    MovementComponentOutInput(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    void Tick(float deltaTime)override;

    void SetSpeed(float s) { speed_ = s; }

    float GetSpeed() const { return speed_; }

    void SetInputDelta(const DirectX::XMFLOAT3& delta) { inputDelta_ = delta; }

    void SetVelocity(const DirectX::XMFLOAT3& velocity) { this->velocity_ = velocity;}

    DirectX::XMFLOAT3 GetVelocity() { return velocity_; }

    const DirectX::XMFLOAT3& GetInputDelta() const { return inputDelta_; }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  speed").c_str()))
        {
            ImGui::DragFloat("speed", &speed_);
            ImGui::TreePop();
        }
#endif
    }

private:
    float speed_ = 2.0f;    // m/s
    DirectX::XMFLOAT3 velocity_ = { 0.0f,0.0f,0.0f };

    DirectX::XMFLOAT3 inputDelta_; // 今フレームの移動量
};

class RotationComponent :public SceneComponent
{
public:
    RotationComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    void SetDirection(const DirectX::XMFLOAT3& dir);

    void Tick(float deltaTime)override;

    void SetRotateTime(float t) { this->rotateTime_ = t; }
private:
    // t: 補間率（0.0?1.0）
    DirectX::XMFLOAT4 SlerpQuaternion(const DirectX::XMFLOAT4& current, const DirectX::XMFLOAT4& target, float t)
    {
        using namespace DirectX;

        XMVECTOR q1 = XMLoadFloat4(&current);
        XMVECTOR q2 = XMLoadFloat4(&target);
        XMVECTOR result = XMQuaternionSlerp(q1, q2, t);

        XMFLOAT4 out;
        XMStoreFloat4(&out, result);
        return out;
    }

    bool IsSameDirection(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        constexpr float epsilon = 0.001f;
        return std::abs(a.x - b.x) < epsilon &&
            std::abs(a.y - b.y) < epsilon &&
            std::abs(a.z - b.z) < epsilon;
    }

    DirectX::XMFLOAT3 direction_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 previousDirection_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 startAngle_ = { 0.0f,0.0f,0.0f};  // degree
    DirectX::XMFLOAT4 targetRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    DirectX::XMFLOAT4 startRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    float lerpTime_ = 0.0f;
    float rotateTime_ = 0.3f;    // 3秒で rotation する
    float rotateSpeed_ = 10.0f; // ( degree / second )
};


class RotationTestComponent :public SceneComponent
{
public:
    RotationTestComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    void SetDirection(const DirectX::XMFLOAT3& dir);

    void SetMoveVector(DirectX::XMFLOAT3 moveVec) { this->moveVec_ = moveVec; }
    void SetForward(DirectX::XMFLOAT3 forward) { this->forward_ = forward; }
    void Tick(float deltaTime)override
    {
        DirectX::XMVECTOR MoveVec = DirectX::XMLoadFloat3(&moveVec_);
        // 進行ベクトルがゼロベクトルの場合は処理する必要なし
        float length = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(MoveVec));
        if (length <= 0.01f)
        {
            return;
        }
        // 進行ベクトルを単位ベクトル化
        MoveVec = DirectX::XMVector3Normalize(MoveVec);

        //startRotation_=DirectX::XMQuaternionRotationRollPitchYaw()

        //auto q = SlerpQuaternion(startRotation_, targetRotation_, t);
        //owner_.lock()->SetQuaternionRotation(q);

    }

    void SetRotateTime(float t) { this->rotateTime_ = t; }
private:
    // t: 補間率（0.0 ~ 1.0）
    DirectX::XMFLOAT4 SlerpQuaternion(const DirectX::XMFLOAT4& current, const DirectX::XMFLOAT4& target, float t)
    {
        using namespace DirectX;

        XMVECTOR q1 = XMLoadFloat4(&current);
        XMVECTOR q2 = XMLoadFloat4(&target);
        XMVECTOR result = XMQuaternionSlerp(q1, q2, t);

        XMFLOAT4 out;
        XMStoreFloat4(&out, result);
        return out;
    }

    bool IsSameDirection(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        constexpr float epsilon = 0.001f;
        return std::abs(a.x - b.x) < epsilon &&
            std::abs(a.y - b.y) < epsilon &&
            std::abs(a.z - b.z) < epsilon;
    }

    DirectX::XMFLOAT3 direction_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 previousDirection_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 startAngle_ = { 0.0f,0.0f,0.0f };  // degree
    DirectX::XMFLOAT4 targetRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    DirectX::XMFLOAT4 startRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    float lerpTime_ = 0.0f;
    float rotateTime_ = 0.3f;    // 3秒で rotation する
    float rotateSpeed_ = 10.0f; // ( degree / second )

    DirectX::XMFLOAT3 moveVec_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 forward_ = { 0.0f,0.0f,0.0f };
};

#endif //CONTROLLER_COMPONENT_H