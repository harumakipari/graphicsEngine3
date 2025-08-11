#include "RiderEnemy.h"


#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Effect/Particles.h"
#include "../../../Widgets/ObjectManager.h"
#include "../../../Widgets/GameObject.h"
#include "../../../Widgets/Mask.h"

//  root(p) ー Change（末端）
//          ー Battle(p)
//             ー Special （末端）
//             ー Pursuit （末端）
//             ー Attack(r)
//                ー Normal （末端）
//                ー Charge （末端）
//                ー Bombing（末端）
//                ー Summon （末端）
//          ー Scout(p)
//             ー Wander（末端）
//             ー Idle  （末端）

#if 1
RiderEnemy::RiderEnemy(const std::string& modelName) :Enemy(modelName)
{
    //// ここで当たり判定を追加
    //Sphere enemyCollider{};
    //enemyCollider.c = DirectX::XMFLOAT3(0.0f, 0.6f, 0.0f);
    //enemyCollider.r = 0.5f;
    //SphereShape enemyColliderShape;
    //colliderComponent.SetShape(enemyColliderShape);

    behaviorData = new BehaviorData();
    aiTree = new BehaviorTree(this);

    aiTree->AddNode("", "Root", 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    aiTree->AddNode("Root", "StartPerf", 0, BehaviorTree::SelectRule::Non, new StartPerfJudgment(this), new StartPerfAction(this));
    aiTree->AddNode("Root", "Damage", 1, BehaviorTree::SelectRule::Non, new DamageJudgment(this), new DamageAction(this));
    //aiTree->AddNode("Root", "Change", 1, BehaviorTree::SelectRule::Non, new ChangeJudgment(this), new ChangeAction(this));
    aiTree->AddNode("Root", "Battle", 2, BehaviorTree::SelectRule::Priority, new BattleJudgment(this), nullptr);
    aiTree->AddNode("Root", "Scout", 3, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    aiTree->AddNode("Battle", "Special", 0, BehaviorTree::SelectRule::Non, new SpecialJudgment(this), new SpecialAction(this));
    aiTree->AddNode("Battle", "Terrain", 1, BehaviorTree::SelectRule::Non, new TerrainJudgment(this), new TerrainAction(this));
    aiTree->AddNode("Battle", "Pursuit", 2, BehaviorTree::SelectRule::Non, new PursuitJudgment(this), new PursuitAction(this));
    aiTree->AddNode("Battle", "Attack", 3, BehaviorTree::SelectRule::Random, new AttackJudgment(this), nullptr);

    aiTree->AddNode("Attack", "Normal", 0, BehaviorTree::SelectRule::Non, new NormalJudgment(this), new NormalAction(this));
    aiTree->AddNode("Attack", "Dash", 1, BehaviorTree::SelectRule::Non, new ChargeJudgment(this), new DashAction(this));
    aiTree->AddNode("Attack", "Bombing", 2, BehaviorTree::SelectRule::Non, nullptr, new BombingAction(this));
    aiTree->AddNode("Attack", "Summon", 3, BehaviorTree::SelectRule::Non, new SummonJudgment(this), new SummonAction(this));

    //aiTree->AddNode("Charge", "Dash", 0, BehaviorTree::SelectRule::Non, nullptr, new DashAction(this));
    //aiTree->AddNode("Charge", "Normal", 1, BehaviorTree::SelectRule::Non, nullptr, new NormalAction(this));

    aiTree->AddNode("Scout", "CoolPursuit", 0, BehaviorTree::SelectRule::Non, new CoolPursuitJudgment(this), new CoolPursuitAction(this));
    aiTree->AddNode("Scout", "Idle", 1, BehaviorTree::SelectRule::Non, nullptr, new IdleAction(this));

    hp = maxHp;
    radius = 1.0f;
    height = 1.0f;
    mass = 150.0f;
    //position.x = 3.0f;
    //TransitionWanderState();
    //TransitionIdleState();
    specialGauge = 0;
    isChange = false;
    canSpecial = false;
    isChange = false;
    distNear = 3.0f;
    distMid = 8.0f;
    distFar = 15.0f;
    runTimer = 0.0f;
    chargeTimer = 0.0f;
    isCoolTime = true;
    coolTime = 0.0f;
    terrainTime = 0.0f;
    canTerrain = false;
    summonTime = 0.0f;
    canSummon = false;
    moveVecX = 0.0f;
    moveVecZ = 0.0f;
    isJumping = false;

    jumpT = 0.0f;

    isStartPerf = true;
    enableHit = false;
}

void RiderEnemy::Finalize()
{
    walkAudioComponent->Stop();
    delete aiTree;
    delete behaviorData;
}

void RiderEnemy::Update(float elapsedTime)
{
    // ボスの目玉のジョイント
    DirectX::XMFLOAT3 bossEye = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
    bossJointComponent->SetRelativeLocationDirect(bossEye);

    hasHitThisFrame = false;

    // 現在実行されているノードが無ければ
    if (activeNode == nullptr)
    {
        // 次に実行するノードを推論する。
        activeNode = aiTree->ActiveNodeInference(behaviorData);
    }
    // 現在実行するノードがあれば
    if (activeNode != nullptr)
    {
        // ビヘイビアツリーからノードを実行。
        activeNode = aiTree->Run(activeNode, behaviorData, elapsedTime);
    }

    if (!canTerrain && GameManager::GetGameTimerStart())
    {
        terrainTime += elapsedTime;
    }

    if (!canSummon && GameManager::GetGameTimerStart())
    {
        summonTime += elapsedTime;
    }

    if (/*isChange && */!canSpecial && GameManager::GetGameTimerStart())
    {
        specialGauge += elapsedTime;
    }

    DirectX::XMFLOAT3 position = GetPosition();

    if (GetAsyncKeyState('R') & 0x8000)
    {
    }

    Character::Update(elapsedTime);
    //velocity.y += gravity * elapsedTime;

    // 垂直速力更新処理
    //UpdateVerticalVelocity(elapsedTime);

    // 水平速力更新処理
    UpdateHorizontalVelocity(elapsedTime);

    // 垂直移動更新処理
    //if (!isJumping)
    //{
        //UpdateVerticalMove(elapsedTime);
    //}

    // 水平移動処理更新
    UpdateHorizontalMove(elapsedTime);

    DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointLocalPosition("R_thumb1_FK", skeltalMeshComponent->model->GetNodes());
    bossHandComponent->SetRelativeLocationDirect(bossHand);
    //DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointWorldPosition("PLT:ThumbFinger2_L_BK", skeltalMeshComponent->model->nodes, rootComponent_->GetComponentWorldTransform().ToWorldTransform());
    //bossHandComponent->SetWorldLocationDirect(bossHand);

    isAttacked = false;

    ////状況に応じた更新処理
    //switch (state)
    //{
    //case State::Idle:
    //    UpdateIdleState(elapsedTime);
    //    break;
    //case State::Wander:
    //    //UpdateWanderState(elapsedTime, playerPosition);
    //    break;
    //case State::Pursuit:
    //    //UpdatePursuitState(elapsedTime, playerPosition);
    //    break;
    //case State::Attack:
    //    UpdateAttackState(elapsedTime);
    //    break;
    //}
    AnimationTransition();

    //GetWorldTransform();

    //UI
    float normalizedHp = static_cast<float>(GetHP() / static_cast<float>(GetMaxHP()));
    ObjectManager::Find("BossHP")->GetComponent<Mask>()->valueX = normalizedHp;
    float normalizedEnergy = specialGauge / static_cast<float>(maxSpecialGauge);
    ObjectManager::Find("BossEnergy")->GetComponent<Mask>()->valueX = normalizedEnergy;


    // 出現場所にギアのモデルを出現させる
    //if (isJumping)
    //{
    DirectX::XMFLOAT3 tarPos = lastTarget;
    tarPos.y += 1.0f;

    gearInMeshComponent->SetIsVisible(isJumping);
    gearInMeshComponent->SetWorldLocationDirect(tarPos);
    //gearInMeshComponent->SetWorldRotationDirect({ 0,0,0,0 });
    gearOutMeshComponent->SetIsVisible(isJumping);
    gearOutMeshComponent->SetWorldLocationDirect(tarPos);
    //gearOutMeshComponent->SetWorldRotationDirect({ 0,0,0,0 });

//}
//else
//{
//    gearInMeshComponent->SetIsVisible(false);
//    gearOutMeshComponent->SetIsVisible(false);
//}

#ifdef USE_IMGUI
    //switch (state)
    //{
    //case State::Idle:
    //    ImGui::Text("Idle State");
    //    break;
    //case State::Wander:
    //    ImGui::Text("Wander State");
    //    break;
    //case State::Pursuit:
    //    ImGui::Text("Pursuit State");
    //    break;
    //case State::Attack:
    //    ImGui::Text("Attack State");
    //    break;

    //}
    ImGui::Checkbox("particle", &isIntegrateParticles);

    //ImGui::DragFloat3("rotation", &rotation.x, 0.5f);
    ImGui::DragFloat3("targetPosition", &targetPosition.x, 0.5f);
    ImGui::Checkbox("Player Detected", &isDetected);
    ImGui::Checkbox("Enemy Attacked", &isAttacked);
    //ImGui::Checkbox("animtionIsFinish", &GetModelComponent().isAnimationFinished);
    //ImGui::Checkbox("animtionIsLoop", &GetModelComponent().isAnimationLoop);
    //ImGui::Checkbox("animtionIsBlending", &GetModelComponent().isBlendingAnimation);
    //ImGui::InputInt("Current Animation Clip:", (int*)&GetModelComponent().animationClip);
    std::string str = "";
    if (activeNode != nullptr)
    {
        str = activeNode->GetName();
    }
    auto player = std::dynamic_pointer_cast<Player>(GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
    DirectX::XMFLOAT3 pos = player->GetPosition();
    ImGui::Begin("Rider Enemy");
    ImGui::Text("Behavior  %s", str.c_str());
    ImGui::Text("HP", &hp);
    ImGui::InputFloat("jumpT", &jumpT);
    ImGui::Checkbox("EnableHit", &enableHit);
    ImGui::DragFloat3("Position", &position.x, 0.5f);
    ImGui::InputFloat("coolTime", &coolTime);
    ImGui::InputFloat("terrainTime", &terrainTime);
    ImGui::InputFloat("ChargeTimer", &chargeTimer);
    ImGui::InputFloat("summonTime", &summonTime);
    ImGui::Checkbox("canTerrain", &canTerrain);
    ImGui::Checkbox("canSummon", &canSummon);
    ImGui::Checkbox("isGround", &isGround);
    ImGui::Checkbox("isJumping", &isJumping);
    ImGui::InputFloat3("targetPos", &targetPosition.x);
    ImGui::DragFloat3("velocity", &velocity.x);

    ImGui::InputFloat("attackRange", &attackRange);
    ImGui::InputFloat("distNear", &distNear/*, 0.5f*/);
    ImGui::InputFloat("distMid", &distMid);
    ImGui::InputFloat("distFar", &distFar/*, 0.5f*/);
    ImGui::InputFloat3("playerPos", &pos.x);
    ImGui::End();

#endif

}

// キネマティック同士の当たり判定を検知する
void RiderEnemy::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    if (auto stage = std::dynamic_pointer_cast<Stage>(hitPair.second->GetActor()))
    {
        //if (hitPair.second->name() == "boxComponent")
        //{
        //    isGround = true;
        //}
        //else
        //{
        //    isGround = false;
        //}
    }
    if (auto beam = std::dynamic_pointer_cast<Beam>(hitPair.second->GetActor()))
    {// ビームに当たったら、
        if (beam->HasAlreadyHit(this))
        {
            return;
        }

        if (hitPair.first->name() != "capsuleComponent")
        {
            return;
        }
        if (beam.get() == lastHitBeam_)
        {// 二度ヒットを防ぐため
            return;
        }
        if (hasHitThisFrame)
        {
            return;
        }

        // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (int)
        int beamPower = beam->GetItemCount();

        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "beam is hit boss! beam power:%d  Before boss Hp:%d BeamName:%s\n", beamPower, hp, beam->GetName().c_str());
        OutputDebugStringA(debugBuffer);

        hp -= beamPower;

        if (beamPower > 20)
        {
            if (!isStaggered)
            {
                isStaggered = true;
            }
        }
        hasHitThisFrame = true;

        GameManager::CallBossDamaged();
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "beam is hit boss! beam power:%d  After boss Hp:%d\n", beamPower, hp);
        OutputDebugStringA(debugBuffer);
        lastHitBeam_ = beam.get();
        beam->RegisterHit(this);
    }
}

void RiderEnemy::UpdateParticle(ID3D11DeviceContext* immediate_context, float deltaTime, ParticleSystem* p)
{

#if 0
    if (state == State::Attack)
    {//攻撃の時
        if (!isIntegrateParticles)
        {
            p->Initialize(immediate_context, deltaTime);
            isIntegrateParticles = true;
            p->particleSystemData.emissionPosition.x = GetJointPosition(64/*tongue_03*/).x;
            p->particleSystemData.emissionPosition.y = GetJointPosition(64/*tongue_03*/).y;
            p->particleSystemData.emissionPosition.z = GetJointPosition(64/*tongue_03*/).z;
        }
        p->Integrate(immediate_context, deltaTime);
        //前方向のベクトルを取得
        DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&GetForward());
        Front = DirectX::XMVector3Normalize(Front);

        DirectX::XMVECTOR ParticlePos = DirectX::XMLoadFloat4(&p->particleSystemData.emissionPosition);
        ParticlePos = DirectX::XMVectorAdd(ParticlePos, DirectX::XMVectorScale(Front, 0.1f));

        DirectX::XMStoreFloat4(&p->particleSystemData.emissionPosition, ParticlePos);
    }
    else
    {
        isIntegrateParticles = false;
    }
#else
    //p->particleSystemData.direction = GetForward();
    //p->particleSystemData.emissionPosition.x = GetJointPosition(64/*tongue_03*/).x;
    //p->particleSystemData.emissionPosition.y = GetJointPosition(64/*tongue_03*/).y /*+ 0.02f*/;
    //p->particleSystemData.emissionPosition.z = GetJointPosition(64/*tongue_03*/).z;
    if (testFlag)
    {
        p->Initialize(immediate_context, deltaTime);
    }
    std::string str = "";
    if (activeNode != nullptr)
    {
        str = activeNode->GetName();
    }
    if (str == "Attack")
    {//攻撃の時
        p->Integrate(immediate_context, deltaTime);
        p->particleSystemData.state = 0; //started
    }
    else
    {//攻撃以外の時パーティクル
        p->particleSystemData.state = 1; //finished
    }
    testFlag = false;
#if 0
    if (state == State::Attack)
    {//攻撃の時
        isIntegrateParticles = true;
        p->Initialize(immediate_context, deltaTime);
        p->particleSystemData.emissionPosition.x = GetJointPosition(64/*tongue_03*/).x;
        p->particleSystemData.emissionPosition.y = GetJointPosition(64/*tongue_03*/).y;
        p->particleSystemData.emissionPosition.z = GetJointPosition(64/*tongue_03*/).z;
    }
    if (isIntegrateParticles)
    {
        p->Integrate(immediate_context, deltaTime);
    }
    //前方向のベクトルを取得
    DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&GetForward());
    Front = DirectX::XMVector3Normalize(Front);

    DirectX::XMVECTOR ParticlePos = DirectX::XMLoadFloat4(&p->particleSystemData.emissionPosition);
    ParticlePos = DirectX::XMVectorAdd(ParticlePos, DirectX::XMVectorScale(Front, 0.1f));

    DirectX::XMStoreFloat4(&p->particleSystemData.emissionPosition, ParticlePos);
#endif
#endif
}
//プレイヤー索敵
bool RiderEnemy::SearchPlayer(DirectX::XMFLOAT3 playerPosition)
{
#if 0
    DirectX::XMFLOAT3 position = GetPosition();

    using namespace DirectX;
    //playerPosition - enemyPosition
    XMVECTOR EP = XMLoadFloat3(&playerPosition) - XMLoadFloat3(&position);
    float dist = XMVectorGetX(XMVector3Length(EP));
    if (dist < searchRange)
    {
        //正規化
        EP = XMVector3Normalize(EP);

        //XMMATRIX World = XMLoadFloat4x4(&GetWorldMatrix());

        //XMVECTOR Forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), World);
        //XMVECTOR Forward = World.r[2];// Z 軸を取り出す
        //方向ベクトル と EP で内積
        //float dot = XMVectorGetX(XMVector3Dot(EP, Forward));

        //if (dot > 0.0f)
        //if (dot > cosf(XMConvertToRadians(60.0f)))
        {
            return true;
        }

    }
#endif
    return false;
}

//目標地点設定
void RiderEnemy::SetRandomTargetPosition()
{
#if 0
    DirectX::XMFLOAT3 position = GetPosition();

    float theta = MathHelper::RandomRange(-DirectX::XM_PI, DirectX::XM_PI);
    float range = MathHelper::RandomRange(3.0f, territoryRange);
    targetPosition.x = position.x + (range * sinf(theta));
    targetPosition.y = position.y;
    targetPosition.z = position.z + (range * cosf(theta));
#endif
}

//目標地点へ移動
void RiderEnemy::MoveToTarget(float elapsedTime, float speedRate)
{
    DirectX::XMFLOAT3 position = GetPosition();

    // ターゲット方向への進行ベクトルを算出
    float vx = targetPosition.x - position.x;
    float vz = targetPosition.z - position.z;
    float dist = sqrtf(vx * vx + vz * vz);
    if (dist > 0.001f)
    {
        vx /= dist;
        vz /= dist;
    }
    //移動処理
    Move(elapsedTime, vx, vz, moveSpeed * speedRate);
    Turn(elapsedTime, vx, vz, turnSpeed * speedRate);

#if 0
    //using namespace DirectX;
    ////ターゲットへの進行方向ベクトル
    //float vx = targetPosition.x - position.x;
    //float vz = targetPosition.z - position.z;
    //XMVECTOR TargetVex = XMVectorSet(vx, 0.0f, vz, 0.0f);
    ////正規化する
    //TargetVex = XMVector3Normalize(TargetVex);
    ////Moveの処理
    //float moveSpeed = 1.0f * elapsedTime * speedRate;
    //position.x += vx * moveSpeed;
    //position.z += vz * moveSpeed;

    //SetPosition(position);

    //DirectX::XMFLOAT3 rotate = GetEulerRotation();
    ////Turnの処理
    ////Enemy の前方向
    //front = GetForward();
    ////Enemy の　3軸を求める
    //XMVECTOR EX, EY, EZ;
    //EY = XMVectorSet(0, 1, 0, 0);
    //EZ = XMLoadFloat3(&front);
    ////enemy の右方向のベクトル
    //EX = XMVector3Cross(EY, EZ);
    ////進行方向とenemy の　今向いている方向の 右ベクトルで内積する
    //float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(TargetVex, EX));
    //const float inputDeadZone = 0.0001f;
    //if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    //{
    //    turningSpeed = maxTurningSpeed * turningSpeedFactor;
    //}
    //rotate.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
    //SetEulerRotation(rotate);
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //クォータニオンから回転行列を作成
    XMVECTOR Y = R.r[1];//モデルのローカルなY軸を取り出す
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, /*+DirectX::XMConvertToRadians(turningSpeed)*/turningSpeed * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
    //rotation.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;n
#endif
}

//目的地へジャンプ
void RiderEnemy::JumpToTarget(float elapsedTime, float maxHeight, float speedRate)
{
    const float duration = 3.0f;

    DirectX::XMFLOAT3 position = GetPosition();

    if (!isJumping /*|| targetPosition.x != lastTarget.x || targetPosition.z != lastTarget.z*/)
    {

        jumpStart = GetPosition();
        jumpEnd = targetPosition;
        lastTarget = targetPosition;

        jumpPeak.x = (jumpStart.x + jumpEnd.x) / 2.0f;
        jumpPeak.z = (jumpStart.z + jumpEnd.z) / 2.0f;
        jumpPeak.y = maxHeight * 2;

        jumpT = 0.0f;
        isJumping = true;
    }

    jumpT += elapsedTime / duration;
    if (jumpT >= 1.0f)
    {
        SetPosition(jumpEnd);
        isJumping = false;
        jumpT = 0.0f;
        return;
    }

    // 二次ベジェ補間
    float u = 1.0f - jumpT;
    DirectX::XMFLOAT3 pos;
    pos.x = u * u * jumpStart.x + 2 * u * jumpT * jumpPeak.x + jumpT * jumpT * jumpEnd.x;
    pos.y = u * u * jumpStart.y + 2 * u * jumpT * jumpPeak.y + jumpT * jumpT * jumpEnd.y;
    pos.z = u * u * jumpStart.z + 2 * u * jumpT * jumpPeak.z + jumpT * jumpT * jumpEnd.z;

    //プレイヤーのほうを向くようにする
    // ターゲット方向への進行ベクトルを算出
    DirectX::XMFLOAT3 targetDir;
    if (jumpT < 0.5f) {
        // 前半は目的地を向く
        targetDir.x = targetPosition.x - position.x;
        targetDir.z = targetPosition.z - position.z;
    }
    else
    {
        // 後半はプレイヤーを向く
        auto player = std::dynamic_pointer_cast<Player>(GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
        if (!player) return;
        DirectX::XMFLOAT3 targetPlayer = player->GetPosition();
        targetDir.x = targetPlayer.x - position.x;
        targetDir.z = targetPlayer.z - position.z;
    }

    float dist = sqrtf(targetDir.x * targetDir.x + targetDir.z * targetDir.z);
    if (dist > 0.001f) {
        targetDir.x /= dist;
        targetDir.z /= dist;
        Turn(elapsedTime, targetDir.x, targetDir.z, turnSpeed * speedRate);
    }

    //Turn(elapsedTime, vx, vz, turnSpeed * speedRate);

    SetPosition(pos);
}

void RiderEnemy::Move(float elapsedTime, float vx, float vz, float speed)
{
    //移動方向ベクトルを設定
    moveVecX = vx;
    moveVecZ = vz;

    //最大速度設定
    maxMoveSpeed = speed;

    //DirectX::XMFLOAT3 pos = GetPosition();
    ////進行方向のベクトル取得
    //DirectX::XMFLOAT3 moveVec = { 0,0,0 };
    //velocity.x = vx;
    //velocity.z = vz;
    //float moveSpeed = 5.0f * elapsedTime;
    //pos.x += moveVec.x * moveSpeed;
    //pos.z += moveVec.z * moveSpeed;
    //SetPosition(pos);
}

void RiderEnemy::Turn(float elapsedTime, float vx, float vz, float speed)
{
    //DirectX::XMFLOAT3 angle = GetEulerRotation();
    //angle = DirectX::XMFLOAT3(DirectX::XMConvertToDegrees(angle.x), DirectX::XMConvertToDegrees(angle.y), DirectX::XMConvertToDegrees(angle.z));

    speed *= elapsedTime;

    // 進行ベクトルがゼロベクトルの場合は処理する必要なし
    float length = sqrtf(vx * vx + vz * vz);
    if (length <= 0.001f)
    {
        return;
    }
    // 進行ベクトルを単位ベクトル化
    vx /= length;
    vz /= length;

    // 自身の回転値から前方向を求める
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    // 回転角を求めるため、2つの単位ベクトルの内積を計算する
    float dot = frontX * vx + frontZ * vz;

    // 内積値は-1.0～1.0で表現されており、2つの単位ベクトルの角度が
    // 小さいほど1.0に近づくという性質を利用して回転速度を調整する
    float rot = 1.0f - dot;

    // 左右判定を行うために２つの単位ベクトルの外積を計算する
    float cross = (frontX * vz) - (frontZ * vx);

    // 2Dの外積値が生の場合か負の場合かによって左右判定が行える
    //// 左右判定を行うことによって左右回転を選択する
    //if (cross < 0.0f)
    //{
    //    angle.y += rot;
    //}
    //else
    //{
    //    angle.y -= rot;
    //}

    float targetAngleY = angle.y;

    if (cross < 0.0f)
    {
        targetAngleY += rot;
    }
    else
    {
        targetAngleY -= rot;
    }

    // 線形補間で現在の角度から目標角度へスムーズに変化させる
    float lerpFactor = 0.1f; // 補間係数（0.0?1.0、小さいほどゆっくり）
    angle.y = angle.y + (targetAngleY - angle.y) * lerpFactor;

    DirectX::XMFLOAT4 quaternion;
    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMStoreFloat4(&quaternion, q);

    SetQuaternionRotation(quaternion);
    //SetEulerRotation(angle);
//SetEulerRotation(DirectX::XMFLOAT3(0.0f,30.0f,0.0f));
//angle = DirectX::XMFLOAT3(DirectX::XMConvertToDegrees(angle.x), DirectX::XMConvertToDegrees(angle.y), DirectX::XMConvertToDegrees(angle.z));

}

void RiderEnemy::UpdateHorizontalVelocity(float elapsedTime)
{
    float elapsedFrame = elapsedTime * 60.0f;

    // XZ平面の速力を減速する
    float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
    if (length > 0.0f)
    {
        // 摩擦力
        float friction = this->friction * elapsedFrame;

        // 摩擦による横方向の減速処理
        if (length > friction)
        {
            // 単位ベクトル化
            float vx = velocity.x / length;
            float vz = velocity.z / length;

            velocity.x -= vx * friction;
            velocity.z -= vz * friction;
        }
        // 横方向の速度が摩擦力以下になったので速力を無効化
        else
        {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
    }
    // XZ平面の速力を加速する
    if (length <= maxMoveSpeed)
    {
        // 移動ベクトルがゼロベクトルでないなら加速する
        float moveVecLength = sqrtf(moveVecX * moveVecX + moveVecZ * moveVecZ);
        if (moveVecLength > 0.0f)
        {
            // 加速力
            float acceleration = this->acceleration * elapsedFrame;

            // 移動ベクトルによる加速処理
            velocity.x += moveVecX * acceleration;
            velocity.z += moveVecZ * acceleration;

            // 最大速度制限
            float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
            if (length > maxMoveSpeed)
            {
                float vx = velocity.x / length;
                float vz = velocity.z / length;

                velocity.x = vx * maxMoveSpeed;
                velocity.z = vz * maxMoveSpeed;
            }
        }
    }
    // 移動ベクトルをリセット
    moveVecX = 0.0f;
    moveVecZ = 0.0f;
}

void RiderEnemy::UpdateHorizontalMove(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetPosition();
    //移動処理
    position.x += velocity.x * elapsedTime;
    position.z += velocity.z * elapsedTime;
    SetPosition(position);

    //moveComponent->SetVelocity(velocity);  // こっちでも動きます！！
}

void RiderEnemy::UpdateVerticalVelocity(float elapsedTime)
{
    //重力処理
    velocity.y += gravity * elapsedTime;
}

void RiderEnemy::UpdateVerticalMove(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetPosition();

    //移動処理
    position.y += velocity.y * elapsedTime;

    //地面判定
    if (position.y < 0.0f)
    {
        position.y = 0.0f;

        isGround = true;
        velocity.y = 0.0f;
    }
    else
    {
        isGround = false;
    }

    SetPosition(position);
}

void RiderEnemy::AnimationTransition()
{
    //switch (state)
    //{
    //case State::Idle:
    //    //GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Idle), true);
    //    break;
    //case State::Wander:
    //    //GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Fwd_Start), true);
    //    break;
    //case State::Pursuit:
    //    //GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Fwd_Start), true);
    //    break;
    //case State::Attack:
    //    //GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Attack), false);
    //    break;
    //}
}

#endif