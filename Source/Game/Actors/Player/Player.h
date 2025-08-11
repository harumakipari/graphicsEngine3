#pragma once
#include <stack>
#include <memory>
#include "Game/Actors/Base/Character.h"
#include "Engine/Input/GamePad.h"
#include "PlayerState.h"

//TODO :01 �����ύX����
// collider���Z�b�g����Ƃ��Ɉʒu�┼�a�A�ǉ������������蔻��̎�ނ�`���ăZ�b�g�ł���悤�ɂ���
//#include "Physics/Collider.h"
//
//#include "Physics/CollisionEvent.h"
//#include "Physics/Physics.h"
//#include "Core/ActorManager.h"


#include "Components/Controller/ControllerComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Effect/EffectComponent.h"

#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Item/HeldEnergyCore.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Managers/GameManager.h"
#include "Game/Managers/ItemManager.h"

#include "Core/ActorManager.h"
#include "Components/Audio/AudioSourceComponent.h"



enum class AnimationState
{
    Idle,
    Running,
    Attack,
    Attack_First,
    Jump_Start,
    Jump_Apex,
    Jump_Land,
    Jump_Recovering,
    Jump_Attack,
    Hit_Damaged,
    Emote,
};

class Player :public Character
{
public:
    enum class State
    {
        Idle,
        Running,
        StartCharge,
        FireBeam,
        FinishBeam,
        Attack,
        CantChargeBeam, // �`���[�g���A���̎��Ɏg�p����X�e�[�g
        CantMoveCharge,
    };

    // �X�e�[�g��ݒ肷��
    void SetState(Player::State state) { this->state = state; }
private:
    State state = Player::State::CantMoveCharge;
public:
    Player() = default;
    ~Player() = default;

    Player(const std::string& modelName) :Character(modelName)
    {
        // �J�v�Z���̓����蔻��𐶐�
        radius = 0.2f;
        height = 0.9f;
        mass = 50.0f;
        hp = maxHp;
        //PushState(std::make_shared<IdlingState>());
    }
    // �`��p�R���|�[�l���g��ǉ�
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    // �`��̊�_�@��
    std::shared_ptr<SkeltalMeshComponent> leftComponent;
    //std::shared_ptr<SceneComponent> leftComponent;
    std::shared_ptr<BoxComponet> boxLeftHitComponent;
    // �`��̊�_�@�E
    std::shared_ptr<SkeltalMeshComponent> rightComponent;
    std::shared_ptr<BoxComponet> boxRightHitComponent;

    // �_���[�W�p�̓����蔻��
    std::shared_ptr<SphereComponent> playerDamageLeft;
    std::shared_ptr<SphereComponent> playerDamageRight;

    void Initialize(const Transform& transform)override;

    // ���͂��I���ɂ��邩
    bool CanMove()override
    {
        // �`���[�W�r�[����Ԃ���Ȃ���������͂��I���ɂ���
        return state != State::StartCharge && state != State::CantMoveCharge;
    }

    std::shared_ptr<EffectComponent> effectChargeComponent;
    std::shared_ptr<SphereComponent> sphereRightComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<SphereComponent> sphereLeftComponent;
    std::shared_ptr<MovementComponent> movementComponent;
    std::shared_ptr<AudioSourceComponent> beamChargeAudioComponent;
    std::shared_ptr<AudioSourceComponent> beamLaunchAudioComponent;
    std::shared_ptr<AudioSourceComponent> itemAudioComponent;
    DirectX::XMFLOAT3 GetVelocity() { return velocity; }

    void Update(float elapsedTime)override;

    // �x���X�V����
    void LateUpdate(float elapsedTime)override;

    //�������̃��C�̃X�e�[�W�Ƃ̌�_���擾����֐�
    DirectX::XMFLOAT3 GetStageIntersect() const { return intersectStagePosition; }

    void DrawImGuiDetails()override;

    void Finalize()override
    {
        leftItemCount = 0;
        rightItemCount = 0;
    }
private:
    // �r�[�����`���[�W����֐�
    void TryStartCharge();

    // �r�[���𔭎˂���֐�
    void FireBeam();

    void Move(float elapsedTime)override;

    void Turn(float elapsedTime);
public:
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec();

    void HandleInput(GamePad& pad);

    void PushState(std::shared_ptr<PlayerState> state);

    void PopState();

    void ChangeState(std::shared_ptr<PlayerState> state);

    //�v���C���[�̍��̃X�e�[�g���擾����
    std::shared_ptr<PlayerState> GetCurrentState()
    {
        if (!stateStack.empty())
        {
            return stateStack.top();
        }
        return 0;
    }

    //�����������̏���
    void Hit();

    // �����A�C�e����S�ăN���A����
    void HasItemReset()
    {
        // �����蔻��͈̔͂�߂�
        boxLeftHitComponent->ResizeBox(firstHalfBoxExtent.x, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxLeftHitComponent->SetRelativeLocationDirect(firstLeftBoxPosition);
        boxRightHitComponent->ResizeBox(firstHalfBoxExtent.x, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxRightHitComponent->SetRelativeLocationDirect(firstRightBoxPosition);
        // �J�E���g��߂�
        leftItemCount = 0;
        rightItemCount = 0;
        // �����ڂ̕`��̃X�P�[����߂�
        DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
        leftComponent->SetRelativeScaleDirect(leftScale);
        DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
        rightComponent->SetRelativeScaleDirect(rightScale);
        // player �̃_���[�W�̓����蔻���߂�
        //playerDamageLeft->SetRelativeLocationDirect(firstLeftDamagePosition);
        playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + radius), rightFirstPos.y, rightFirstPos.z));
        playerDamageLeft->ResizeSphere(firstDamageRadius);
        //playerDamageRight->SetRelativeLocationDirect(firstRightDamagePosition);
        playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightFirstPos.x + radius), rightFirstPos.y, rightFirstPos.z));
        playerDamageRight->ResizeSphere(firstDamageRadius);
    }

    // ���̃A�C�e���X�g�b�N�Ƀ_���[�W��K������
    void ApplyDamageToLeft(int damage)
    {
        if (damage <= 0)
        {
            return;
        }

        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "Before:leftItemCount : %d,PlayerHp : %d\n",
            leftItemCount, hp);
        OutputDebugStringA(debugBuffer);

        // ���U���g�p::�v���C���[�̔�e��
        GameManager::CallPlayerDamaged();
        BlinkInit();

        int leftDamage = damage;
        int leftRestDamage = leftDamage - leftItemCount;
        leftItemCount -= leftDamage;
        if (leftItemCount < 0)
        {
            leftItemCount = 0;
        }
        if (leftRestDamage > 0)
        {
            this->hp -= leftRestDamage;
        }
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "After:leftItemCount : %d,PlayerHp : %d\n",
            leftItemCount, hp);
        OutputDebugStringA(debugBuffer);

        //int leftDamage = std::min<int>(damage, leftItemCount);
        //leftItemCount -= leftDamage;
        //damage -= leftDamage;
        //if (damage > 0)
        //{// �c��͖{�̂ɓK��
        //    this->hp -= damage;
        //}
        // �����蔻�������������


        DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
        leftScale.x += leftItemCount * scaleBigSize;
        leftScale.y += leftItemCount * scaleBigSize;
        leftScale.z += leftItemCount * scaleBigSize;
        // �v���C���[�̍��̃A�C�e���̎��W�̓����蔻���傫������
        float leftBoxWidth = -leftFirstPos.x + leftScale.x * firstPlayerSideSize;
        boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, firstLeftBoxPosition.y, firstLeftBoxPosition.z));
        // �v���C���[�̍��̃_���[�W�����蔻���傫������
        float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player �̔��a�͑����Ȃ�
        playerDamageLeft->ResizeSphere(leftDamageRadius);
        playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + leftDamageRadius), rightFirstPos.y + leftDamageRadius, rightFirstPos.z));
        // �X�P�[�������������鏈��
        afterLeftScale = leftScale.x;
        isLeftShrinking = true;
        shrinkElapsedTimeLeft = 0.0f;
        currentLeftScale = leftComponent->GetRelativeScale().x;
    }

    // �E�̃A�C�e���X�g�b�N�Ƀ_���[�W��K������
    void ApplyDamageToRight(int damage)
    {
        if (damage <= 0)
        {
            return;
        }
        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "Before:rightItemCount : %d,PlayerHp : %d\n",
            rightItemCount, hp);
        OutputDebugStringA(debugBuffer);

        // ���U���g�p::�v���C���[�̔�e��
        GameManager::CallPlayerDamaged();
        BlinkInit();
        int rightDamage = damage;
        int rightRestDamage = rightDamage - rightItemCount;
        rightItemCount -= rightDamage;
        if (rightItemCount < 0)
        {
            rightItemCount = 0;
        }
        if (rightRestDamage > 0)
        {
            this->hp -= rightRestDamage;
        }

        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "After:rightItemCount : %d,PlayerHp : %d\n",
            rightItemCount, hp);
        OutputDebugStringA(debugBuffer);

        //int rightDamage = std::min<int>(damage, rightItemCount);
        //damage -= rightDamage;

        //if (damage > 0)
        //{// �c��͖{�̂ɓK��
        //    this->hp -= damage;
        //}

        // �����蔻�������������
        DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
        rightScale.x += rightItemCount * scaleBigSize;
        rightScale.y += rightItemCount * scaleBigSize;
        rightScale.z += rightItemCount * scaleBigSize;
        // �v���C���[�̉E�̃A�C�e���̎��W�̓����蔻���傫������
        float rightBoxWidth = rightFirstPos.x + rightScale.x * firstPlayerSideSize;
        boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxRightHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightBoxWidth) * 0.5f, firstRightBoxPosition.y, firstRightBoxPosition.z));
        // �v���C���[�̉E�̃_���[�W�����蔻���傫������
        float rightDamageRadius = (rightScale.x * firstPlayerSideSize) * 0.5f; // player �̔��a�͑����Ȃ�
        playerDamageRight->ResizeSphere(rightDamageRadius);
        playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3(rightFirstPos.x + rightDamageRadius, rightFirstPos.y + rightDamageRadius, rightFirstPos.z));


        // �X�P�[�������������鏈��
        isRightShrinking = true;
        shrinkElapsedTimeRight = 0.0f;
        currentRightScale = rightComponent->GetRelativeScale().x;
        afterRightScale = rightScale.x;
    }

    // �v���C���[�̖{�̂ɒ��ڃ_���[�W����
    void ApplyDirectHpDamage(int damage)
    {
        this->hp -= damage;
    }

    // �X�P�[����߂�����
    void UpdateItemVisualShrink(float deltaTime)
    {
        //// ��b�Ŗ߂�
        //const float shrinkDuration = 1.0f;

        //if (isLeftShrinking)
        //{// ���̏���
        //    shrinkElapsedTimeLeft += deltaTime;
        //    float t = std::min<float>(shrinkElapsedTimeLeft / shrinkDuration, 1.0f);
        //    float scale = std::lerp(currentLeftScale, afterLeftScale, t);
        //    DirectX::XMFLOAT3 newScale = { scale,scale,scale };
        //    leftComponent->SetRelativeScaleDirect(newScale);

        //    if (t >= 1.0f)
        //    {
        //        isLeftShrinking = false;
        //    }
        //}

        //if (isRightShrinking)
        //{// �E�ɓ���������
        //    shrinkElapsedTimeRight += deltaTime;
        //    float t = std::min<float>(shrinkElapsedTimeRight / shrinkDuration, 1.0f);
        //    float scale = std::lerp(currentRightScale, afterRightScale, t);
        //    DirectX::XMFLOAT3 newScale = { scale,scale,scale };
        //    rightComponent->SetRelativeScaleDirect(newScale);

        //    if (t >= 0.0f)
        //    {
        //        isRightShrinking = false;
        //    }
        //}
    }


public:
    //������ւ̗�
    float jumpPower = 5.0f;

    //����̃m�[�h�ԍ�
    //size_t nodeAttackIndex = 153; //"VB root Weapon"
    size_t nodeAttackIndex = 0; //"VB root Weapon"

    // ���̃A�C�e���ۗL�����擾����֐�
    int GetLeftItemCount() const { return leftItemCount; }
    // �E�̃A�C�e���ۗL�����擾����֐�
    int GetRightItemCount() const { return rightItemCount; }

    void ResetHitFlags()
    {
        hitLeftThisFrame = false;
        hitRightThisFrame = false;
        currentFrameDamage = 0;
        hasDamageThisFrame = false;
    }
private:
    GamePad pad;
    //���̓V�ӂ̃m�[�h�ԍ�
    size_t nodeTopIndex = 126;   //"hair_top_mid_01"
    //���̉��̃m�[�h�ԍ�
    size_t nodeBottomIndex = 146;    //"ik_foot_root"
    //�v���C���[�̃X�e�[�g
    std::stack<std::shared_ptr<PlayerState>> stateStack;

    // �v���C���[���������Ă���A�C�e��
    //std::vector<std::shared_ptr<HeldEnergyCore>> hasLeftItems;
    //std::vector<std::shared_ptr<HeldEnergyCore>> hasRightItems;
    // �����̃A�C�e����
    int leftItemCount = 0;
    // �E���̃A�C�e����
    int rightItemCount = 0;
    // player Side �̒��a�̏����l
    float firstPlayerSideSize = 0.35f;
    //float firstPlayerSideSize = 0.5f;
    // �ǂꂭ�炢 player side ���傫���Ȃ��Ă�����
    float scaleBigSize = 0.2f;
    // �ŏ��̍��E�̓����蔻��̑傫��
    //DirectX::XMFLOAT3 firstHalfBoxExtent = { 0.2f,0.45f,0.2f };
    DirectX::XMFLOAT3 firstHalfBoxExtent = { 0.5f,0.45f,0.7f };
    // �ŏ��̍��E�̃A�C�e���̎��W�̓����蔻��̈ʒu
    DirectX::XMFLOAT3 firstLeftBoxPosition = { -0.5f , -0.4f, 0.5f };
    DirectX::XMFLOAT3 firstRightBoxPosition = { 0.5f , -0.4f, 0.5f };
    //DirectX::XMFLOAT3 firstLeftBoxPosition = { -radius , 0.0f, 0.0f };
    //DirectX::XMFLOAT3 firstRightBoxPosition = { radius , 0.0f, 0.0f };
    // �ŏ��̍��E�̃_���[�W�̓����蔻��̈ʒu
    DirectX::XMFLOAT3 firstLeftDamagePosition = { -radius , 0.0f, 0.0f };
    DirectX::XMFLOAT3 firstRightDamagePosition = { radius , 0.0f, 0.0f };
    float firstDamageRadius = 0.01f;
    // �A�C�e���̔��a
    float itemRadius = 0.5f;
    // �A�C�e�������������ɍ��̃^���N�����k����t���O
    bool isLeftShrinking = false;
    // �A�C�e�������������ɍ��̃^���N�����k����o�ߎ���
    float shrinkElapsedTimeLeft = 0.0f;
    // �A�C�e��������O�̃X�P�[��
    float currentLeftScale = 0.0f;
    // �A�C�e�������������ƑO�̃X�P�[��
    float afterLeftScale = 0.0f;
    // �A�C�e�������������ɉE�̃^���N�����k����t���O
    bool isRightShrinking = false;
    // �A�C�e�������������ɉE�̃^���N�����k����o�ߎ���
    float shrinkElapsedTimeRight = 0.0f;
    // �A�C�e��������O�̃X�P�[��
    float currentRightScale = 0.0f;
    // �A�C�e�������������ƑO�̃X�P�[��
    float afterRightScale = 0.0f;
    // �v���C���[�� eraseInArea ���G����
    float invisibleTime = 0.0f;
    // �v���C���[�� boss ���G����
    float bossInvisibleTime = 0.0f;
    // �v���C���[�̌��݂̃X�s�[�h
    float currentSpeed = 5.0f;
    // �v���C���[�̓A�C�e���������Ă��Ȃ��Ƃ��̃X�s�[�h
    float noItemSpeed = 10.0f;
    // �v���C���[�� Max �X�s�[�h
    float maxSpeed = 5.0f;
    // �v���C���[�� Min �X�s�[�h
    float minSpeed = 2.0f;
    // �v���C���[�̌��݂̉�]�X�s�[�h
    float currentTurnSpeed = 720.0f;
    // �v���C���[�� Max ��]�X�s�[�h
    float maxTurnSpeed = 720.0f;
    // �v���C���[�� Min ��]�X�s�[�h
    float minTurnSpeed = 90.0f;
    // ���ɉ�����ꂽ�A�C�e�������m�F����
    PickUpItem* lastHitPickUpItem = nullptr;
    // �v���C���[�̃}�b�N�XHP
    int maxHp = 20;
    bool isIdleEnd = false;
public:
    // ���G���Ԓ����ǂ���
    bool IsInvincible() const { return invisibleTime > 0.0f; }
    // ���G���ԊԊu�ݒ�
    void SetInvincible(float maxInvincibleTime = 3.0f) { invisibleTime = maxInvincibleTime; }

    // ���G���Ԓ����ǂ���
    bool IsBossInvincible() const { return bossInvisibleTime > 0.0f; }
    // ���G���ԊԊu�ݒ�
    void SetBossInvincible(float maxInvincibleTime = 3.0f) { bossInvisibleTime = maxInvincibleTime; }

    // EraseInArea �Ŏg�p
    // ���̃t���[���œK�����閳�G���Ԃ̃t���O�𗧂Ă�
    bool applyInvincibilityNextFrame = false;
    // ���񂾂��_���[�W�����Z����������
    bool hasDamageThisFrame = false;
    // �_���[�W�L�^
    bool hitLeftThisFrame = false;
    bool hitRightThisFrame = false;
    int currentFrameDamage = 0;

    DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 prePosition = { 0.0f,0.0f,0.0f };

    int leftItemMax = 15;
    int rightItemMax = 15;

    // ����̃T�C�Y
    DirectX::XMFLOAT3 leftFirstPos = { -0.5f,-0.5f,0.2f };
    DirectX::XMFLOAT3 rightFirstPos = { 0.5f,-0.5f,0.2f };

private:
    // �v���C���[��e���̓_��
    float hitBlinkElapsed = 0.0f;
    float hitBlinkInterval = 0.1f;
    float hitBlinkTotalTime = 1.5f;
    bool isHitBlinking = false;
    bool isRed = false;

    void BlinkInit()
    {
        isHitBlinking = true;
        hitBlinkElapsed = 0.0f;
    }

    void SetBlinkColor(bool isRed)
    {
        if (isRed)
        {
            color = { 1.0f,0.2f,0.2f };
        }
        else
        {
            color = { 1.0f,1.0f,1.0f };
        }
    }

    DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };

    // 
    bool onceFrag = false;

    // ���E�̐F
    DirectX::XMFLOAT3 firstColor = { 0.302f,0.910f,1.0f };
    DirectX::XMFLOAT3 secondColor = { 0.0f, 0.85f, 0.55f };
    DirectX::XMFLOAT3 thirdColor = { 0.0f, 1.0f, 0.4f };
    DirectX::XMFLOAT3 FourthColor = { 0.5f, 0.4f, 1.0f };
    DirectX::XMFLOAT3 FinalColor = { 1.0f,0.239f,1.0f };

    // ���E�̃A�C�e�����ɂ���ĐF��ύX����
    void ItemColor(float deltaTime)
    {
        elapsedTime_ += deltaTime;

        // ���`��Ԋ֐�
        auto LerpColor = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t) -> DirectX::XMFLOAT3
            {
                t = std::clamp(t, 0.0f, 1.0f);
                return {
                    std::lerp(a.x, b.x, t),
                    std::lerp(a.y, b.y, t),
                    std::lerp(a.z, b.z, t)
                };
            };

        static const std::vector<std::pair<int, DirectX::XMFLOAT3>> colorStops = {
            {0,  {0.302f, 0.910f, 1.0f}},   
            {4,  {0.0f,   0.85f,  0.55f}},  
            {7,  {0.0f,   1.0f,   0.4f}},   
            {10, {0.5f,   0.4f,   1.0f}},   
            {13, {1.0f,   0.239f, 1.0f}},   
            {15, {1.0f,   0.239f, 1.0f}},   
        };

        auto GetColor = [&](int count) -> DirectX::XMFLOAT3
            {
                // ���
                if (count >= colorStops.back().first)
                    return colorStops.back().second;

                for (size_t i = 0; i < colorStops.size() - 1; ++i)
                {
                    auto [startCount, startColor] = colorStops[i];
                    auto [endCount, endColor] = colorStops[i + 1];

                    if (count >= startCount && count < endCount)
                    {
                        float t = static_cast<float>(count - startCount) / (endCount - startCount);
                        return LerpColor(startColor, endColor, t);
                    }
                }

                return colorStops.front().second; 
            };

        leftComponent->model->cpuColor = GetColor(leftItemCount);
        rightComponent->model->cpuColor = GetColor(rightItemCount);
    }


    // MAX�A�C�e���̎��ɐF���R���R���ς��
    DirectX::XMFLOAT3 MaxColorChange(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
        float m = v - c;

        float r, g, b;
        if (h < 1.0f / 6.0f) { r = c; g = x; b = 0; }
        else if (h < 2.0f / 6.0f) { r = x; g = c; b = 0; }
        else if (h < 3.0f / 6.0f) { r = 0; g = c; b = x; }
        else if (h < 4.0f / 6.0f) { r = 0; g = x; b = c; }
        else if (h < 5.0f / 6.0f) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        return { r + m, g + m, b + m };
    }
private:
    float elapsedTime_ = 0.0f;
};