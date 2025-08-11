#pragma once
#include "Enemy.h"

#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Beam/Beam.h"

#include "BehaviorTree.h"
#include "BehaviorData.h"
#include "NodeBase.h"
#include "JudgmentDerived.h"
#include "ActionDerived.h"

#include "Components/Audio/AudioSourceComponent.h"
#include "Components/Controller/ControllerComponent.h"

struct ParticleSystem;

class BehaviorTree;
class BehaviorData;
class NodeBase;

class RiderEnemy :public Enemy
{
public:
    RiderEnemy() = default;
    ~RiderEnemy() override {}

    RiderEnemy(const std::string& modelName);

    //コピーコンストラクタとコピー代入演算子を禁止にする
    RiderEnemy(const RiderEnemy&) = delete;
    RiderEnemy& operator=(const RiderEnemy&) = delete;

    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    std::shared_ptr<SkeltalMeshComponent> gearInMeshComponent;
    std::shared_ptr<SkeltalMeshComponent> gearOutMeshComponent;
    std::shared_ptr<SphereComponent> bossHandComponent;
    std::shared_ptr<MovementComponentOutInput> moveComponent;
    std::shared_ptr<AudioSourceComponent> rushAudioComponent;
    std::shared_ptr<AudioSourceComponent> misileAudioComponent;
    std::shared_ptr<AudioSourceComponent> specialAudioComponent;
    std::shared_ptr<AudioSourceComponent> landingAudioComponent;
    std::shared_ptr<AudioSourceComponent> walkAudioComponent;
    std::shared_ptr<SphereComponent> bossJointComponent = nullptr;
public:
    void Initialize(const Transform& transform)override
    {
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
#if 0
        //skeltalMeshComponent->SetModel("..\\glTF-Sample-Models-main\\original\\EnemyTest\\Idle_Relaxed_B_HS.gltf");
        skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/plantune.gltf");
        //skeltalMeshComponent->model->isModelInMeters = false;
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        SetScale(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
#else // モデル確認
        skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_idle.gltf");
        skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_emission2");
        skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_boss_emission");
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        //SetScale(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
#endif
        SetPosition(transform.GetLocation());
        DirectX::XMFLOAT4 quaternion;
        angle.y = -180.0f;
        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
        DirectX::XMStoreFloat4(&quaternion, q);
        SetQuaternionRotation(quaternion);
        SetScale(transform.GetScale());
        SetPosition({ 0,10,0.1f });

        //SetPosition(DirectX::XMFLOAT3(3.0f, 0.0f, -5.0f));
        const std::vector<std::string> animationFilenames =
        {
            "./Data/Models/Characters/Enemy/BOS_walk.gltf",
            "./Data/Models/Characters/Enemy/BOS_punch.gltf",
            "./Data/Models/Characters/Enemy/yobi.gltf",
            "./Data/Models/Characters/Enemy/rotate.gltf",
            "./Data/Models/Characters/Enemy/rotate_end.gltf",
            "./Data/Models/Characters/Enemy/jump_yobi.gltf",
            "./Data/Models/Characters/Enemy/jump_air.gltf",
            "./Data/Models/Characters/Enemy/jump_landing.gltf",
            "./Data/Models/Characters/Enemy/jump_landing2.gltf",
            "./Data/Models/Characters/Enemy/specal_move.gltf",
            "./Data/Models/Characters/Enemy/aristrike.gltf",
            "./Data/Models/Characters/Enemy/BOS_damage.gltf",

            //"./Data/Models/Characters/Enemy/.gltf",

            //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Ability_E_InMotion.glb",
            //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Primary_Attack_Fast_A.glb",
        };

        skeltalMeshComponent->AppendAnimations(animationFilenames);

        // アニメーションコントローラーを作成
        auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
        controller->AddAnimation("Idle", 0);
        controller->AddAnimation("Walk", 1);
        controller->AddAnimation("Normal", 2);
        controller->AddAnimation("RotatePre", 3);
        controller->AddAnimation("Rotate", 4);
        controller->AddAnimation("RotateEnd", 5);
        controller->AddAnimation("JumpPre", 6);
        controller->AddAnimation("Jump", 7);
        controller->AddAnimation("JumpEnd", 8);
        controller->AddAnimation("JumpEnd2", 9);
        controller->AddAnimation("Special", 10);
        controller->AddAnimation("Bombing", 11);
        controller->AddAnimation("Damage", 12);

        // アニメーションコントローラーをcharacterに追加
        this->SetAnimationController(controller);
        this->PlayAnimation("Walk");

        moveComponent = this->NewSceneComponent<MovementComponentOutInput>("movementComponent", "skeltalComponent");

        //std::shared_ptr<CapsuleComponent> capsuleComponent = this->NewComponent<class CapsuleComponent>("capsuleComponent", "skeltalComponent");
        //capsuleComponent->SetRadiusAndHeight(radius, height);
        //capsuleComponent->SetMass(40.0f);
        //capsuleComponent->Initialize();


        std::shared_ptr<BoxComponet> boxComponent = this->NewSceneComponent<class BoxComponet>("capsuleComponent", "skeltalComponent");
        boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.5f, 2.1f, radius));
        boxComponent->SetModelHeight(2.1f * 0.5f);
        boxComponent->SetMass(40.0f);
        //boxComponent->SetStatic(true);
        boxComponent->SetLayer(CollisionLayer::Enemy);
        boxComponent->SetResponseToLayer(CollisionLayer::EnemyHand, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->Initialize();


        bossHandComponent = this->NewSceneComponent<class SphereComponent>("bossHand", "skeltalComponent");
        bossHandComponent->SetRadius(0.5f);
        //DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointLocalPosition("R_thumb1_FK", skeltalMeshComponent->model->GetNodes());
        DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointLocalPosition("R_thumb1_FK", skeltalMeshComponent->modelNodes);
        bossHandComponent->SetRelativeLocationDirect(bossHand);
        bossHandComponent->SetMass(40.0f);
        bossHandComponent->SetLayer(CollisionLayer::EnemyHand);
        bossHandComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
        bossHandComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        bossHandComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        bossHandComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Trigger);
        bossHandComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        bossHandComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        bossHandComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        bossHandComponent->Initialize();

        //std::shared_ptr<SphereComponent> attackRangeComponent = this->NewSceneComponent<class SphereComponent>("attackRangeComponent", "skeltalComponent");
        //attackRangeComponent->SetRadius(attackRange);
        //attackRangeComponent->SetIsVisibleDebugBox(false);

        std::shared_ptr<SphereComponent> nearRangeComponent = this->NewSceneComponent<class SphereComponent>("nearRangeComponent", "skeltalComponent");
        nearRangeComponent->SetRadius(distNear);
        nearRangeComponent->SetIsVisibleDebugBox(false);

        std::shared_ptr<SphereComponent> midRangeComponent = this->NewSceneComponent<class SphereComponent>("midRangeComponent", "skeltalComponent");
        midRangeComponent->SetRadius(distMid);
        midRangeComponent->SetIsVisibleDebugBox(false);

        std::shared_ptr<SphereComponent> farRangeComponent = this->NewSceneComponent<class SphereComponent>("farRangeComponent", "skeltalComponent");
        farRangeComponent->SetRadius(distFar);
        farRangeComponent->SetIsVisibleDebugBox(false);


        // ギアのモデルコンポーネントを追加
        gearInMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("gearInComponent", "skeltalComponent");
        gearInMeshComponent->SetModel("./Data/Effect/Models/gear_effect_in.gltf");
        gearInMeshComponent->SetIsCastShadow(false);
        gearInMeshComponent->SetIsVisible(false);
        gearOutMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("gearOutComponent", "skeltalComponent");
        gearOutMeshComponent->SetModel("./Data/Effect/Models/gear_effect.gltf");
        gearOutMeshComponent->SetIsCastShadow(false);
        gearOutMeshComponent->SetIsVisible(false);

        // 突進時の音コンポーネントを追加
        rushAudioComponent = this->NewSceneComponent<AudioSourceComponent>("rushAudioComponent", "skeltalComponent");
        rushAudioComponent->SetSource(L"./Data/Sound/SE/rush.wav");

        // ミサイル時の音コンポーネントを追加
        misileAudioComponent = this->NewSceneComponent<AudioSourceComponent>("misileAudioComponent", "skeltalComponent");
        misileAudioComponent->SetSource(L"./Data/Sound/SE/missile_launch.wav");

        // 必殺技時の音コンポーネントを追加
        specialAudioComponent = this->NewSceneComponent<AudioSourceComponent>("specialAudioComponent", "skeltalComponent");
        specialAudioComponent->SetSource(L"./Data/Sound/SE/special.wav");

        // 着地時の音コンポーネントを追加
        landingAudioComponent = this->NewSceneComponent<AudioSourceComponent>("landingAudioComponent", "skeltalComponent");
        landingAudioComponent->SetSource(L"./Data/Sound/SE/landing.wav");

        // 歩きのコンポーネント
        walkAudioComponent = this->NewSceneComponent<AudioSourceComponent>("walkAudioComponent", "skeltalComponent");
        walkAudioComponent->SetSource(L"./Data/Sound/SE/walk.wav");

        // 目玉にジョイントを入れる
        bossJointComponent = this->NewSceneComponent<SphereComponent>("bossJointComponent", "skeltalComponent");
        bossJointComponent->SetRadius(1.0f);
        DirectX::XMFLOAT3 bossJoint = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
        bossJointComponent->SetRelativeLocationDirect(bossJoint);

        OutputDebugStringA(("Actor::Initialize called. rootComponent_ use_count = " + std::to_string(rootComponent_.use_count()) + "\n").c_str());
    }

    //　collisionComponent　が Dynamic の物と当たった時に通る
    void NotifyHit(Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
#if 0
        if (auto beam = dynamic_cast<Beam*>(otherActor))
        {// ビームに当たったら、
            if (beam == lastHitBeam_)
            {// 二度ヒットを防ぐため
                return;
            }

            // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (int)
            int beamPower = beam->GetItemCount();


            hp -= beamPower;
            //char debugBuffer[128];
            //sprintf_s(debugBuffer, sizeof(debugBuffer),
            //    "beam is hit beam! beam power:%d\n",beamPower);
            //OutputDebugStringA(debugBuffer);
            lastHitBeam_ = beam;
        }

#endif // 0
    }

    // キネマティック同士の当たり判定を検知する
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override;

    //// 当たった時に通る関数       // hitShapes.firstが自身 hitShapes.secondが相手
    //void BroadcastHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)override
    //{
    //    if (hitShapes.second->GetOwner()->GetName() == "actor")
    //    {
    //        OutputDebugStringA(("actor hit enemy!! \n"));
    //    }
    //    //else if (others->GetName() == "actor")
    //    //{
    //    //    OutputDebugStringA(("enemy hit actor!! \n"));
    //    //}
    //};

    void Finalize()override;

    void Update(float elapsedTime)override;

    void UpdateParticle(ID3D11DeviceContext* immediate_context, float deltaTime, ParticleSystem* p)override;

    void AnimationTransition();

    //プレイヤー索敵
    bool SearchPlayer(DirectX::XMFLOAT3 playerPosition);

    //目標地点設定
    void SetRandomTargetPosition();

    //目標地点へ移動   
    void MoveToTarget(float elapsedTime, float speedRate);

    //目的地へジャンプ
    void JumpToTarget(float elapsedTime, float heightMax, float speedRate);

    //移動処理
    void Move(float elapsedTime, float vx, float vz, float speed);

    //回転処理
    void Turn(float elapsedTime, float vx, float vz, float speed);

    //水平速力処理
    void UpdateHorizontalVelocity(float elapsedTime);

    //水平移動処理
    void UpdateHorizontalMove(float elapsedTime);

    //垂直速力更新処理
    void UpdateVerticalVelocity(float elapsedTime);

    //垂直移動更新処理
    void UpdateVerticalMove(float elapsedTime);

    //ターゲットポジション取得
    DirectX::XMFLOAT3 GetTargetPosition() { return targetPosition; }

    //ターゲットポジション設定
    void SetTargetPosition(DirectX::XMFLOAT3 position) { targetPosition = position; }

    //最大HP取得
    int GetMaxHP() { return maxHp; }

    //攻撃範囲取得
    float GetAttackRange() { return attackRange; }

    //HP取得
    int GetHP() { return hp; }

    //HP設定
    void SetHP(int hp) { this->hp = hp; }

    //現在の必殺ゲージ取得
    int GetCurrentSpecialGauge() { return static_cast<int>(specialGauge); }

    //現在の必殺ゲージ設定
    void SetCurrentSpecialGauge(float currentSpecial) { this->specialGauge = currentSpecial; }

    //最大必殺ゲージ取得
    int GetMaxSpecialGauge() { return static_cast<int>(maxSpecialGauge); }

    //形態変化フラグ取得
    bool GetIsChange() { return isChange; }

    //形態変化フラグ設定
    void SetIsChange(bool isChange) { this->isChange = isChange; }

    //距離小取得
    float GetDistNear() { return distNear; }

    //距離中取得
    float GetDistMid() { return distMid; }

    //距離大取得
    float GetDistFar() { return distFar; }

    //追跡時間取得
    float GetRunTimer() { return runTimer; }

    //追跡時間設定
    void SetRunTimer(float runTimer) { this->runTimer = runTimer; }

    //突進最大タイマー取得
    float GetMaxChargeTimer() { return MaxChargeTimer; }

    //突進タイマー取得
    float GetChargeTimer() { return chargeTimer; }

    //突進タイマー設定
    void SetChargeTimer(float chargeTimer) { this->chargeTimer = chargeTimer; }

    //クールタイム中か取得
    bool GetIsCoolTime() { return isCoolTime; }

    //クールタイム中か設定
    void SetIsCoolTime(bool coolTime) { this->isCoolTime = coolTime; }

    //最大クールタイム取得
    float GetMaxCoolTime() { return maxCoolTime; }
    //最小クールタイム取得
    float GetMinCoolTime() { return minCoolTime; }

    //クールタイム取得
    float GetCoolTime() { return coolTime; }

    //クールタイム設定
    void SetCoolTime(float coolTime) { this->coolTime = coolTime; }

    //地形変動最大クールタイム取得
    float GetMaxTerrainTime() { return maxTerrainTime; }

    //地形変動クールタイム取得
    float GetTerrainTime() { return terrainTime; }

    //地形変動クールタイム設定
    void SetTerrainTime(float terrainTime) { this->terrainTime = terrainTime; }

    //地形変動を出せるかどうか取得
    bool GetCanTerrain() { return canTerrain; }

    //地形変動を出せるかどうか設定
    void SetCanTerrain(bool canTerrain) { this->canTerrain = canTerrain; }

    //必殺を出せるか取得
    bool GetCanSpecial() { return canSpecial; }

    //必殺を出せるか設定
    void SetCanSpecial(bool finishSpecial) { this->canSpecial = finishSpecial; }

    //召喚最大クールタイム取得
    float GetMaxSummonTime() { return maxSummonTime; }

    //召喚クールタイム取得
    float GetSummonTime() { return summonTime; }

    //召喚クールタイム設定
    void SetSummonTime(float summonTime) { this->summonTime = summonTime; }

    //召喚できるか取得
    bool GetCanSummon() { return canSummon; }

    //召喚できるか設定
    void SetCanSummon(bool canSummon) { this->canSummon = canSummon; }

    //ジャンプ中か取得
    bool GetIsJumping() { return isJumping; }

    //ジャンプ中か設定
    void SetIsJumping(bool isJumping) { this->isJumping = isJumping; }

    //20以上のダメージを受けたか取得
    bool GetIsStaggered() { return isStaggered; }

    //20以上のダメージを受けたか設定
    void SetIsStaggered(bool isStaggered) { this->isStaggered = isStaggered; }

    //ダメージを受けた回数取得
    int GetDamageCount() { return damageCount; }

    //ダメージを受けた回数設定
    void SetDamageCount(int damageCount) { this->damageCount = damageCount; }

    //最初の演出か取得
    bool GetIsStartPerf() { return isStartPerf; }

    //最初の演出か設定
    void SetIsStartPerf(bool isStartPerf) { this->isStartPerf = isStartPerf; }

    //最初の演出で敵が落ち始めるか取得
    bool GetIsEnemyFall() { return isStartEnemyFall; }

    //ダメージを判定を開始するか取得
    bool GetEnableHit() { return enableHit; }

    //ダメージを判定を開始するか設定
    void SetEnableHit(bool enableHit) { this->enableHit = enableHit; }

private:

    enum class AnimationState
    {
        Idle,
        FwdStart,
        FwdWalk,
        FwdStop,
        Attack,
        Damage,
        Death,
    };

private:
    // 同じフレームでのダメージ処理を取る
    bool hasHitThisFrame = false;

    //索敵範囲
    float searchRange = 5.0f;

    //目標地点
    DirectX::XMFLOAT3 targetPosition{ 0.0f,0.0f,0.0f };

    //目標地点を設定する範囲
    float territoryRange = 8.0f;

    //待機時間
    float stateTimer = 0.0f;

    //TODO:02IMGUI用
    bool isDetected = false;
    bool isAttacked = false;

    // particles を 更新するかどうか
    bool isIntegrateParticles = false;

    bool testFlag = false;

    //最大HP
    int maxHp = 210;

    //攻撃範囲
    float attackRange = 15.0f;

    //現在の必殺ゲージ
    float specialGauge = 0;
    //最大必殺ゲージ
    const float maxSpecialGauge = 40;
    //必殺技を出したかどうか
    bool canSpecial = false;

    //形態変化したかどうか
    bool isChange = false;

    //距離小
    float distNear = 3.0f;
    //距離中
    float distMid = 8.0f;
    //距離大
    float distFar = 15.0f;

    //追跡時間
    float runTimer = 0.0f;

    //突進の溜め最大時間
    const float MaxChargeTimer = 2.0f;
    //突進の溜め時間
    float chargeTimer = 0.0f;

    //クールタイム中か
    bool isCoolTime = false;

    //最大クールタイム
    const float maxCoolTime = 4.0f;
    //最小クールタイム
    const float minCoolTime = 3.0f;
    //クールタイム
    float coolTime = 0.0f;

    //地形変動最大クールタイム
    const float maxTerrainTime = 25.0f;
    //地形変動クールタイム
    float terrainTime = 0.0f;
    //地形変動を出せるかどうか
    bool canTerrain = false;

    //召喚最大クールタイム
    const float maxSummonTime = 5.0f;
    //召喚クールタイム
    float summonTime = 0.0f;
    //召喚クールタイム中か
    bool canSummon = true;

    //摩擦
    float friction = 0.5f;
    //加速度
    float acceleration = 1.0f;
    //最大速度
    float maxMoveSpeed = 5.0f;
    //X軸移動ベクトル
    float moveVecX = 0.0f;
    //Z軸移動ベクトル
    float moveVecZ = 0.0f;
    //空中の制御
    float airControl = 0.3f;
    //移動速度
    float moveSpeed = 3.0f;
    //回転速度
    float turnSpeed = DirectX::XMConvertToRadians(360);

    //ジャンプ中か
    bool isJumping = false;
    //ジャンプ経過時間
    float jumpT = 0.0f;
    //ジャンプ始点
    DirectX::XMFLOAT3 jumpStart = { 0,0,0 };
    //ジャンプ終点
    DirectX::XMFLOAT3 jumpEnd = { 0,0,0 };
    //ジャンプの中点
    DirectX::XMFLOAT3 jumpPeak = { 0,0,0 };
    //最終的な終点
    DirectX::XMFLOAT3 lastTarget = { 0,0,0 };

    DirectX::XMFLOAT4 uiAngle = { 0,0,0,0 };

    //20以上のダメージを受けたか
    bool isStaggered = false;

    //ダメージを受けた回数
    int damageCount = 0;

    //ダメージ判定を開始するか
    bool enableHit = false;
public:
    //最初の演出かどうか
    bool isStartPerf = true;
    // 敵が落ちてくるかどうか
    bool isStartEnemyFall = false;
private:
    BehaviorTree* aiTree = nullptr;
    BehaviorData* behaviorData = nullptr;
public:
    NodeBase* activeNode = nullptr;
private:
    DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };

    Beam* lastHitBeam_ = nullptr;

public:
    // ここが歯車の出る場所
    DirectX::XMFLOAT3 GetJumpPosition()
    {
        return lastTarget;
    }
    // 歯車が出ている時間
    bool IsEnemyJumping()
    {
        return isJumping;
    }

};