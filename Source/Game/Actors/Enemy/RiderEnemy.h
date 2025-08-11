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

    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
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
#else // ���f���m�F
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

        // �A�j���[�V�����R���g���[���[���쐬
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

        // �A�j���[�V�����R���g���[���[��character�ɒǉ�
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


        // �M�A�̃��f���R���|�[�l���g��ǉ�
        gearInMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("gearInComponent", "skeltalComponent");
        gearInMeshComponent->SetModel("./Data/Effect/Models/gear_effect_in.gltf");
        gearInMeshComponent->SetIsCastShadow(false);
        gearInMeshComponent->SetIsVisible(false);
        gearOutMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("gearOutComponent", "skeltalComponent");
        gearOutMeshComponent->SetModel("./Data/Effect/Models/gear_effect.gltf");
        gearOutMeshComponent->SetIsCastShadow(false);
        gearOutMeshComponent->SetIsVisible(false);

        // �ːi���̉��R���|�[�l���g��ǉ�
        rushAudioComponent = this->NewSceneComponent<AudioSourceComponent>("rushAudioComponent", "skeltalComponent");
        rushAudioComponent->SetSource(L"./Data/Sound/SE/rush.wav");

        // �~�T�C�����̉��R���|�[�l���g��ǉ�
        misileAudioComponent = this->NewSceneComponent<AudioSourceComponent>("misileAudioComponent", "skeltalComponent");
        misileAudioComponent->SetSource(L"./Data/Sound/SE/missile_launch.wav");

        // �K�E�Z���̉��R���|�[�l���g��ǉ�
        specialAudioComponent = this->NewSceneComponent<AudioSourceComponent>("specialAudioComponent", "skeltalComponent");
        specialAudioComponent->SetSource(L"./Data/Sound/SE/special.wav");

        // ���n���̉��R���|�[�l���g��ǉ�
        landingAudioComponent = this->NewSceneComponent<AudioSourceComponent>("landingAudioComponent", "skeltalComponent");
        landingAudioComponent->SetSource(L"./Data/Sound/SE/landing.wav");

        // �����̃R���|�[�l���g
        walkAudioComponent = this->NewSceneComponent<AudioSourceComponent>("walkAudioComponent", "skeltalComponent");
        walkAudioComponent->SetSource(L"./Data/Sound/SE/walk.wav");

        // �ڋʂɃW���C���g������
        bossJointComponent = this->NewSceneComponent<SphereComponent>("bossJointComponent", "skeltalComponent");
        bossJointComponent->SetRadius(1.0f);
        DirectX::XMFLOAT3 bossJoint = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
        bossJointComponent->SetRelativeLocationDirect(bossJoint);

        OutputDebugStringA(("Actor::Initialize called. rootComponent_ use_count = " + std::to_string(rootComponent_.use_count()) + "\n").c_str());
    }

    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    void NotifyHit(Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
#if 0
        if (auto beam = dynamic_cast<Beam*>(otherActor))
        {// �r�[���ɓ���������A
            if (beam == lastHitBeam_)
            {// ��x�q�b�g��h������
                return;
            }

            // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (int)
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

    // �L�l�}�e�B�b�N���m�̓����蔻������m����
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override;

    //// �����������ɒʂ�֐�       // hitShapes.first�����g hitShapes.second������
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

    //�v���C���[���G
    bool SearchPlayer(DirectX::XMFLOAT3 playerPosition);

    //�ڕW�n�_�ݒ�
    void SetRandomTargetPosition();

    //�ڕW�n�_�ֈړ�   
    void MoveToTarget(float elapsedTime, float speedRate);

    //�ړI�n�փW�����v
    void JumpToTarget(float elapsedTime, float heightMax, float speedRate);

    //�ړ�����
    void Move(float elapsedTime, float vx, float vz, float speed);

    //��]����
    void Turn(float elapsedTime, float vx, float vz, float speed);

    //�������͏���
    void UpdateHorizontalVelocity(float elapsedTime);

    //�����ړ�����
    void UpdateHorizontalMove(float elapsedTime);

    //�������͍X�V����
    void UpdateVerticalVelocity(float elapsedTime);

    //�����ړ��X�V����
    void UpdateVerticalMove(float elapsedTime);

    //�^�[�Q�b�g�|�W�V�����擾
    DirectX::XMFLOAT3 GetTargetPosition() { return targetPosition; }

    //�^�[�Q�b�g�|�W�V�����ݒ�
    void SetTargetPosition(DirectX::XMFLOAT3 position) { targetPosition = position; }

    //�ő�HP�擾
    int GetMaxHP() { return maxHp; }

    //�U���͈͎擾
    float GetAttackRange() { return attackRange; }

    //HP�擾
    int GetHP() { return hp; }

    //HP�ݒ�
    void SetHP(int hp) { this->hp = hp; }

    //���݂̕K�E�Q�[�W�擾
    int GetCurrentSpecialGauge() { return static_cast<int>(specialGauge); }

    //���݂̕K�E�Q�[�W�ݒ�
    void SetCurrentSpecialGauge(float currentSpecial) { this->specialGauge = currentSpecial; }

    //�ő�K�E�Q�[�W�擾
    int GetMaxSpecialGauge() { return static_cast<int>(maxSpecialGauge); }

    //�`�ԕω��t���O�擾
    bool GetIsChange() { return isChange; }

    //�`�ԕω��t���O�ݒ�
    void SetIsChange(bool isChange) { this->isChange = isChange; }

    //�������擾
    float GetDistNear() { return distNear; }

    //�������擾
    float GetDistMid() { return distMid; }

    //������擾
    float GetDistFar() { return distFar; }

    //�ǐՎ��Ԏ擾
    float GetRunTimer() { return runTimer; }

    //�ǐՎ��Ԑݒ�
    void SetRunTimer(float runTimer) { this->runTimer = runTimer; }

    //�ːi�ő�^�C�}�[�擾
    float GetMaxChargeTimer() { return MaxChargeTimer; }

    //�ːi�^�C�}�[�擾
    float GetChargeTimer() { return chargeTimer; }

    //�ːi�^�C�}�[�ݒ�
    void SetChargeTimer(float chargeTimer) { this->chargeTimer = chargeTimer; }

    //�N�[���^�C�������擾
    bool GetIsCoolTime() { return isCoolTime; }

    //�N�[���^�C�������ݒ�
    void SetIsCoolTime(bool coolTime) { this->isCoolTime = coolTime; }

    //�ő�N�[���^�C���擾
    float GetMaxCoolTime() { return maxCoolTime; }
    //�ŏ��N�[���^�C���擾
    float GetMinCoolTime() { return minCoolTime; }

    //�N�[���^�C���擾
    float GetCoolTime() { return coolTime; }

    //�N�[���^�C���ݒ�
    void SetCoolTime(float coolTime) { this->coolTime = coolTime; }

    //�n�`�ϓ��ő�N�[���^�C���擾
    float GetMaxTerrainTime() { return maxTerrainTime; }

    //�n�`�ϓ��N�[���^�C���擾
    float GetTerrainTime() { return terrainTime; }

    //�n�`�ϓ��N�[���^�C���ݒ�
    void SetTerrainTime(float terrainTime) { this->terrainTime = terrainTime; }

    //�n�`�ϓ����o���邩�ǂ����擾
    bool GetCanTerrain() { return canTerrain; }

    //�n�`�ϓ����o���邩�ǂ����ݒ�
    void SetCanTerrain(bool canTerrain) { this->canTerrain = canTerrain; }

    //�K�E���o���邩�擾
    bool GetCanSpecial() { return canSpecial; }

    //�K�E���o���邩�ݒ�
    void SetCanSpecial(bool finishSpecial) { this->canSpecial = finishSpecial; }

    //�����ő�N�[���^�C���擾
    float GetMaxSummonTime() { return maxSummonTime; }

    //�����N�[���^�C���擾
    float GetSummonTime() { return summonTime; }

    //�����N�[���^�C���ݒ�
    void SetSummonTime(float summonTime) { this->summonTime = summonTime; }

    //�����ł��邩�擾
    bool GetCanSummon() { return canSummon; }

    //�����ł��邩�ݒ�
    void SetCanSummon(bool canSummon) { this->canSummon = canSummon; }

    //�W�����v�����擾
    bool GetIsJumping() { return isJumping; }

    //�W�����v�����ݒ�
    void SetIsJumping(bool isJumping) { this->isJumping = isJumping; }

    //20�ȏ�̃_���[�W���󂯂����擾
    bool GetIsStaggered() { return isStaggered; }

    //20�ȏ�̃_���[�W���󂯂����ݒ�
    void SetIsStaggered(bool isStaggered) { this->isStaggered = isStaggered; }

    //�_���[�W���󂯂��񐔎擾
    int GetDamageCount() { return damageCount; }

    //�_���[�W���󂯂��񐔐ݒ�
    void SetDamageCount(int damageCount) { this->damageCount = damageCount; }

    //�ŏ��̉��o���擾
    bool GetIsStartPerf() { return isStartPerf; }

    //�ŏ��̉��o���ݒ�
    void SetIsStartPerf(bool isStartPerf) { this->isStartPerf = isStartPerf; }

    //�ŏ��̉��o�œG�������n�߂邩�擾
    bool GetIsEnemyFall() { return isStartEnemyFall; }

    //�_���[�W�𔻒���J�n���邩�擾
    bool GetEnableHit() { return enableHit; }

    //�_���[�W�𔻒���J�n���邩�ݒ�
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
    // �����t���[���ł̃_���[�W���������
    bool hasHitThisFrame = false;

    //���G�͈�
    float searchRange = 5.0f;

    //�ڕW�n�_
    DirectX::XMFLOAT3 targetPosition{ 0.0f,0.0f,0.0f };

    //�ڕW�n�_��ݒ肷��͈�
    float territoryRange = 8.0f;

    //�ҋ@����
    float stateTimer = 0.0f;

    //TODO:02IMGUI�p
    bool isDetected = false;
    bool isAttacked = false;

    // particles �� �X�V���邩�ǂ���
    bool isIntegrateParticles = false;

    bool testFlag = false;

    //�ő�HP
    int maxHp = 210;

    //�U���͈�
    float attackRange = 15.0f;

    //���݂̕K�E�Q�[�W
    float specialGauge = 0;
    //�ő�K�E�Q�[�W
    const float maxSpecialGauge = 40;
    //�K�E�Z���o�������ǂ���
    bool canSpecial = false;

    //�`�ԕω��������ǂ���
    bool isChange = false;

    //������
    float distNear = 3.0f;
    //������
    float distMid = 8.0f;
    //������
    float distFar = 15.0f;

    //�ǐՎ���
    float runTimer = 0.0f;

    //�ːi�̗��ߍő厞��
    const float MaxChargeTimer = 2.0f;
    //�ːi�̗��ߎ���
    float chargeTimer = 0.0f;

    //�N�[���^�C������
    bool isCoolTime = false;

    //�ő�N�[���^�C��
    const float maxCoolTime = 4.0f;
    //�ŏ��N�[���^�C��
    const float minCoolTime = 3.0f;
    //�N�[���^�C��
    float coolTime = 0.0f;

    //�n�`�ϓ��ő�N�[���^�C��
    const float maxTerrainTime = 25.0f;
    //�n�`�ϓ��N�[���^�C��
    float terrainTime = 0.0f;
    //�n�`�ϓ����o���邩�ǂ���
    bool canTerrain = false;

    //�����ő�N�[���^�C��
    const float maxSummonTime = 5.0f;
    //�����N�[���^�C��
    float summonTime = 0.0f;
    //�����N�[���^�C������
    bool canSummon = true;

    //���C
    float friction = 0.5f;
    //�����x
    float acceleration = 1.0f;
    //�ő呬�x
    float maxMoveSpeed = 5.0f;
    //X���ړ��x�N�g��
    float moveVecX = 0.0f;
    //Z���ړ��x�N�g��
    float moveVecZ = 0.0f;
    //�󒆂̐���
    float airControl = 0.3f;
    //�ړ����x
    float moveSpeed = 3.0f;
    //��]���x
    float turnSpeed = DirectX::XMConvertToRadians(360);

    //�W�����v����
    bool isJumping = false;
    //�W�����v�o�ߎ���
    float jumpT = 0.0f;
    //�W�����v�n�_
    DirectX::XMFLOAT3 jumpStart = { 0,0,0 };
    //�W�����v�I�_
    DirectX::XMFLOAT3 jumpEnd = { 0,0,0 };
    //�W�����v�̒��_
    DirectX::XMFLOAT3 jumpPeak = { 0,0,0 };
    //�ŏI�I�ȏI�_
    DirectX::XMFLOAT3 lastTarget = { 0,0,0 };

    DirectX::XMFLOAT4 uiAngle = { 0,0,0,0 };

    //20�ȏ�̃_���[�W���󂯂���
    bool isStaggered = false;

    //�_���[�W���󂯂���
    int damageCount = 0;

    //�_���[�W������J�n���邩
    bool enableHit = false;
public:
    //�ŏ��̉��o���ǂ���
    bool isStartPerf = true;
    // �G�������Ă��邩�ǂ���
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
    // ���������Ԃ̏o��ꏊ
    DirectX::XMFLOAT3 GetJumpPosition()
    {
        return lastTarget;
    }
    // ���Ԃ��o�Ă��鎞��
    bool IsEnemyJumping()
    {
        return isJumping;
    }

};