#include "ActionDerived.h"
#include "RiderEnemy.h"
#include "Game/Actors/Player/Player.h"
#include "EnemyMath.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Scenes/MainScene.h"
#include "Engine/Scene/Scene.h"
//TODO:Behavior �A�N�V�����ǉ�

float animationBaseSpeed = 1.0f;

// �ŏ��̉��o
ActionBase::State StartPerfAction::Run(float elapsedTime)
{
    static bool isJumpEnd = false;      //���n��������
    static bool Jumping = false;      //�W�����v��������
    static float jumpTimer = 0.0f;      //�W�����v���̃^�C�}�[
    static float startTimer = 0.0f;
    static bool isIdle = false;
    static bool playAudio = false;

    switch (step)
    {
    case 0:
        isJumpEnd = false;
        Jumping = true;
        jumpTimer = 0.0f;
        startTimer = 0.0f;
        isIdle = false;
        playAudio = false;

        DirectX::XMFLOAT3 targetPos = { 0,0,0 };
        owner->SetTargetPosition(targetPos);
        owner->skeltalMeshComponent->SetIsCastShadow(false);
        owner->SetAnimationRate(animationBaseSpeed);

        //�A�j���[�V�����Đ�
        //owner->PlayAnimation("Idle");
        owner->PlayAnimation("Jump");
        step++;
        break;
    case 1:
        startTimer += elapsedTime;

        if (startTimer >= 2.0f)
        {
            owner->UpdateVerticalVelocity(elapsedTime);
            owner->UpdateVerticalMove(elapsedTime);
            owner->isStartEnemyFall = true;
        }

        float threshold = 1.0f; // ���e�덷�i���a�j

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ���ʂ̋������v�Z
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // Y���̍�������
        bool isOnGround = pos.y <= 0.0f;
        bool canJumpEnd = pos.y <= 9.0f;

        owner->MoveToTarget(elapsedTime, 0);

        //���n�Ɉڂ�(���n�����ꂢ�Ɍ�����悤�ɂ�����ƍ��߂Ŕ��肷��)
        if (canJumpEnd && !isJumpEnd)
        {
            owner->PlayAnimation("JumpEnd", false);
            owner->skeltalMeshComponent->SetIsCastShadow(true);
            isJumpEnd = true;
        }

        if (horizontalDistSq <= threshold * threshold && isOnGround)
        {
            owner->SetIsJumping(false);
            //owner->SetIsCoolTime(true);
            owner->velocity = { 0,0,0 };
            owner->SetPosition({ pos.x,0.0f,pos.z });
            if (!playAudio)
            {
                owner->landingAudioComponent->Play();
                playAudio = true;
            }
        }

        if (isJumpEnd && !owner->GetAnimationController()->IsPlayAnimation() && !isIdle)
        {
            owner->PlayAnimation("Idle");
            jumpTimer = 0.0f;
            owner->SetIsStartPerf(false);
            // �G�������I������̂Ł@false �ɂ���
            owner->isStartEnemyFall = false;
            isIdle = true;
        }

        if (GameManager::GetGameTimerStart())
        {
            owner->SetAnimationRate(animationBaseSpeed);
            isJumpEnd = false;
            playAudio = false;
            step = 0;
            return ActionBase::State::Complete;
        }

        break;
    }
    return ActionBase::State::Run;
}

// �Ђ��
ActionBase::State DamageAction::Run(float elapsedTime)
{
    switch (step)
    {
    case 0:
        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Damage", false);

        step++;
        break;
    case 1:
    {
        //�A�j���[�V�������I�����Ă���Ƃ�
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            owner->SetIsStaggered(false);
            step = 0;
            return ActionBase::State::Complete;
        }
        break;
    }
    }
    return ActionBase::State::Run;
}

// ��ԕω�
ActionBase::State ChangeAction::Run(float elapsedTime)
{
    static float timer = 3.0f;
    switch (step)
    {
    case 0:
        //�A�j���[�V�����Đ�

        step++;
        break;
    case 1:
        timer -= elapsedTime;
        //�A�j���[�V�������I�����Ă���Ƃ�
        //if (!owner->GetAnimationController()->IsPlayAnimation())
        if (timer <= 0.0f)
        {
            //��ԕω��t���Otrue
            owner->SetIsChange(true);
            timer = 3.0f;
            step = 0;
            //������Ԃ�
            return ActionBase::State::Complete;
        }
        break;
    }
    return ActionBase::State::Run;
}

// �K�E�Z
ActionBase::State SpecialAction::Run(float elapsedTime)
{
    switch (step)
    {
    case 0:
        owner->SetCanSpecial(true);
        // �K�E�Z�̉����Đ�
        owner->specialAudioComponent->Play();
        if (owner->GetCanTerrain())
        {
            owner->SetCanTerrain(false);
        }

        //�G�t�F�N�g�p�n���h��
        handler.Clear();
        handler.SetEasing(EaseType::Linear, 0.0f, 3.0f, 3.0f);
        handler.SetEasing(EaseType::Linear, 3.0f, 5.0f, 0.5f);
        handler.SetEasing(EaseType::OutExp, 5.0f, 1.0f, 1.0f);

        easeY = 3.0f;

        yEasing.Clear();
        yEasing.SetWait(3.0f);
        yEasing.SetEasing(EaseType::OutSine, 3.0f, 4.0f, 0.5f);
        yEasing.SetEasing(EaseType::OutSine, 4.0f, 0.0f, 1.0f);
        yEasing.SetCompletedFunction([&]() {
            if (MainScene* scene = dynamic_cast<MainScene*>(Scene::GetCurrentScene()))
            {
                DirectX::XMFLOAT3 pos, forward;
                pos = owner->GetPosition();
                forward = owner->GetForward();

                XMVECTOR Pos = XMLoadFloat3(&pos);
                XMVECTOR Forward = XMLoadFloat3(&forward);

                XMVECTOR Target = Pos + Forward * 2.0f;
                XMStoreFloat3(&pos, Target);
                pos.y = easeY;

                scene->effectSystem->SpawnEmitter(1, pos, 0);
            }
            }, true);

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Special", false);

        step++;
        break;
    case 1:
        //�A�j���[�V�������I�����Ă���Ƃ�
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            //�K�E�Z����
            GameManager::GetBuildingManager()->SwitchToBossBuilding();
            owner->SetCanSpecial(false);
            owner->SetCurrentSpecialGauge(0);
            owner->SetIsCoolTime(true);
            step = 0;
            //������Ԃ�
            return ActionBase::State::Complete;
        }

        //TODO:86_�K�E���Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            owner->SetIsStaggered(false);
            //owner->SetCanSpecial(false);
            //owner->SetCurrentSpecialGauge(0);
            //owner->SetIsCoolTime(true);
            //step = 0;
            //return ActionBase::State::Failed;
        }

        //�G�t�F�N�g�X�V

        float power = 0.1f;
        handler.Update(power, elapsedTime);
        yEasing.Update(easeY, elapsedTime);
        if (MainScene* scene = dynamic_cast<MainScene*>(Scene::GetCurrentScene()))
        {
            DirectX::XMFLOAT3 pos, forward;
            pos = owner->GetPosition();
            forward = owner->GetForward();

            XMVECTOR Pos = XMLoadFloat3(&pos);
            XMVECTOR Forward = XMLoadFloat3(&forward);

            XMVECTOR Target = Pos + Forward * 2.0f;
            XMStoreFloat3(&pos, Target);
            pos.y = easeY;

            scene->effectSystem->SpawnEmitter(0, pos, power);
        }

        break;
    }
    return ActionBase::State::Run;
}

// �n�`�ϓ�
ActionBase::State TerrainAction::Run(float elapsedTime)
{
    static bool isJumpPre = false;      //�\�����쒆������
    static bool isJumpEnd = false;      //���n��������
    static bool Jumping = false;      //�W�����v��������
    static float jumpPreTimer = 0.0f;   //�\�����쒆�̃^�C�}�[
    static float jumpEndTimer = 0.0f;   //���n���̃^�C�}�[
    static float jumpTimer = 0.0f;      //�W�����v���̃^�C�}�[
    static bool canComplete = false;    //�r�������̃t���O

    //�ǂ̃p�^�[�����I��p
    static int index = 0;

    const char* names[] = {
        "NormalBill1",
        "NormalBill2",
        "NormalBill3",
        "NormalBill4",
        "NormalBill5",
    };

    switch (step)
    {
    case 0:
    {
        //�o���`�����I
        //index = rand() % ARRAYSIZE(names);

        //�o���`���ɉ����Ĕ��ł����ꏊ���w��
        XMFLOAT3 playerPos = owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor")->GetPosition();
        XMVECTOR PlayerPos = XMLoadFloat3(&playerPos);

        float farLengthSq = 0.0f;
        XMVECTOR FarTarget{};
        for (int i = 0; i < ARRAYSIZE(names); i++)
        {
            XMFLOAT3 moveTarget = GameManager::GetBuildingManager()->GetMovePos(names[i]);
            XMVECTOR MoveTarget = XMLoadFloat3(&moveTarget);
            float lengthSq = XMVectorGetX(XMVector3LengthSq(MoveTarget - PlayerPos));
            if (lengthSq > farLengthSq)
            {
                farLengthSq = lengthSq;
                FarTarget = MoveTarget;
                index = i;
            }
        }
        XMFLOAT3 target;
        XMStoreFloat3(&target, FarTarget);

        owner->SetTargetPosition(target);

        //������
        jumpPreTimer = 0.0f;
        jumpEndTimer = 0.0f;
        jumpTimer = 0.0f;
        isJumpPre = false;
        isJumpEnd = false;
        Jumping = false;
        canComplete = false;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("JumpPre", false);
        //�\������Ɉڂ点��
        isJumpPre = true;

        step++;
        break;
    }
    case 1:
        jumpPreTimer += elapsedTime;
        float threshold = 1.0f; // ���e�덷�i���a�j

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ���ʂ̋������v�Z
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // �����ɉ����� isOnGround �̔���臒l�𒲐�
        float groundThreshold = 0.0f;

        bool isOnGround = pos.y <= groundThreshold;
        bool canJumpEnd = pos.y <= 8.0f;

        //�\������̃^�C�}�[�����ȏ�ŃW�����v����Ɉڂ�
        if (!Jumping)
        {
            if (jumpPreTimer >= 3.5f * 0.5f)
            {
                owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                owner->PlayAnimation("Jump");
                Jumping = true;
            }
            else
            {
                owner->MoveToTarget(elapsedTime, 0);
            }
        }

        //�w��̏ꏊ�ɔ��ł���
        if (isJumpPre && Jumping)
        {
            owner->JumpToTarget(elapsedTime, 10, 2.0);
            jumpTimer += elapsedTime;
        }
        
        if (isJumpEnd)
        {
            jumpEndTimer += elapsedTime;
        }

        //���n�Ɉڂ�(���n�����ꂢ�Ɍ�����悤�ɂ�����ƍ��߂Ŕ��肷��)
        if (canJumpEnd && !isJumpEnd && jumpTimer >= 1.0f)
        {
            owner->PlayAnimation("JumpEnd", false);
            isJumpEnd = true;
        }

        if (isJumpEnd && isOnGround)
        {
            if (isJumpPre)
            {
                isJumpPre = false;
                owner->landingAudioComponent->Play();
            }
            owner->SetIsJumping(false);
            owner->SetCanTerrain(false);
            owner->SetIsCoolTime(true);
            owner->SetTerrainTime(0.0f);
            owner->velocity = { 0,0,0 };
            owner->SetPosition({ pos.x,0.0f,pos.z });
            owner->SetEnableHit(true);
            //owner->velocity = { 0, 0, 0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
            //isJumpPre = false;
        }

        //�w��̈ʒu�ɗ����珉����
        if (owner->GetCanTerrain() && horizontalDistSq <= threshold * threshold && isOnGround)		// ��������FXZ������ threshold �ȓ� & Y�����n�ʋ߂�
        {
            //isJumpPre = false;
            //owner->SetIsJumping(false);
            //owner->SetCanTerrain(false);
            //owner->SetIsCoolTime(true);
            //owner->SetTerrainTime(0.0f);
            //owner->velocity = { 0,0,0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
        }

        //���n���̏���
        if (isJumpEnd && !isJumpPre)
        {
            //�����������u�ԂɃr�������܂��悤�ɔ��������Ă܂�
            if (jumpEndTimer >= 1.2f && !canComplete)
            {
                owner->SetEnableHit(false);
                //TODO::�r������
                GameManager::GetBuildingManager()->Spawn(names[index]);
                canComplete = true;
            }
            
            //������Ԃ�
            if (!owner->GetAnimationController()->IsPlayAnimation() && canComplete)
            {
                owner->SetEnableHit(false);
                step = 0;
                jumpTimer = 0.0f;
                canComplete = false;
                isJumpEnd = false;
                jumpPreTimer = 0.0f;
                jumpEndTimer = 0.0f;
                owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                return ActionBase::State::Complete;
            }
        }

        //TODO:86_�n�`�ϓ����Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered() && !owner->GetIsJumping())
        {
            isJumpPre = false;
            owner->SetIsJumping(false);
            owner->SetCanTerrain(false);
            owner->SetIsCoolTime(true);
            owner->SetTerrainTime(0.0f);
            owner->velocity = { 0,0,0 };
            owner->SetEnableHit(false);
            step = 0;
            jumpTimer = 0.0f;
            canComplete = false;
            isJumpEnd = false;
            jumpPreTimer = 0.0f;
            jumpEndTimer = 0.0f;
            DirectX::XMFLOAT3 ownerPos = owner->GetPosition();
            if (ownerPos.y > 0.0f)
            {
                owner->SetPosition({ ownerPos.x,0,ownerPos.z });
            }
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Failed;
        }

        break;
    }
    return ActionBase::State::Run;
}

// �ǐՍs��
ActionBase::State PursuitAction::Run(float elapsedTime)
{
    const float animationFps = 120.0f;
    static float frameCounter = 0.0f;
    const int totalFrame = 226;

    int currentFrame = static_cast<int>(frameCounter);
    currentFrame %= totalFrame; // ���[�v�Đ��ɔ����ăt���[������܂�Ԃ�

    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    switch (step)
    {
    case 0:
        //�ڕW�n�_���v���C���[�ɐݒ�
        owner->SetTargetPosition(player->GetPosition());

        frameCounter = 0.0f;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Walk");

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;
        //�ڕW�n�_���v���C���[�ɐݒ�
        owner->SetTargetPosition(player->GetPosition());
        //�ړI�n�ֈړ�
        if ((currentFrame >= 0.0f && currentFrame <= 40.0f) || (currentFrame >= 100.0f && currentFrame <= 130.0f))
        {
            if ((currentFrame >= 38.0f && currentFrame <= 40.0f) || (currentFrame >= 128.0f && currentFrame <= 130.0f))
            {
                owner->walkAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            }
            owner->MoveToTarget(elapsedTime, 0.3f);
        }
        else if ((currentFrame >= 65.0f && currentFrame <= 85.0f) || (currentFrame >= 160.0f && currentFrame <= 210.0f))
        {
            owner->MoveToTarget(elapsedTime, 0.8f);
        }
        else
        {
            owner->MoveToTarget(elapsedTime, 0.0f);
        }

        // �v���C���[�Ƃ̋������v�Z
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();

        float vx = targetPosition.x - position.x;
        float vy = targetPosition.y - position.y;
        float vz = targetPosition.z - position.z;
        float dist = sqrtf(vx * vx + vy * vy + vz * vz);
        // �U���͈͂ɂ���Ƃ�
        if (dist < owner->GetAttackRange())
        {
            owner->walkAudioComponent->Stop();
            step = 0;
            // �ǐՐ�����Ԃ�
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Complete;
        }

        //TODO:86_�ǐՒ��Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            owner->walkAudioComponent->Stop();
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Failed;
        }

        break;

    }
    return ActionBase::State::Run;
}

// �ʏ�U��
ActionBase::State NormalAction::Run(float elapsedTime)
{
    //170~260�܂œ����蔻������
    const float animationFps = 60*1.5f;
    static float frameCounter = 0.0f;
    const int totalFrame = 226;

    int currentFrame = static_cast<int>(frameCounter);

    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)return ActionBase::State::Failed;

    switch (step)
    {
    case 0:
        owner->SetEnableHit(false);

        owner->SetAnimationRate(1.5f);

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Normal", false);

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;

        if (currentFrame >= 170 && currentFrame <= 250)
        {
            owner->SetEnableHit(true);
        }

        //�A�j���[�V�������I�����Ă���Ƃ�
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            owner->SetAnimationRate(1.0f);
            owner->SetEnableHit(false);
            step = 0;
            owner->SetIsCoolTime(true);
            //������Ԃ�
            return ActionBase::State::Complete;
        }

        //TODO:86_�ʏ�U�����Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            owner->SetAnimationRate(1.0f);
            owner->SetEnableHit(false);
            step = 0;
            return ActionBase::State::Failed;
        }

        break;
    }
    return ActionBase::State::Run;
}

#if 0
// �ːi�s��
ActionBase::State DashAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(ActorManager::GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    static bool charge = false;
    static bool animation = false;
    static float x = 0.0f;
    static float z = 0.0f;

    switch (step)
    {
    case 0:
        //������
        if (!charge)
        {
            owner->SetChargeTimer(owner->GetMaxChargeTimer());
            owner->SetAnimationRate(animationBaseSpeed);
            //�\���̃A�j���[�V�����Đ�
            owner->PlayAnimation("RotatePre", false);
            charge = true;
            animation = false;
        }

        //�������v���C���[�ɍ��킹��
        owner->SetChargeTimer(owner->GetChargeTimer() - elapsedTime);
        owner->SetTargetPosition(player->GetPosition());
        owner->MoveToTarget(elapsedTime, 0.0f);

        if (owner->GetChargeTimer() <= 0.0f && !owner->GetAnimationController()->IsPlayAnimation())
        {
            x = owner->GetTargetPosition().x;
            z = owner->GetTargetPosition().z;

            x = std::clamp(x, -17.0f, 17.0f);
            z = std::clamp(z, -12.0f, 12.0f);

            owner->SetTargetPosition({ x, player->GetPosition().y, z });

            // �ːi���̉����Đ�����i�������[�v�j
            owner->rushAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            //�ːi�̃A�j���[�V�����Đ�
            owner->PlayAnimation("Rotate");
            owner->SetAnimationRate(2.0f);
            owner->SetEnableHit(true);
            step++;
        }

        //TODO:86_�ːi���Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            //����~
            owner->rushAudioComponent->Stop();
            charge = false;
            animation = false;
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Failed;
        }

        break;
    case 1:
        // �ړI�n�_�܂ł�XZ���ʂł̋�������
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();
        float vx = targetPosition.x - position.x;
        float vz = targetPosition.z - position.z;
        float distSq = vx * vx + vz * vz;

        // �ړI�n�_�ֈړ�
        if (!animation)
        {
            owner->MoveToTarget(elapsedTime, 5.0f);
        }

        float radius = owner->radius + 0.0f;
        // �ړI�n�֒�����
        if (distSq < radius * radius)
        {
            owner->GetAnimationController()->RequestStopLoop();
            //owner->SetAnimationRate(10.0f);
            owner->velocity = { 0,0,0 };
            owner->SetPosition({ targetPosition.x,0.0f,targetPosition.z });
            owner->SetChargeTimer(owner->GetMaxChargeTimer());
            charge = false;
            owner->SetIsCoolTime(true);
            if (!animation && !owner->GetAnimationController()->IsPlayAnimation())
            {
                owner->SetEnableHit(false);
                owner->SetAnimationRate(1.5f);
                // �ːi���̉��̍Đ����~�߂�
                owner->rushAudioComponent->Stop();
                owner->PlayAnimation("RotateEnd",false);
                animation = true;
            }
        }
        
        //������Ԃ�
        if (animation && !owner->GetAnimationController()->IsPlayAnimation())
        {
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Complete;
        }

        //TODO:86_�ːi���Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            //����~
            owner->rushAudioComponent->Stop();
            charge = false;
            animation = false;
            owner->SetAnimationRate(animationBaseSpeed);
            owner->SetEnableHit(false);
            step = 0;
            return ActionBase::State::Failed;
        }

        break;
    }
    return ActionBase::State::Run;
}
#endif

//�ːi�����E��
ActionBase::State DashAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    static bool charge = false;
    static bool loopAnimation = false;
    static bool rotateEndAnime = false;
    static float x = 0.0f;
    static float z = 0.0f;
    static float chargeTimer = 0.0f;
    static bool rotatePreAnime = false;

    switch (step)
    {
    case 0:
        //������

        charge = false;
        loopAnimation = false;
        rotatePreAnime = false;
        rotateEndAnime = false;
        x = 0.0f;
        z = 0.0f;
        chargeTimer = 0.0f;

        chargeTimer = owner->GetMaxChargeTimer();
        owner->SetChargeTimer(chargeTimer);

        owner->SetAnimationRate(animationBaseSpeed);
        //�\���̃A�j���[�V�����Đ�
        owner->PlayAnimation("RotatePre", false);

        step++;
        break;
    case 1:
        //���ߎ��Ԍ��Z
        chargeTimer -= elapsedTime;
        owner->SetChargeTimer(chargeTimer);

        //���ߎ��Ԓ��͌������v���C���[�ɍ��킹��
        if (!charge)
        {
            owner->SetTargetPosition(player->GetPosition());
            owner->MoveToTarget(elapsedTime, 0.0f);
        }

        //���ߎ��Ԃ��I�������
        if (!charge && chargeTimer <= 0.0f && !owner->GetAnimationController()->IsPlayAnimation())
        {
            charge = true;
            x = owner->GetTargetPosition().x;
            z = owner->GetTargetPosition().z;

            x = std::clamp(x, -17.0f, 17.0f);
            z = std::clamp(z, -12.0f, 12.0f);

            owner->SetTargetPosition({ x, player->GetPosition().y, z });

            // �ːi���̉����Đ�����i�������[�v�j
            owner->rushAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            //�ːi�̃A�j���[�V�����Đ�
            owner->PlayAnimation("Rotate");
            owner->SetAnimationRate(2.0f);
            owner->SetEnableHit(true);
            loopAnimation = true;
        }

        // �ړI�n�_�܂ł�XZ���ʂł̋�������
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();
        float vx = targetPosition.x - position.x;
        float vz = targetPosition.z - position.z;
        float distSq = vx * vx + vz * vz;

        // �ړI�n�_�ֈړ�
        if (loopAnimation && charge)
        {
            owner->MoveToTarget(elapsedTime, 5.0f);
        }

        float radius = owner->radius + 0.0f;
        // �ړI�n�֒�����
        if (distSq < radius * radius)
        {
            owner->GetAnimationController()->RequestStopLoop();
            //owner->SetAnimationRate(10.0f);
            owner->velocity = { 0,0,0 };
            owner->SetPosition({ targetPosition.x,0.0f,targetPosition.z });
            owner->SetChargeTimer(owner->GetMaxChargeTimer());
            //charge = false;
            owner->SetIsCoolTime(true);
            if (loopAnimation && !owner->GetAnimationController()->IsPlayAnimation())
            {
                owner->SetEnableHit(false);
                owner->SetAnimationRate(1.5f);
                // �ːi���̉��̍Đ����~�߂�
                owner->rushAudioComponent->Stop();
                owner->PlayAnimation("RotateEnd",false);
                loopAnimation = false;
            }
        }

        //������Ԃ�
        if (!loopAnimation && !owner->GetAnimationController()->IsPlayAnimation())
        {
            charge = false;
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Complete;
        }

        //TODO:86_�ːi���Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            //����~
            owner->rushAudioComponent->Stop();
            charge = false;
            loopAnimation = false;
            owner->SetAnimationRate(animationBaseSpeed);
            owner->SetEnableHit(false);
            step = 0;
            return ActionBase::State::Failed;
        }
        break;
    }
    return ActionBase::State::Run;
}

// �����s��
ActionBase::State BombingAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    static float timer = 0.0f;                              //���e���������ꂢ�Ɍ����邽�߂̃^�C�}�[
    static std::vector<DirectX::XMFLOAT3> bombPositions;    //���e�̃|�W�V������ۑ����Ă�������
    const int bombCount = 8;                                //���e�̍ő吔
    const float bombRadius = 2.0f;                          //���e�̔��a
    const float minDistance = bombRadius * 2.0f;            //���e�Ɣ��e�̍ŏ�����(���e���m�����Ȃ��悤��)
    static bool ready = false;                              //��������������
    static bool isBombProduce = false;                      //���e�𐶐����邩����
    static float x = 0.0f;
    static float z = 0.0f;

    switch (step)
    {
    case 0:
    {
        //������
        isBombProduce = false;
        timer = 0.0f;
        ready = false;
        x = 0.0f;
        z = 0.0f;

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Bombing", false);
        step++;
        break;
    }
    case 1:
    {
        //������������Ȃ�������(��x�����ʂ�Ȃ��悤�ɂ��邽��)
        if (!ready)
        {
            bombPositions.clear(); // �O��̈ʒu���N���A

            for (int i = 0; i < bombCount; ++i)
            {
                bool validPosition = false;
                DirectX::XMFLOAT3 newPos;

                // �ő�100��܂Ŕ��e�̗L���Ȑݒu�ʒu�����s
                for (int attempt = 0; attempt < 100; ++attempt)
                {
                    // �v���C���[�̌��݈ʒu���擾
                    x = player->GetPosition().x;
                    z = player->GetPosition().z;

                    // �v���C���[�̎��́}8�͈̔͂Ƀ����_���ȍ��W�𐶐�
                    float randomX = Mathf::RandomRange(-10.0f, 10.0f) + x;
                    float randomZ = Mathf::RandomRange(-10.0f, 10.0f) + z;

                    //// ���W���X�e�[�W�͈͂ɐ���
                    //randomX = std::clamp(randomX, -18.0f, 18.0f);
                    //randomZ = std::clamp(randomZ, -12.0f, 12.0f);

                    // �X�e�[�W�͈͊O�Ȃ�X�L�b�v�i�␳���Ȃ��j
                    if (randomX < -18.0f || randomX > 18.0f || randomZ < -12.0f || randomZ > 12.0f)
                        continue;

                    // ����(Y)�̓����_���ɐݒ�
                    float randomY = Mathf::RandomRange(8.0f, 12.0f);
                    newPos = { randomX, randomY, randomZ };

                    bool tooClose = false;

                    // ���̔��e�Ƃ̋������`�F�b�N
                    for (const auto& pos : bombPositions)
                    {
                        float dx = pos.x - newPos.x;
                        float dz = pos.z - newPos.z;
                        // �߂�����ꍇ��NG
                        if ((dx * dx + dz * dz) < minDistance * minDistance)
                        {
                            tooClose = true;
                            break;
                        }
                    }

                    // �G�iowner�j�Ƃ̋������`�F�b�N
                    float dxEnemy = newPos.x - owner->GetPosition().x;
                    float dzEnemy = newPos.z - owner->GetPosition().z;
                    float distanceToEnemySq = dxEnemy * dxEnemy + dzEnemy * dzEnemy;
                    float minDistanceToEnemy = owner->radius + bombRadius;

                    // �G�Ƌ߂�����ꍇ��NG
                    if (distanceToEnemySq < minDistanceToEnemy * minDistanceToEnemy)
                    {
                        tooClose = true;
                    }

                    // �L���Ȉʒu��������΃��[�v�𔲂���
                    if (!tooClose)
                    {
                        validPosition = true;
                        break;
                    }
                }

                //�L���ȏꏊ��������Ύ��s��Ԃ�
                if (!validPosition)
                {
                    isBombProduce = false;
                    timer = 0.0f;
                    step = 0;
                    ready = false;
                    owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                    return ActionBase::State::Failed;
                }

                //���W���ꎞ�I�ɓo�^
                bombPositions.push_back(newPos);
            }
            ready = true;
        }

        timer += elapsedTime;

        //���e�����̃^�C�~���O�����ꂢ�ɂȂ�悤��3�b��
        if (timer >= 3.0f)
        {
            //���e����������Ă��Ȃ�������(��x�����ʂ�Ȃ��悤�ɂ��邽��)
            if (!isBombProduce)
            {
                for (int i = 0; i < bombPositions.size(); ++i)
                {
                    DirectX::XMFLOAT3 currentPlayerPos = player->GetPosition();

                    float posDifferenceX = currentPlayerPos.x - x;
                    float posDifferenceZ = currentPlayerPos.z - z;

                    const auto& relativePos = bombPositions[i];

                    // �v���C���[�̌��݈ʒu�ɑ��΍��W�����Z���čŏI�ʒu������
                    DirectX::XMFLOAT3 finalPos = {
                        relativePos.x + posDifferenceX,
                        relativePos.y,
                        relativePos.z + posDifferenceZ
                    };

                    // �X�e�[�W�͈͂Ɏ��߂�i��FX: -18?18, Z: -12?12�j
                    finalPos.x = std::clamp(finalPos.x, -18.0f, 18.0f);
                    finalPos.z = std::clamp(finalPos.z, -12.0f, 12.0f);

                    Transform bombTr(
                        finalPos,
                        DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f },
                        DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f }
                    );

                    //���e����
                    auto bomb = owner->GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<Bomb>("bomb", bombTr);
                    if (!bomb)
                    {
                        isBombProduce = false;
                        timer = 0.0f;
                        step = 0;
                        owner->SetIsCoolTime(true);
                        ready = false;
                        owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                        return ActionBase::State::Failed;
                    }

                    // ���e�̏㏸�p�x��ݒ�
                    bomb->SetFanIndex(i, static_cast<int>(bombPositions.size()));
                    // �~�T�C���̉����Đ�
                    owner->misileAudioComponent->Play();
                }
                isBombProduce = true;
            }
        }

        //�A�j���[�V�������I�����Ă�����
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            isBombProduce = false;
            timer = 0.0f;
            step = 0;
            owner->SetIsCoolTime(true);
            ready = false;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Complete;
        }

        //TODO:86_�������Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered() && !owner->GetIsJumping())
        {
            isBombProduce = false;
            timer = 0.0f;
            step = 0;
            owner->SetIsCoolTime(true);
            ready = false;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Failed;
        }

        break;
    }
    }

    return ActionBase::State::Run;
}

// �����s��
ActionBase::State SummonAction::Run(float elapsedTime)
{
    static bool  isJumpPre = false;      //�\�����쒆������
    static bool  isJumpEnd = false;      //���n��������
    static bool  Jumping = false;      //�W�����v��������
    static float jumpPreTimer = 0.0f;   //�\�����쒆�̃^�C�}�[
    static float jumpEndTimer = 0.0f;   //���n���̃^�C�}�[
    static float jumpTimer = 0.0f;      //�W�����v���̃^�C�}�[
    static bool  canComplete = false;    //�r�������̃t���O

    //�ǂ̃p�^�[�����I��p
    static int index = 0;

    const char* names[] = {
        "BombBill1",
        "BombBill2",
        "BombBill3",
        "BombBill4",
        "BombBill5",
    };

    switch (step)
    {
    case 0:
    {
        //�o���`�����I
        //index = rand() % ARRAYSIZE(names);

        //�o���`���ɉ����Ĕ��ł����ꏊ���w��
        XMFLOAT3 playerPos = owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor")->GetPosition();
        XMVECTOR PlayerPos = XMLoadFloat3(&playerPos);

        float farLengthSq = 0.0f;
        XMVECTOR FarTarget{};
        for (int i = 0; i < ARRAYSIZE(names); i++)
        {
            XMFLOAT3 moveTarget = GameManager::GetBuildingManager()->GetMovePos(names[i]);
            XMVECTOR MoveTarget = XMLoadFloat3(&moveTarget);
            float lengthSq = XMVectorGetX(XMVector3LengthSq(MoveTarget - PlayerPos));
            if (lengthSq > farLengthSq)
            {
                farLengthSq = lengthSq;
                FarTarget = MoveTarget;
                index = i;
            }
        }
        XMFLOAT3 target;
        XMStoreFloat3(&target, FarTarget);

        owner->SetTargetPosition(target);

        //������
        jumpPreTimer = 0.0f;
        jumpEndTimer = 0.0f;
        jumpTimer = 0.0f;
        isJumpPre = false;
        isJumpEnd = false;
        Jumping = false;
        canComplete = false;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //�A�j���[�V�����Đ�
        owner->PlayAnimation("JumpPre", false);

        //�\������Ɉڂ点��
        isJumpPre = true;

        step++;
        break;
    }
    case 1:
        jumpPreTimer += elapsedTime;
        float threshold = 0.0f; // ���e�덷�i���a�j

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ���ʂ̋������v�Z
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // �����ɉ����� isOnGround �̔���臒l�𒲐�
        float groundThreshold = 0.0f;

        bool isOnGround = pos.y <= groundThreshold;
        bool canJumpEnd = pos.y <= 8.0f;

        //�\������̃^�C�}�[�����ȏ�ŃW�����v����Ɉڂ�
        if (!Jumping && jumpPreTimer >= 3.5f*0.5f)
        {
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            owner->PlayAnimation("Jump");
            Jumping = true;
        }

        //�w��̏ꏊ�ɔ��ł���
        if (isJumpPre && Jumping)
        {
            owner->JumpToTarget(elapsedTime, 10, 2.0);
            jumpTimer += elapsedTime;
        }
        else
        {
            owner->MoveToTarget(elapsedTime, 0);
        }

        if (isJumpEnd)
        {
            jumpEndTimer += elapsedTime;
        }

        //���n�Ɉڂ�(���n�����ꂢ�Ɍ�����悤�ɂ�����ƍ��߂Ŕ��肷��)
        if (canJumpEnd && !isJumpEnd && jumpTimer >= 1.0f)
        {
            owner->PlayAnimation("JumpEnd2", false);
            isJumpEnd = true;
        }

        if (isJumpEnd && isOnGround)
        {
            if (isJumpPre)
            {
                isJumpPre = false;
                owner->landingAudioComponent->Play();
            }
            owner->SetIsJumping(false);
            owner->SetCanSummon(false);
            owner->SetSummonTime(0.0f);
            owner->SetIsCoolTime(true);
            owner->velocity = { 0,0,0 };
            owner->SetPosition({ pos.x,0.0f,pos.z });
            owner->SetEnableHit(true);
            //owner->velocity = { 0, 0, 0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
            //isJumpPre = false;
        }

        //�w��̈ʒu�ɗ����珉����
        if (owner->GetCanSummon() && horizontalDistSq <= threshold * threshold && isOnGround)		// ��������FXZ������ threshold �ȓ� & Y�����n�ʋ߂�
        {
            //isJumpPre = false;
            //owner->SetIsJumping(false);
            //owner->SetCanSummon(false);
            //owner->SetSummonTime(0.0f);
            //owner->SetIsCoolTime(true);
            //owner->velocity = { 0,0,0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
        }

        //���n���̏���
        if (isJumpEnd && !isJumpPre)
        {
            //�����������u�ԂɃr�������܂��悤�ɔ��������Ă܂�
            if (jumpEndTimer >= 1.2f && !canComplete)
            {
                owner->SetEnableHit(false);
                //����
                GameManager::GetBuildingManager()->Spawn(names[index], false);
                canComplete = true;
            }

            //������Ԃ�
            if (!owner->GetAnimationController()->IsPlayAnimation() && canComplete)
            {
                owner->SetEnableHit(false);
                step = 0;
                jumpTimer = 0.0f;
                canComplete = false;
                isJumpEnd = false;
                jumpPreTimer = 0.0f;
                jumpEndTimer = 0.0f;
                owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                return ActionBase::State::Complete;
            }
        }

        //TODO:86_�������Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            isJumpPre = false;
            owner->SetIsJumping(false);
            owner->SetCanSummon(false);
            owner->SetSummonTime(0.0f);
            owner->SetIsCoolTime(true);
            owner->velocity = { 0,0,0 };
            owner->SetEnableHit(false);
            step = 0;
            jumpTimer = 0.0f;
            canComplete = false;
            isJumpEnd = false;
            jumpPreTimer = 0.0f;
            jumpEndTimer = 0.0f;
            DirectX::XMFLOAT3 ownerPos = owner->GetPosition();
            if (ownerPos.y > 0.0f)
            {
                owner->SetPosition({ ownerPos.x,0,ownerPos.z });
            }
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Failed;
        }

        break;
    }

    return ActionBase::State::Run;
}

// �ҋ@�s��
ActionBase::State IdleAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)return ActionBase::State::Failed;
    switch (step)
    {
    case 0:
        owner->SetCoolTime(Mathf::RandomRange(owner->GetMinCoolTime(), owner->GetMaxCoolTime()));
        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Idle");

        step++;
        break;
    case 1:
        //�v���C���[�̂ق��������悤�ɂ���
        owner->SetTargetPosition(player->GetPosition());
        owner->MoveToTarget(elapsedTime, 0.0f);

        owner->SetCoolTime(owner->GetCoolTime() - elapsedTime);

        //�ҋ@���Ԃ��߂����Ƃ�
        if (owner->GetCoolTime() <= 0.0f)
        {
            owner->SetIsCoolTime(false);
            //�����A�N�V����
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Complete;
        }

        //�U���͈͊O��������
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = player->GetPosition();
        float vx = targetPosition.x - position.x;
        float vy = targetPosition.y - position.y;
        float vz = targetPosition.z - position.z;
        float dist = sqrtf(vx * vx + vy * vy + vz * vz);

        if (dist > owner->GetAttackRange())
        {
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Failed;
        }

        //TODO:86_�N�[���^�C���ҋ@���Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Failed;
        }

        break;
    }
    return ActionBase::State::Run;
}

// �N�[���^�C�����̒ǐՍs��
ActionBase::State CoolPursuitAction::Run(float elapsedTime)
{
    const float animationFps = 120.0f;
    static float frameCounter = 0.0f;
    const int totalFrame = 226;

    int currentFrame = static_cast<int>(frameCounter);
    currentFrame %= totalFrame; // ���[�v�Đ��ɔ����ăt���[������܂�Ԃ�

    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)return ActionBase::State::Failed;

    float runTimer = owner->GetRunTimer();
    switch (step)
    {
    case 0:
        //�ڕW�n�_���v���C���[�ɐݒ�
        owner->SetTargetPosition(player->GetPosition());
        owner->SetRunTimer(Mathf::RandomRange(5.0f, 8.0f));
        frameCounter = 0.0f;

        owner->GetAnimationController()->SetAnimationRate(2.0f);
        //�A�j���[�V�����Đ�
        owner->PlayAnimation("Walk");

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;
        runTimer -= elapsedTime;
        //�^�C�}�[�X�V
        owner->SetRunTimer(runTimer);
        //�ڕW���v���C���[�ʒu�ɐݒ�
        owner->SetTargetPosition(player->GetPosition());
        //�ړI�n�ֈړ�
        if ((currentFrame >= 0.0f && currentFrame <= 40.0f) || (currentFrame >= 100.0f && currentFrame <= 130.0f))
        {
            if ((currentFrame >= 38.0f && currentFrame <= 40.0f) || (currentFrame >= 128.0f && currentFrame <= 130.0f))
            {
                owner->walkAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            }
            owner->MoveToTarget(elapsedTime, 0.3f);
        }
        else if ((currentFrame >= 65.0f && currentFrame <= 85.0f) || (currentFrame >= 160.0f && currentFrame <= 210.0f))
        {
            owner->MoveToTarget(elapsedTime, 0.8f);
        }
        else
        {
            owner->MoveToTarget(elapsedTime, 0.0f);
        }


        //�v���C���[�Ƃ̋������v�Z
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPos = owner->GetTargetPosition();


        float vx = targetPos.x - position.x;
        float vy = targetPos.y - position.y;
        float vz = targetPos.z - position.z;
        float dist = sqrtf(vx * vx + vy * vy + vz * vz);
        //�߂��͈͂ɂ���Ƃ�
        if (dist < owner->GetDistMid())
        {
            owner->walkAudioComponent->Stop();
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Complete;
        }
        // �s�����Ԃ��߂�����
        if (runTimer <= 0.0f)
        {
            owner->walkAudioComponent->Stop();
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            // �ǐՎ��s��Ԃ�
            return ActionBase::State::Complete;
        }

        //TODO:86_�N�[���^�C���ǐՒ��Ƀ_���[�W���󂯂���
        if (owner->GetIsStaggered())
        {
            owner->walkAudioComponent->Stop();
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Failed;
        }

        break;
    }
    return ActionBase::State::Run;
}

