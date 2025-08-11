#include "ActionDerived.h"
#include "RiderEnemy.h"
#include "Game/Actors/Player/Player.h"
#include "EnemyMath.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Scenes/MainScene.h"
#include "Engine/Scene/Scene.h"
//TODO:Behavior アクション追加

float animationBaseSpeed = 1.0f;

// 最初の演出
ActionBase::State StartPerfAction::Run(float elapsedTime)
{
    static bool isJumpEnd = false;      //着地中か判定
    static bool Jumping = false;      //ジャンプ中か判定
    static float jumpTimer = 0.0f;      //ジャンプ中のタイマー
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

        //アニメーション再生
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

        float threshold = 1.0f; // 許容誤差（半径）

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ平面の距離を計算
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // Y軸の高さ判定
        bool isOnGround = pos.y <= 0.0f;
        bool canJumpEnd = pos.y <= 9.0f;

        owner->MoveToTarget(elapsedTime, 0);

        //着地に移る(着地がきれいに見えるようにちょっと高めで判定する)
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
            // 敵が落ち終わったので　false にする
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

// ひるみ
ActionBase::State DamageAction::Run(float elapsedTime)
{
    switch (step)
    {
    case 0:
        //アニメーション再生
        owner->PlayAnimation("Damage", false);

        step++;
        break;
    case 1:
    {
        //アニメーションが終了しているとき
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

// 状態変化
ActionBase::State ChangeAction::Run(float elapsedTime)
{
    static float timer = 3.0f;
    switch (step)
    {
    case 0:
        //アニメーション再生

        step++;
        break;
    case 1:
        timer -= elapsedTime;
        //アニメーションが終了しているとき
        //if (!owner->GetAnimationController()->IsPlayAnimation())
        if (timer <= 0.0f)
        {
            //状態変化フラグtrue
            owner->SetIsChange(true);
            timer = 3.0f;
            step = 0;
            //成功を返す
            return ActionBase::State::Complete;
        }
        break;
    }
    return ActionBase::State::Run;
}

// 必殺技
ActionBase::State SpecialAction::Run(float elapsedTime)
{
    switch (step)
    {
    case 0:
        owner->SetCanSpecial(true);
        // 必殺技の音を再生
        owner->specialAudioComponent->Play();
        if (owner->GetCanTerrain())
        {
            owner->SetCanTerrain(false);
        }

        //エフェクト用ハンドラ
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

        //アニメーション再生
        owner->PlayAnimation("Special", false);

        step++;
        break;
    case 1:
        //アニメーションが終了しているとき
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            //必殺技発動
            GameManager::GetBuildingManager()->SwitchToBossBuilding();
            owner->SetCanSpecial(false);
            owner->SetCurrentSpecialGauge(0);
            owner->SetIsCoolTime(true);
            step = 0;
            //成功を返す
            return ActionBase::State::Complete;
        }

        //TODO:86_必殺中にダメージを受けたら
        if (owner->GetIsStaggered())
        {
            owner->SetIsStaggered(false);
            //owner->SetCanSpecial(false);
            //owner->SetCurrentSpecialGauge(0);
            //owner->SetIsCoolTime(true);
            //step = 0;
            //return ActionBase::State::Failed;
        }

        //エフェクト更新

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

// 地形変動
ActionBase::State TerrainAction::Run(float elapsedTime)
{
    static bool isJumpPre = false;      //予備動作中か判定
    static bool isJumpEnd = false;      //着地中か判定
    static bool Jumping = false;      //ジャンプ中か判定
    static float jumpPreTimer = 0.0f;   //予備動作中のタイマー
    static float jumpEndTimer = 0.0f;   //着地時のタイマー
    static float jumpTimer = 0.0f;      //ジャンプ中のタイマー
    static bool canComplete = false;    //ビル生成のフラグ

    //どのパターンか選択用
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
        //出現形式抽選
        //index = rand() % ARRAYSIZE(names);

        //出現形式に応じて飛んでいく場所を指定
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

        //初期化
        jumpPreTimer = 0.0f;
        jumpEndTimer = 0.0f;
        jumpTimer = 0.0f;
        isJumpPre = false;
        isJumpEnd = false;
        Jumping = false;
        canComplete = false;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //アニメーション再生
        owner->PlayAnimation("JumpPre", false);
        //予備動作に移らせる
        isJumpPre = true;

        step++;
        break;
    }
    case 1:
        jumpPreTimer += elapsedTime;
        float threshold = 1.0f; // 許容誤差（半径）

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ平面の距離を計算
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // 距離に応じて isOnGround の判定閾値を調整
        float groundThreshold = 0.0f;

        bool isOnGround = pos.y <= groundThreshold;
        bool canJumpEnd = pos.y <= 8.0f;

        //予備動作のタイマーが一定以上でジャンプ動作に移る
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

        //指定の場所に飛んでいく
        if (isJumpPre && Jumping)
        {
            owner->JumpToTarget(elapsedTime, 10, 2.0);
            jumpTimer += elapsedTime;
        }
        
        if (isJumpEnd)
        {
            jumpEndTimer += elapsedTime;
        }

        //着地に移る(着地がきれいに見えるようにちょっと高めで判定する)
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

        //指定の位置に来たら初期化
        if (owner->GetCanTerrain() && horizontalDistSq <= threshold * threshold && isOnGround)		// 判定条件：XZ距離が threshold 以内 & Y軸が地面近く
        {
            //isJumpPre = false;
            //owner->SetIsJumping(false);
            //owner->SetCanTerrain(false);
            //owner->SetIsCoolTime(true);
            //owner->SetTerrainTime(0.0f);
            //owner->velocity = { 0,0,0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
        }

        //着地時の処理
        if (isJumpEnd && !isJumpPre)
        {
            //足が着いた瞬間にビルが生まれるように微調整してます
            if (jumpEndTimer >= 1.2f && !canComplete)
            {
                owner->SetEnableHit(false);
                //TODO::ビル生成
                GameManager::GetBuildingManager()->Spawn(names[index]);
                canComplete = true;
            }
            
            //完了を返す
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

        //TODO:86_地形変動中にダメージを受けたら
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

// 追跡行動
ActionBase::State PursuitAction::Run(float elapsedTime)
{
    const float animationFps = 120.0f;
    static float frameCounter = 0.0f;
    const int totalFrame = 226;

    int currentFrame = static_cast<int>(frameCounter);
    currentFrame %= totalFrame; // ループ再生に備えてフレーム数を折り返す

    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    switch (step)
    {
    case 0:
        //目標地点をプレイヤーに設定
        owner->SetTargetPosition(player->GetPosition());

        frameCounter = 0.0f;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //アニメーション再生
        owner->PlayAnimation("Walk");

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;
        //目標地点をプレイヤーに設定
        owner->SetTargetPosition(player->GetPosition());
        //目的地へ移動
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

        // プレイヤーとの距離を計算
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();

        float vx = targetPosition.x - position.x;
        float vy = targetPosition.y - position.y;
        float vz = targetPosition.z - position.z;
        float dist = sqrtf(vx * vx + vy * vy + vz * vz);
        // 攻撃範囲にいるとき
        if (dist < owner->GetAttackRange())
        {
            owner->walkAudioComponent->Stop();
            step = 0;
            // 追跡成功を返す
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Complete;
        }

        //TODO:86_追跡中にダメージを受けたら
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

// 通常攻撃
ActionBase::State NormalAction::Run(float elapsedTime)
{
    //170~260まで当たり判定を取る
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

        //アニメーション再生
        owner->PlayAnimation("Normal", false);

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;

        if (currentFrame >= 170 && currentFrame <= 250)
        {
            owner->SetEnableHit(true);
        }

        //アニメーションが終了しているとき
        if (!owner->GetAnimationController()->IsPlayAnimation())
        {
            owner->SetAnimationRate(1.0f);
            owner->SetEnableHit(false);
            step = 0;
            owner->SetIsCoolTime(true);
            //成功を返す
            return ActionBase::State::Complete;
        }

        //TODO:86_通常攻撃中にダメージを受けたら
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
// 突進行動
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
        //初期化
        if (!charge)
        {
            owner->SetChargeTimer(owner->GetMaxChargeTimer());
            owner->SetAnimationRate(animationBaseSpeed);
            //構えのアニメーション再生
            owner->PlayAnimation("RotatePre", false);
            charge = true;
            animation = false;
        }

        //向きをプレイヤーに合わせる
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

            // 突進時の音を再生する（無限ループ）
            owner->rushAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            //突進のアニメーション再生
            owner->PlayAnimation("Rotate");
            owner->SetAnimationRate(2.0f);
            owner->SetEnableHit(true);
            step++;
        }

        //TODO:86_突進中にダメージを受けたら
        if (owner->GetIsStaggered())
        {
            //音停止
            owner->rushAudioComponent->Stop();
            charge = false;
            animation = false;
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Failed;
        }

        break;
    case 1:
        // 目的地点までのXZ平面での距離判定
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();
        float vx = targetPosition.x - position.x;
        float vz = targetPosition.z - position.z;
        float distSq = vx * vx + vz * vz;

        // 目的地点へ移動
        if (!animation)
        {
            owner->MoveToTarget(elapsedTime, 5.0f);
        }

        float radius = owner->radius + 0.0f;
        // 目的地へ着いた
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
                // 突進時の音の再生を止める
                owner->rushAudioComponent->Stop();
                owner->PlayAnimation("RotateEnd",false);
                animation = true;
            }
        }
        
        //成功を返す
        if (animation && !owner->GetAnimationController()->IsPlayAnimation())
        {
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Complete;
        }

        //TODO:86_突進中にダメージを受けたら
        if (owner->GetIsStaggered())
        {
            //音停止
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

//突進処理・改
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
        //初期化

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
        //構えのアニメーション再生
        owner->PlayAnimation("RotatePre", false);

        step++;
        break;
    case 1:
        //ため時間減算
        chargeTimer -= elapsedTime;
        owner->SetChargeTimer(chargeTimer);

        //ため時間中は向きをプレイヤーに合わせる
        if (!charge)
        {
            owner->SetTargetPosition(player->GetPosition());
            owner->MoveToTarget(elapsedTime, 0.0f);
        }

        //ため時間が終わったら
        if (!charge && chargeTimer <= 0.0f && !owner->GetAnimationController()->IsPlayAnimation())
        {
            charge = true;
            x = owner->GetTargetPosition().x;
            z = owner->GetTargetPosition().z;

            x = std::clamp(x, -17.0f, 17.0f);
            z = std::clamp(z, -12.0f, 12.0f);

            owner->SetTargetPosition({ x, player->GetPosition().y, z });

            // 突進時の音を再生する（無限ループ）
            owner->rushAudioComponent->Play(XAUDIO2_LOOP_INFINITE);
            //突進のアニメーション再生
            owner->PlayAnimation("Rotate");
            owner->SetAnimationRate(2.0f);
            owner->SetEnableHit(true);
            loopAnimation = true;
        }

        // 目的地点までのXZ平面での距離判定
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPosition = owner->GetTargetPosition();
        float vx = targetPosition.x - position.x;
        float vz = targetPosition.z - position.z;
        float distSq = vx * vx + vz * vz;

        // 目的地点へ移動
        if (loopAnimation && charge)
        {
            owner->MoveToTarget(elapsedTime, 5.0f);
        }

        float radius = owner->radius + 0.0f;
        // 目的地へ着いた
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
                // 突進時の音の再生を止める
                owner->rushAudioComponent->Stop();
                owner->PlayAnimation("RotateEnd",false);
                loopAnimation = false;
            }
        }

        //成功を返す
        if (!loopAnimation && !owner->GetAnimationController()->IsPlayAnimation())
        {
            charge = false;
            owner->SetAnimationRate(animationBaseSpeed);
            step = 0;
            return ActionBase::State::Complete;
        }

        //TODO:86_突進中にダメージを受けたら
        if (owner->GetIsStaggered())
        {
            //音停止
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

// 爆撃行動
ActionBase::State BombingAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)
    {
        return ActionBase::State::Failed;
    }

    static float timer = 0.0f;                              //爆弾生成をきれいに見せるためのタイマー
    static std::vector<DirectX::XMFLOAT3> bombPositions;    //爆弾のポジションを保存しておくもの
    const int bombCount = 8;                                //爆弾の最大数
    const float bombRadius = 2.0f;                          //爆弾の半径
    const float minDistance = bombRadius * 2.0f;            //爆弾と爆弾の最小距離(爆弾同士が被らないように)
    static bool ready = false;                              //準備完了か判定
    static bool isBombProduce = false;                      //爆弾を生成するか判定
    static float x = 0.0f;
    static float z = 0.0f;

    switch (step)
    {
    case 0:
    {
        //初期化
        isBombProduce = false;
        timer = 0.0f;
        ready = false;
        x = 0.0f;
        z = 0.0f;

        //アニメーション再生
        owner->PlayAnimation("Bombing", false);
        step++;
        break;
    }
    case 1:
    {
        //準備完了じゃなかったら(一度しか通らないようにするため)
        if (!ready)
        {
            bombPositions.clear(); // 前回の位置をクリア

            for (int i = 0; i < bombCount; ++i)
            {
                bool validPosition = false;
                DirectX::XMFLOAT3 newPos;

                // 最大100回まで爆弾の有効な設置位置を試行
                for (int attempt = 0; attempt < 100; ++attempt)
                {
                    // プレイヤーの現在位置を取得
                    x = player->GetPosition().x;
                    z = player->GetPosition().z;

                    // プレイヤーの周囲±8の範囲にランダムな座標を生成
                    float randomX = Mathf::RandomRange(-10.0f, 10.0f) + x;
                    float randomZ = Mathf::RandomRange(-10.0f, 10.0f) + z;

                    //// 座標をステージ範囲に制限
                    //randomX = std::clamp(randomX, -18.0f, 18.0f);
                    //randomZ = std::clamp(randomZ, -12.0f, 12.0f);

                    // ステージ範囲外ならスキップ（補正しない）
                    if (randomX < -18.0f || randomX > 18.0f || randomZ < -12.0f || randomZ > 12.0f)
                        continue;

                    // 高さ(Y)はランダムに設定
                    float randomY = Mathf::RandomRange(8.0f, 12.0f);
                    newPos = { randomX, randomY, randomZ };

                    bool tooClose = false;

                    // 他の爆弾との距離をチェック
                    for (const auto& pos : bombPositions)
                    {
                        float dx = pos.x - newPos.x;
                        float dz = pos.z - newPos.z;
                        // 近すぎる場合はNG
                        if ((dx * dx + dz * dz) < minDistance * minDistance)
                        {
                            tooClose = true;
                            break;
                        }
                    }

                    // 敵（owner）との距離もチェック
                    float dxEnemy = newPos.x - owner->GetPosition().x;
                    float dzEnemy = newPos.z - owner->GetPosition().z;
                    float distanceToEnemySq = dxEnemy * dxEnemy + dzEnemy * dzEnemy;
                    float minDistanceToEnemy = owner->radius + bombRadius;

                    // 敵と近すぎる場合もNG
                    if (distanceToEnemySq < minDistanceToEnemy * minDistanceToEnemy)
                    {
                        tooClose = true;
                    }

                    // 有効な位置が見つかればループを抜ける
                    if (!tooClose)
                    {
                        validPosition = true;
                        break;
                    }
                }

                //有効な場所が無ければ失敗を返す
                if (!validPosition)
                {
                    isBombProduce = false;
                    timer = 0.0f;
                    step = 0;
                    ready = false;
                    owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
                    return ActionBase::State::Failed;
                }

                //座標を一時的に登録
                bombPositions.push_back(newPos);
            }
            ready = true;
        }

        timer += elapsedTime;

        //爆弾生成のタイミングがきれいになるように3秒後
        if (timer >= 3.0f)
        {
            //爆弾生成がされていなかったら(一度しか通らないようにするため)
            if (!isBombProduce)
            {
                for (int i = 0; i < bombPositions.size(); ++i)
                {
                    DirectX::XMFLOAT3 currentPlayerPos = player->GetPosition();

                    float posDifferenceX = currentPlayerPos.x - x;
                    float posDifferenceZ = currentPlayerPos.z - z;

                    const auto& relativePos = bombPositions[i];

                    // プレイヤーの現在位置に相対座標を加算して最終位置を決定
                    DirectX::XMFLOAT3 finalPos = {
                        relativePos.x + posDifferenceX,
                        relativePos.y,
                        relativePos.z + posDifferenceZ
                    };

                    // ステージ範囲に収める（例：X: -18?18, Z: -12?12）
                    finalPos.x = std::clamp(finalPos.x, -18.0f, 18.0f);
                    finalPos.z = std::clamp(finalPos.z, -12.0f, 12.0f);

                    Transform bombTr(
                        finalPos,
                        DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f },
                        DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f }
                    );

                    //爆弾生成
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

                    // 爆弾の上昇角度を設定
                    bomb->SetFanIndex(i, static_cast<int>(bombPositions.size()));
                    // ミサイルの音を再生
                    owner->misileAudioComponent->Play();
                }
                isBombProduce = true;
            }
        }

        //アニメーションが終了していたら
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

        //TODO:86_爆撃中にダメージを受けたら
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

// 召喚行動
ActionBase::State SummonAction::Run(float elapsedTime)
{
    static bool  isJumpPre = false;      //予備動作中か判定
    static bool  isJumpEnd = false;      //着地中か判定
    static bool  Jumping = false;      //ジャンプ中か判定
    static float jumpPreTimer = 0.0f;   //予備動作中のタイマー
    static float jumpEndTimer = 0.0f;   //着地時のタイマー
    static float jumpTimer = 0.0f;      //ジャンプ中のタイマー
    static bool  canComplete = false;    //ビル生成のフラグ

    //どのパターンか選択用
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
        //出現形式抽選
        //index = rand() % ARRAYSIZE(names);

        //出現形式に応じて飛んでいく場所を指定
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

        //初期化
        jumpPreTimer = 0.0f;
        jumpEndTimer = 0.0f;
        jumpTimer = 0.0f;
        isJumpPre = false;
        isJumpEnd = false;
        Jumping = false;
        canComplete = false;

        owner->GetAnimationController()->SetAnimationRate(2.0f);

        //アニメーション再生
        owner->PlayAnimation("JumpPre", false);

        //予備動作に移らせる
        isJumpPre = true;

        step++;
        break;
    }
    case 1:
        jumpPreTimer += elapsedTime;
        float threshold = 0.0f; // 許容誤差（半径）

        DirectX::XMFLOAT3 pos = owner->GetPosition();
        DirectX::XMFLOAT3 target = owner->GetTargetPosition();

        // XZ平面の距離を計算
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        float horizontalDistSq = dx * dx + dz * dz;

        // 距離に応じて isOnGround の判定閾値を調整
        float groundThreshold = 0.0f;

        bool isOnGround = pos.y <= groundThreshold;
        bool canJumpEnd = pos.y <= 8.0f;

        //予備動作のタイマーが一定以上でジャンプ動作に移る
        if (!Jumping && jumpPreTimer >= 3.5f*0.5f)
        {
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            owner->PlayAnimation("Jump");
            Jumping = true;
        }

        //指定の場所に飛んでいく
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

        //着地に移る(着地がきれいに見えるようにちょっと高めで判定する)
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

        //指定の位置に来たら初期化
        if (owner->GetCanSummon() && horizontalDistSq <= threshold * threshold && isOnGround)		// 判定条件：XZ距離が threshold 以内 & Y軸が地面近く
        {
            //isJumpPre = false;
            //owner->SetIsJumping(false);
            //owner->SetCanSummon(false);
            //owner->SetSummonTime(0.0f);
            //owner->SetIsCoolTime(true);
            //owner->velocity = { 0,0,0 };
            //owner->SetPosition({ pos.x,0.0f,pos.z });
        }

        //着地時の処理
        if (isJumpEnd && !isJumpPre)
        {
            //足が着いた瞬間にビルが生まれるように微調整してます
            if (jumpEndTimer >= 1.2f && !canComplete)
            {
                owner->SetEnableHit(false);
                //生成
                GameManager::GetBuildingManager()->Spawn(names[index], false);
                canComplete = true;
            }

            //完了を返す
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

        //TODO:86_召喚中にダメージを受けたら
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

// 待機行動
ActionBase::State IdleAction::Run(float elapsedTime)
{
    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)return ActionBase::State::Failed;
    switch (step)
    {
    case 0:
        owner->SetCoolTime(Mathf::RandomRange(owner->GetMinCoolTime(), owner->GetMaxCoolTime()));
        //アニメーション再生
        owner->PlayAnimation("Idle");

        step++;
        break;
    case 1:
        //プレイヤーのほうを向くようにする
        owner->SetTargetPosition(player->GetPosition());
        owner->MoveToTarget(elapsedTime, 0.0f);

        owner->SetCoolTime(owner->GetCoolTime() - elapsedTime);

        //待機時間が過ぎたとき
        if (owner->GetCoolTime() <= 0.0f)
        {
            owner->SetIsCoolTime(false);
            //何かアクション
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);
            return ActionBase::State::Complete;
        }

        //攻撃範囲外だったら
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

        //TODO:86_クールタイム待機中にダメージを受けたら
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

// クールタイム中の追跡行動
ActionBase::State CoolPursuitAction::Run(float elapsedTime)
{
    const float animationFps = 120.0f;
    static float frameCounter = 0.0f;
    const int totalFrame = 226;

    int currentFrame = static_cast<int>(frameCounter);
    currentFrame %= totalFrame; // ループ再生に備えてフレーム数を折り返す

    auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    if (!player)return ActionBase::State::Failed;

    float runTimer = owner->GetRunTimer();
    switch (step)
    {
    case 0:
        //目標地点をプレイヤーに設定
        owner->SetTargetPosition(player->GetPosition());
        owner->SetRunTimer(Mathf::RandomRange(5.0f, 8.0f));
        frameCounter = 0.0f;

        owner->GetAnimationController()->SetAnimationRate(2.0f);
        //アニメーション再生
        owner->PlayAnimation("Walk");

        step++;
        break;
    case 1:
        frameCounter += elapsedTime * animationFps;
        runTimer -= elapsedTime;
        //タイマー更新
        owner->SetRunTimer(runTimer);
        //目標をプレイヤー位置に設定
        owner->SetTargetPosition(player->GetPosition());
        //目的地へ移動
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


        //プレイヤーとの距離を計算
        DirectX::XMFLOAT3 position = owner->GetPosition();
        DirectX::XMFLOAT3 targetPos = owner->GetTargetPosition();


        float vx = targetPos.x - position.x;
        float vy = targetPos.y - position.y;
        float vz = targetPos.z - position.z;
        float dist = sqrtf(vx * vx + vy * vy + vz * vz);
        //近い範囲にいるとき
        if (dist < owner->GetDistMid())
        {
            owner->walkAudioComponent->Stop();
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            return ActionBase::State::Complete;
        }
        // 行動時間が過ぎた時
        if (runTimer <= 0.0f)
        {
            owner->walkAudioComponent->Stop();
            owner->SetIsCoolTime(false);
            step = 0;
            owner->GetAnimationController()->SetAnimationRate(animationBaseSpeed);

            // 追跡失敗を返す
            return ActionBase::State::Complete;
        }

        //TODO:86_クールタイム追跡中にダメージを受けたら
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

