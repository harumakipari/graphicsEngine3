#ifndef TITLE_PLAYER_H
#define TITLE_PLAYER_H

#include "Game/Actors/Base/Character.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Stage/TitleStage.h"
#include "Utils/EasingHandler.h"
#include "Game/Actors/Enemy/DefeatEnemy.h"


class TitlePlayer :public Character
{
public:
    enum class Type
    {
        Title,
        ResultWin,
        ResultWinMove,
        ResultLose
    };
    Type type = Type::Title;
public:
    TitlePlayer(const std::string& modelName) :Character(modelName)
    {
        // カプセルの当たり判定を生成
        radius = 0.2f;
        height = 0.9f;
        mass = 50.0f;
        hp = 300;
        //PushState(std::make_shared<IdlingState>());
    }
    std::shared_ptr<SphereComponent> playerJointComponent = nullptr;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    std::shared_ptr<SphereComponent> socketNodeComponent;
    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        //skeltalMeshComponent->SetModel("./Data/Models/Characters/Player/chara_animation.gltf");
        skeltalMeshComponent->SetModel("./Data/Models/Characters/GirlSoldier/Idle.gltf");
        //skeltalMeshComponent->SetModel("./Data/Models/Characters/Player/chara_idle.gltf");
        //skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        //CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        //skeltalMeshComponent->SetIsCastShadow(false);
        const std::vector<std::string> animationFilenames =
        {
            "./Data/Models/Characters/GirlSoldier/Emote_Slice.glb",
            "./Data/Models/Characters/GirlSoldier/Jog_Fwd.glb",
            "./Data/Models/Characters/GirlSoldier/Ability_E_InMotion.glb",
            "./Data/Models/Characters/GirlSoldier/HitReact_Front.glb",
        };
        skeltalMeshComponent->AppendAnimations(animationFilenames);
        // アニメーションコントローラーを作成
        auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
        controller->AddAnimation("Idle", 0);
        controller->AddAnimation("Rotation", 1);
        controller->AddAnimation("Win", 2);
        controller->AddAnimation("Lose", 3);
        controller->AddAnimation("LoseIdle", 4);
        // アニメーションコントローラーをcharacterに追加
        this->SetAnimationController(controller);
        PlayAnimation("Idle");

        playerJointComponent = this->NewSceneComponent<SphereComponent>("playerJointComponent", "skeltalComponent");
        playerJointComponent->SetRadius(1.0f);
        DirectX::XMFLOAT3 playerHead = skeltalMeshComponent->model->GetJointLocalPosition("atama_FK", skeltalMeshComponent->model->GetNodes());

        playerJointComponent->SetRelativeLocationDirect(playerHead);

        socketNodeComponent = this->NewSceneComponent<SphereComponent>("socketNode", "playerJointComponent");
        socketNodeComponent->SetRadius(1.0f);


        SetPosition(transform.GetLocation());
        //SetQuaternionRotation(transform.GetRotation());
        angle = { 0.0f,160.0f,0.0f };
        //DirectX::XMFLOAT3 angle = { 0.0f,40.0f,0.0f };    // 対峙
        DirectX::XMVECTOR qutVec = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
        DirectX::XMFLOAT4 qut;
        DirectX::XMStoreFloat4(&qut, qutVec);
        SetQuaternionRotation(qut);
        //SetEulerRotation(angle);
        //SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
    }

    void SwitchPS(bool useDeffered)
    {
        if (useDeffered)
        {
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDefferedPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        }
        else
        {
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        }
    }

    void Update(float deltaTime)override
    {
        DirectX::XMFLOAT3 playerHead = skeltalMeshComponent->model->GetJointLocalPosition("atama_FK", skeltalMeshComponent->model->GetNodes());
        playerJointComponent->SetRelativeLocationDirect(playerHead);
        socketNodeComponent->SetRelativeLocationDirect(jointOffset);
        Character::Update(deltaTime);

        switch (type)
        {
        case Type::Title:
            if (!animationController_->IsPlayAnimation())
            {// 回転のアニメーションが終わったら
                isFinishRotate = true;
                Scene* currentScene = Scene::GetCurrentScene();
                if (!currentScene)
                {
                    // シーンがなければ処理しない or エラー処理
                    return;
                }
                if (auto title = std::dynamic_pointer_cast<TitleStage>(currentScene->GetActorManager()->GetActorByName("title")))
                {
                    title->OnPushButton();
                    PlayAnimation("Idle");
                }
            }
            if (isFinishRotate)
            {
                float rotateTime = 0.8f;
#if 1
                elapsedTimer += deltaTime;
                float t = std::clamp(elapsedTimer / rotateTime, 0.0f, 1.0f);
                angle.y = std::lerp(160.0f, 40.0f, t);
                SetEulerRotation(angle);
                if (t >= 1.0f)
                {
                    isFinishRotate = false;
                }
#else
                yEasing.Update(easeY, deltaTime);
                angle.y = easeY;
                SetEulerRotation(angle);
#endif // 0
            }
            if (onPushBack)
            {// 戻るボタンが押されたら
                float rotateTime = 0.6f;
                elapsedTimer += deltaTime;
                float t = std::clamp(elapsedTimer / rotateTime, 0.0f, 1.0f);
                angle.y = std::lerp(40.0f, 160.0f, t);
                SetEulerRotation(angle);
                if (t >= 1.0f)
                {
                    onPushBack = false;
                }
            }

            break;
        case Type::ResultLose:
        {
#if 1
            DirectX::XMFLOAT3 pos = GetPosition();
            float speedTime = 3.0f;
            elapsedTimer += deltaTime;
            float t = std::clamp(elapsedTimer / speedTime, 0.0f, 1.0f);
            pos.x = std::lerp(-3.0f, 0.0f, t);
            pos.z = std::lerp(10.0f, 0.0f, t);
            SetPosition(pos);
            if (pos.x <= -3.0f && !isFinishRotate)
            {
                PlayAnimation("Lose", false);
                isFinishRotate = true;
            }
#endif // 0
            if (!animationController_->IsPlayAnimation())
            {// ガックシのアニメーションが終わったら
                PlayAnimation("LoseIdle");
            }
        }
        break;
        case Type::ResultWin:
        {
            DirectX::XMFLOAT3 pos = GetPosition();
            static DirectX::XMFLOAT3 prevPos = GetPosition();

            // ボスを中心に円運動
            Scene* currentScene = Scene::GetCurrentScene();
            if (!currentScene)
            {
                // シーンがなければ処理しない or エラー処理
                return;
            }
            DefeatEnemy* enemy = dynamic_cast<DefeatEnemy*>(currentScene->GetActorManager()->GetActorByName("defeatEnemy").get());
            DirectX::XMFLOAT3 center = enemy->GetPosition();
            float radius = 8.0f;
            //if (isCameraFinish)
            elapsedTimer += deltaTime;
            if (elapsedTimer >= 0.00f)
            {
                degree += angularSpeed * deltaTime;
            }
            float x = center.x + radius * std::cosf(degree);
            float z = center.z + radius * std::sinf(degree);
            float y = GetPosition().y;

            DirectX::XMFLOAT3 newPos = { x, y, z };
            SetPosition(newPos);

            newTarget = socketNodeComponent->GetComponentWorldTransform().GetLocation();

            DirectX::XMFLOAT3 moveDir = {
                newPos.x - prevPos.x,
                0.0f,
                newPos.z - prevPos.z
            };

            // 正規化（長さゼロでなければ）
            float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
            if (len > 0.0001f)
            {
                moveDir.x /= len;
                moveDir.z /= len;

                // atan2(x, z) で進行方向をYaw角として得る
                float yaw = std::atan2(moveDir.x, moveDir.z); // X,Zの順に注意

                // クォータニオンで回転適用
                DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, yaw, 0.0f);
                DirectX::XMFLOAT4 quaternion;
                DirectX::XMStoreFloat4(&quaternion, q);
                SetQuaternionRotation(quaternion);
            }

            prevPos = newPos;            //float b = DirectX::XMConvertToRadians((DirectX::XMConvertToDegrees(degree) + c));
            //DirectX::XMFLOAT3 a = { 0.0f,b,0.0f };
            //float postDegree = DirectX::XMConvertToDegrees(degree) + c;
            //SetEulerRotation({ 0.0f,postDegree,0.0f });

            //DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(a.x, a.y, a.z);
            //DirectX::XMFLOAT4 q;
            //DirectX::XMStoreFloat4(&q, Quat);
            //SetQuaternionRotation(q);
            //elapsedTimer = 0.0f;
        }
        break;
        case Type::ResultWinMove:
        {
            //PlayAnimation("Win");
            //DirectX::XMFLOAT3 angle = { 0.0f,80.0f,0.0f };    // これが前面を見ている
            //SetEulerRotation(angle);
            //SetScale({ 1.0f,1.0f,-1.0f });
            DirectX::XMFLOAT3 pos = GetPosition();
            static DirectX::XMFLOAT3 prevPos = GetPosition();

            // ボスを中心に円運動
            Scene* currentScene = Scene::GetCurrentScene();
            if (!currentScene)
            {
                // シーンがなければ処理しない or エラー処理
                return;
            }
            DefeatEnemy* enemy = dynamic_cast<DefeatEnemy*>(currentScene->GetActorManager()->GetActorByName("defeatEnemy").get());
            DirectX::XMFLOAT3 center = enemy->GetPosition();
            float radius = 6.0f;
            elapsedTimer += deltaTime;
            //if (elapsedTimer >= 0.0f)
            {
                degree += angularSpeed * deltaTime;
            }

            float x = center.x + radius * std::cosf(degree);
            float z = center.z + radius * std::sinf(degree);
            float y = GetPosition().y;

            DirectX::XMFLOAT3 newPos = { x, y, z };
            SetPosition(newPos);

            DirectX::XMFLOAT3 moveDir = {
                newPos.x - prevPos.x,
                0.0f,
                newPos.z - prevPos.z
            };

            // 正規化（長さゼロでなければ）
            float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
            if (len > 0.0001f)
            {
                moveDir.x /= len;
                moveDir.z /= len;

                // atan2(x, z) で進行方向をYaw角として得る
                float yaw = std::atan2(moveDir.x, moveDir.z); // X,Zの順に注意

                // クォータニオンで回転適用
                DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, yaw, 0.0f);
                DirectX::XMFLOAT4 quaternion;
                DirectX::XMStoreFloat4(&quaternion, q);
                SetQuaternionRotation(quaternion);
            }

            prevPos = newPos;            //float b = DirectX::XMConvertToRadians((DirectX::XMConvertToDegrees(degree) + c));
            //DirectX::XMFLOAT3 a = { 0.0f,b,0.0f };
            //float postDegree = DirectX::XMConvertToDegrees(degree) + c;
            //SetEulerRotation({ 0.0f,postDegree,0.0f });

            //DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(a.x, a.y, a.z);
            //DirectX::XMFLOAT4 q;
            //DirectX::XMStoreFloat4(&q, Quat);
            //SetQuaternionRotation(q);
        }
        break;
        }
#ifdef USE_IMGUI
        ImGui::Begin("resultPlayer");
        ImGui::DragFloat3("jointOffset", &jointOffset.x, 0.01f);
        ImGui::DragFloat("degreeOffset", &c, 0.01f);
        ImGui::DragFloat("degree", &degree, 0.01f);
        ImGui::DragFloat3("angle", &angle.x, 1.0f);
        ImGui::End();
        DirectX::XMFLOAT3 eulerRadNew = 
        {
            DirectX::XMConvertToRadians(angle.x),
            DirectX::XMConvertToRadians(angle.y),
            DirectX::XMConvertToRadians(angle.z)
        };
        DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw(
            eulerRadNew.x, eulerRadNew.y, eulerRadNew.z
        );

        DirectX::XMFLOAT4 qNew;
        XMStoreFloat4(&qNew, quatNew);
        SetQuaternionRotation(qNew);

#endif // USE_IMGUI

    }

    // このプレイヤーの種類を設定
    void SetType(Type type) { this->type = type; }

    // 
    void OnPushBackToTitle()
    {
        onPushBack = true;
        elapsedTimer = 0.0f;
    }

    void OnPushStart()
    {
        PlayAnimation("Rotation", false);
        elapsedTimer = 0.0f;
    }
private:
    EasingHandler yEasing;
    float easeY = 0.0f;

    float elapsedTimer = 0.0f;
    bool isFinishRotate = false;
    bool onPushBack = false;
    //DirectX::XMFLOAT3 jointOffset = { 1.1f,0.006f,0.12f };
    //DirectX::XMFLOAT3 jointOffset = { 1.1f,0.006f,0.12f };
    DirectX::XMFLOAT3 jointOffset = { -3.3f,-0.234f,0.12f };

    float c = 80.0f;

    float angularSpeed = DirectX::XMConvertToRadians(30.0f);
    //float degree = 0.5f;
    //float degree = 1.5f;
    float degree = 2.5f;

public:
    DirectX::XMFLOAT3 newTarget = { 0.0f,0.0f,0.0f };
    bool isCameraFinish = false;
};

#endif // !TITLE_PLAYER_H
