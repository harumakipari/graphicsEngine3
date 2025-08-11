#ifndef DEFEAT_ENEMY_CAMERA_H
#define DEFEAT_ENEMY_CAMERA_H
#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Components/Camera/CameraComponent.h"
#include "Game/Actors/Enemy/DefeatEnemy.h"
#include "Utils/EasingHandler.h"
#include "Game/Actors/Player/TitlePlayer.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Camera/Camera.h"

class DefeatEnemyCamera :public Camera
{
public:
    //引数付きコンストラクタ
    DefeatEnemyCamera(std::string actorName) :Camera(actorName) {}

    virtual ~DefeatEnemyCamera() = default;
    std::shared_ptr<CameraComponent> mainCameraComponent;
    std::shared_ptr<SphereComponent> sphereComponent;

    float easeX = 0.0f;
    float easeY = 0.0f;
    float easeZ = 0.0f;
    EasingHandler xEasing;
    EasingHandler yEasing;
    EasingHandler zEasing;

    void Initialize(/*const Transform& transform*/)override
    {
        mainCameraComponent = this->NewSceneComponent<class CameraComponent>("mainCamera");
        mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(45), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
        SetPosition(DirectX::XMFLOAT3(0.0f, 5.0f, -5.0f));
        SetEulerRotation(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));

        //SetPosition(transfor)

        SetCaemraEasing();

        // enemy を先に作成しているため
        DefeatEnemy* enemy = dynamic_cast<DefeatEnemy*>(GetOwnerScene()->GetActorManager()->GetActorByName("defeatEnemy").get());
        if (enemy)
        {
            enemy->onAnimationFinished = [this]()
                {
                    //Transform playerTr(DirectX::XMFLOAT3{ 1.0f,0.4f,6.5f }, DirectX::XMFLOAT3{ 0.0f,-76.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
                    Transform playerTr(DirectX::XMFLOAT3{ 5.0f,0.4f,0.04f }, DirectX::XMFLOAT3{ 0.0f,-76.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
                    resultPlayer = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<TitlePlayer>("resultActor", playerTr);
                    resultPlayer->SetType(TitlePlayer::Type::ResultWin);
                    resultPlayer->PlayAnimation("Win");

                    //resultPlayerPos = resultPlayer->GetPosition();
                    // これが次の target 
                    //resultPlayerPos = resultPlayer->playerJointComponent->GetComponentWorldTransform().GetLocation();
                    resultPlayerPos = resultPlayer->socketNodeComponent->GetComponentWorldTransform().GetLocation();
                    DirectX::XMFLOAT3 forward = resultPlayer->GetForward();
                    XMVECTOR Focus = XMLoadFloat3(&resultPlayerPos);
                    XMVECTOR Forward = XMVector3Normalize(XMLoadFloat3(&forward));
                    XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
                    XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Forward, Up));
                    Up = XMVector3Normalize(XMVector3Cross(Forward, Right));

                    XMVECTOR Eye = Focus + (Right * -eyeOffset.x) + (Forward * eyeOffset.z) + (Up * eyeOffset.y);
                    XMStoreFloat3(&nextEyePos, Eye);


                    //
                    //DirectX::XMFLOAT3 targetPos =
                    //{
                    //    (enemyPos.x + playerPos.x) * 0.5f,
                    //    (enemyPos.y + playerPos.y) * 0.5f,
                    //    (enemyPos.z + playerPos.z) * 0.5f
                    //};

                    state = State::SceneLerpPlayer;
                    //DirectX::XMFLOAT3 eyePos =
                    //{
                    //    targetPos.x ,
                    //    targetPos.y + 1.5f,
                    //    targetPos.z + 5.0f
                    //};
                    DirectX::XMFLOAT3 eyePos =
                    {
                        -4.8f ,
                        2.0f,
                        8.7f
                    };

                    // イージングでeyeに向けてカメラを移動させる
                    x2Easing.Clear();
                    x2Easing.SetEasing(EaseType::InOutSine, GetPosition().x, eyePos.x, 1.5f);

                    y2Easing.Clear();
                    y2Easing.SetEasing(EaseType::InOutSine, GetPosition().y, eyePos.y, 1.5f);

                    z2Easing.Clear();
                    z2Easing.SetEasing(EaseType::InOutSine, GetPosition().z, eyePos.z, 1.5f);

                };
        }
    };

    void SetCaemraEasing()
    {
#if 0
        xEasing.Clear();
        xEasing.SetWait(2.0f);
        xEasing.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 1.0f);
        xEasing.SetEasing(EaseType::InSine, 1.0f, 0.0f, 1.0f);

        yEasing.Clear();
        easeY = 2.5f;
        yEasing.SetWait(1.0f);
        yEasing.SetEasing(EaseType::InOutSine, 2.5f, 0.5f, 2.0f);

        zEasing.Clear();
        zEasing.SetEasing(EaseType::Linear, 3.0f, 5.0f, 2.0f);

        zEasing.SetEasing(EaseType::OutQuart, 5.0f, 4.0f, 0.5f);
#else
        xEasing.Clear();
        xEasing.SetWait(5.f);
        xEasing.SetEasing(EaseType::InOutSine, 0.0f, 1.0f, 1.5f);

        yEasing.Clear();
        easeY = 2.5f;
        yEasing.SetWait(5.f);
        yEasing.SetEasing(EaseType::Linear, 2.5f, 1.0f, 1.0f);
        //yEasing.SetWait(0.15f);
        yEasing.SetEasing(EaseType::OutCubic, 1.0f, 1.5f, 0.25f);
        yEasing.SetEasing(EaseType::OutCubic, 1.5f, 1.0f, 0.25f);

        zEasing.Clear();
        zEasing.SetEasing(EaseType::Linear, 2.0f, 6.0f, 4.0f);

        zEasing.SetEasing(EaseType::OutExp, 6.0f, 5.0f, 0.25f);

        //zEasing.SetEasing(EaseType::Linear, 6.0f, 5.0f, 0.5f);

#endif // 0

    }
    //更新処理
    void Update(float deltaTime)override
    {
#if 1
        DefeatEnemy* enemy = dynamic_cast<DefeatEnemy*>(GetOwnerScene()->GetActorManager()->GetActorByName("defeatEnemy").get());

        switch (state)
        {
        case DefeatEnemyCamera::State::EndingPerform:
            xEasing.Update(easeX, deltaTime);
            yEasing.Update(easeY, deltaTime);
            zEasing.Update(easeZ, deltaTime);

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
            preTargetPos = focus;
            preEyePos = eye;
            mainCameraComponent->customTarget = true;
            mainCameraComponent->_target = focus;
            SetPosition(eye);
            elapsedTime = 0.0f;
            break;
        case DefeatEnemyCamera::State::SceneLerpPlayer:
        {
            //DirectX::XMFLOAT3 forward = resultPlayer->GetForward();
            //resultPlayerPos = resultPlayer->socketNodeComponent->GetComponentWorldTransform().GetLocation();
            resultPlayerPos = resultPlayer->newTarget;
            DirectX::XMFLOAT3 forward = resultPlayer->GetForward();
            XMVECTOR Focus = XMLoadFloat3(&resultPlayerPos);
            XMVECTOR Forward = XMVector3Normalize(XMLoadFloat3(&forward));
            XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
            XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Forward, Up));
            Up = XMVector3Normalize(XMVector3Cross(Forward, Right));

            XMVECTOR Eye = Focus + (Right * -eyeOffset.x) + (Forward * eyeOffset.z) + (Up * eyeOffset.y);
            XMStoreFloat3(&nextEyePos, Eye);

            DirectX::XMVECTOR PreTarget = DirectX::XMLoadFloat3(&preTargetPos);
            DirectX::XMVECTOR AfterTarget = DirectX::XMLoadFloat3(&resultPlayerPos);
            DirectX::XMVECTOR PreEye = DirectX::XMLoadFloat3(&preEyePos);
            DirectX::XMVECTOR AfterEye = DirectX::XMLoadFloat3(&nextEyePos);
            elapsedTime += deltaTime;
            float lerpTime = 3.0f;
            float t = std::clamp(elapsedTime / lerpTime, 0.0f, 1.0f);

            DirectX::XMVECTOR Target = DirectX::XMVectorLerp(PreTarget, AfterTarget, t);
            DirectX::XMVECTOR NowEye = DirectX::XMVectorLerp(PreEye, AfterEye, t);

            DirectX::XMFLOAT3 tar, e;

            DirectX::XMStoreFloat3(&tar, Target);
            DirectX::XMStoreFloat3(&e, NowEye);


            //mainCameraComponent->_target = resultPlayerPos;
            //SetPosition(nextEyePos);
            mainCameraComponent->customTarget = true;

            if (t >= 1.0f)
            {
                //resultPlayer->SetType(TitlePlayer::Type::ResultWinMove);
                mainCameraComponent->_target = tar;
                SetPosition(nextEyePos);
                state = State::SceneResult;
                resultPlayer->isCameraFinish = true;
            }
            else
            {
                mainCameraComponent->_target = tar;
                SetPosition(e);
            }
#if 0
            x2Easing.Update(easeX2, deltaTime);
            y2Easing.Update(easeY2, deltaTime);
            z2Easing.Update(easeZ2, deltaTime);
            auto enemyActor = ActorManager::GetActorByName("defeatEnemy");
            //DirectX::XMFLOAT3 enemyPos = enemyActor->GetPosition();

            //float lerpTime = 10.0f;
            //elapsedTime += deltaTime;
            //float t = std::clamp(elapsedTime / lerpTime, 0.0f, 1.0f);
            //DirectX::XMVECTOR enemyPosVec = DirectX::XMLoadFloat3(&enemyPos);
            //DirectX::XMVECTOR Target = DirectX::XMVectorSet(-1.2f, 0.1f, -0.9f, 1.0f);
            //enemyPosVec = DirectX::XMVectorLerp(enemyPosVec, Target, t);

            //DirectX::XMFLOAT3 pos = {};
            //DirectX::XMStoreFloat3(&pos, enemyPosVec);
            //enemyActor->SetPosition(pos);
            SetPosition({ easeX2,easeY2,easeZ2 });

#endif // 0
        }
        break;
        case DefeatEnemyCamera::State::SceneResult:
        {
            //resultPlayerPos = resultPlayer->playerJointComponent->GetComponentWorldTransform().GetLocation();
            resultPlayerPos = resultPlayer->socketNodeComponent->GetComponentWorldTransform().GetLocation();
            DirectX::XMFLOAT3 forward = resultPlayer->GetForward();
            XMVECTOR Focus = XMLoadFloat3(&resultPlayerPos);

            XMVECTOR Forward = XMVector3Normalize(XMLoadFloat3(&forward));
            XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
            XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Forward, Up));
            Up = XMVector3Normalize(XMVector3Cross(Forward, Right));

            XMVECTOR Eye = Focus + (Right * -eyeOffset.x) + (Forward * eyeOffset.z) + (Up * eyeOffset.y);
            XMStoreFloat3(&nextEyePos, Eye);
            mainCameraComponent->customTarget = true;
            mainCameraComponent->_target = resultPlayerPos;
            SetPosition(nextEyePos);
        }
        break;
        default:
            break;
        }
#else
        //SetEulerRotation(rotation);

        DirectX::XMFLOAT3 eye = GetPosition();
        DirectX::XMVECTOR backDir = XMVectorSet(0, 0, -1, 0); // 後ろ向き
        DirectX::XMMATRIX Rot = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&GetQuaternionRotation()));
        backDir = XMVector3TransformNormal(backDir, Rot);
        backDir = XMVector3Normalize(backDir);

        DirectX::XMVECTOR targetVec = XMLoadFloat3(&target);
        DirectX::XMVECTOR eyePos = XMVectorMultiplyAdd(backDir, XMVectorReplicate(distance), targetVec);

        DirectX::XMStoreFloat3(&eye, eyePos);
        SetPosition(eye);
#endif // 0
    }

    void SetTarget(DirectX::XMFLOAT3 target)
    {
        this->target = target;
    }

    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI
        ImGui::DragFloat3("cameraTarget", &target.x, 0.3f);
        ImGui::DragFloat3("eyeOffset", &eyeOffset.x, 0.3f);
        ImGui::DragFloat3("targetOffset", &targetOffset.x, 0.3f);
        ImGui::DragFloat("distance", &distance, 0.3f);
#endif
    }

private:
    DirectX::XMFLOAT3 target = { 0.0f,0.0f,0.0f };
    float distance = 5.1f;
    enum class State
    {
        EndingPerform,  // 終わりの演出
        SceneLerpPlayer,    // リザルト画面の演出
        SceneResult,
    };
    State state = State::EndingPerform;

    float easeX2 = 0.0f;
    float easeY2 = 0.0f;
    float easeZ2 = 0.0f;
    EasingHandler x2Easing;
    EasingHandler y2Easing;
    EasingHandler z2Easing;

    float elapsedTime = 0.0f;

    DirectX::XMFLOAT3 resultPlayerPos = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 nextEyePos = { 0.0f,0.0f,0.0f };

    std::shared_ptr<TitlePlayer> resultPlayer;
    //DirectX::XMFLOAT3 eyeOffset = { -1.1f,-0.5f,-3.8f };
    //DirectX::XMFLOAT3 eyeOffset = { 0.1f,-0.5f,-3.8f };
    DirectX::XMFLOAT3 eyeOffset = { 3.7f,-2.0f,6.4f };
    DirectX::XMFLOAT3 targetOffset = { 0.0f,0.0f,0.0f };

    DirectX::XMFLOAT3 preTargetPos = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 preEyePos = { 0.0f,0.0f,0.0f };

    bool skipFirstFrameInSceneResult = false;
};

#endif // !DEFEAT_ENEMY_CAMERA_H
