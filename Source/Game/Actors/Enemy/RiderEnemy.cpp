#include "RiderEnemy.h"


#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Effect/Particles.h"
#include "../../../Widgets/ObjectManager.h"
#include "../../../Widgets/GameObject.h"
#include "../../../Widgets/Mask.h"

//  root(p) �[ Change�i���[�j
//          �[ Battle(p)
//             �[ Special �i���[�j
//             �[ Pursuit �i���[�j
//             �[ Attack(r)
//                �[ Normal �i���[�j
//                �[ Charge �i���[�j
//                �[ Bombing�i���[�j
//                �[ Summon �i���[�j
//          �[ Scout(p)
//             �[ Wander�i���[�j
//             �[ Idle  �i���[�j

#if 1
RiderEnemy::RiderEnemy(const std::string& modelName) :Enemy(modelName)
{
    //// �����œ����蔻���ǉ�
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
    // �{�X�̖ڋʂ̃W���C���g
    DirectX::XMFLOAT3 bossEye = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
    bossJointComponent->SetRelativeLocationDirect(bossEye);

    hasHitThisFrame = false;

    // ���ݎ��s����Ă���m�[�h���������
    if (activeNode == nullptr)
    {
        // ���Ɏ��s����m�[�h�𐄘_����B
        activeNode = aiTree->ActiveNodeInference(behaviorData);
    }
    // ���ݎ��s����m�[�h�������
    if (activeNode != nullptr)
    {
        // �r�w�C�r�A�c���[����m�[�h�����s�B
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

    // �������͍X�V����
    //UpdateVerticalVelocity(elapsedTime);

    // �������͍X�V����
    UpdateHorizontalVelocity(elapsedTime);

    // �����ړ��X�V����
    //if (!isJumping)
    //{
        //UpdateVerticalMove(elapsedTime);
    //}

    // �����ړ������X�V
    UpdateHorizontalMove(elapsedTime);

    DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointLocalPosition("R_thumb1_FK", skeltalMeshComponent->model->GetNodes());
    bossHandComponent->SetRelativeLocationDirect(bossHand);
    //DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointWorldPosition("PLT:ThumbFinger2_L_BK", skeltalMeshComponent->model->nodes, rootComponent_->GetComponentWorldTransform().ToWorldTransform());
    //bossHandComponent->SetWorldLocationDirect(bossHand);

    isAttacked = false;

    ////�󋵂ɉ������X�V����
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


    // �o���ꏊ�ɃM�A�̃��f�����o��������
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

// �L�l�}�e�B�b�N���m�̓����蔻������m����
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
    {// �r�[���ɓ���������A
        if (beam->HasAlreadyHit(this))
        {
            return;
        }

        if (hitPair.first->name() != "capsuleComponent")
        {
            return;
        }
        if (beam.get() == lastHitBeam_)
        {// ��x�q�b�g��h������
            return;
        }
        if (hasHitThisFrame)
        {
            return;
        }

        // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (int)
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
    {//�U���̎�
        if (!isIntegrateParticles)
        {
            p->Initialize(immediate_context, deltaTime);
            isIntegrateParticles = true;
            p->particleSystemData.emissionPosition.x = GetJointPosition(64/*tongue_03*/).x;
            p->particleSystemData.emissionPosition.y = GetJointPosition(64/*tongue_03*/).y;
            p->particleSystemData.emissionPosition.z = GetJointPosition(64/*tongue_03*/).z;
        }
        p->Integrate(immediate_context, deltaTime);
        //�O�����̃x�N�g�����擾
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
    {//�U���̎�
        p->Integrate(immediate_context, deltaTime);
        p->particleSystemData.state = 0; //started
    }
    else
    {//�U���ȊO�̎��p�[�e�B�N��
        p->particleSystemData.state = 1; //finished
    }
    testFlag = false;
#if 0
    if (state == State::Attack)
    {//�U���̎�
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
    //�O�����̃x�N�g�����擾
    DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&GetForward());
    Front = DirectX::XMVector3Normalize(Front);

    DirectX::XMVECTOR ParticlePos = DirectX::XMLoadFloat4(&p->particleSystemData.emissionPosition);
    ParticlePos = DirectX::XMVectorAdd(ParticlePos, DirectX::XMVectorScale(Front, 0.1f));

    DirectX::XMStoreFloat4(&p->particleSystemData.emissionPosition, ParticlePos);
#endif
#endif
}
//�v���C���[���G
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
        //���K��
        EP = XMVector3Normalize(EP);

        //XMMATRIX World = XMLoadFloat4x4(&GetWorldMatrix());

        //XMVECTOR Forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), World);
        //XMVECTOR Forward = World.r[2];// Z �������o��
        //�����x�N�g�� �� EP �œ���
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

//�ڕW�n�_�ݒ�
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

//�ڕW�n�_�ֈړ�
void RiderEnemy::MoveToTarget(float elapsedTime, float speedRate)
{
    DirectX::XMFLOAT3 position = GetPosition();

    // �^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
    float vx = targetPosition.x - position.x;
    float vz = targetPosition.z - position.z;
    float dist = sqrtf(vx * vx + vz * vz);
    if (dist > 0.001f)
    {
        vx /= dist;
        vz /= dist;
    }
    //�ړ�����
    Move(elapsedTime, vx, vz, moveSpeed * speedRate);
    Turn(elapsedTime, vx, vz, turnSpeed * speedRate);

#if 0
    //using namespace DirectX;
    ////�^�[�Q�b�g�ւ̐i�s�����x�N�g��
    //float vx = targetPosition.x - position.x;
    //float vz = targetPosition.z - position.z;
    //XMVECTOR TargetVex = XMVectorSet(vx, 0.0f, vz, 0.0f);
    ////���K������
    //TargetVex = XMVector3Normalize(TargetVex);
    ////Move�̏���
    //float moveSpeed = 1.0f * elapsedTime * speedRate;
    //position.x += vx * moveSpeed;
    //position.z += vz * moveSpeed;

    //SetPosition(position);

    //DirectX::XMFLOAT3 rotate = GetEulerRotation();
    ////Turn�̏���
    ////Enemy �̑O����
    //front = GetForward();
    ////Enemy �́@3�������߂�
    //XMVECTOR EX, EY, EZ;
    //EY = XMVectorSet(0, 1, 0, 0);
    //EZ = XMLoadFloat3(&front);
    ////enemy �̉E�����̃x�N�g��
    //EX = XMVector3Cross(EY, EZ);
    ////�i�s������enemy �́@�������Ă�������� �E�x�N�g���œ��ς���
    //float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(TargetVex, EX));
    //const float inputDeadZone = 0.0001f;
    //if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    //{
    //    turningSpeed = maxTurningSpeed * turningSpeedFactor;
    //}
    //rotate.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
    //SetEulerRotation(rotate);
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //�N�H�[�^�j�I�������]�s����쐬
    XMVECTOR Y = R.r[1];//���f���̃��[�J����Y�������o��
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, /*+DirectX::XMConvertToRadians(turningSpeed)*/turningSpeed * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
    //rotation.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;n
#endif
}

//�ړI�n�փW�����v
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

    // �񎟃x�W�F���
    float u = 1.0f - jumpT;
    DirectX::XMFLOAT3 pos;
    pos.x = u * u * jumpStart.x + 2 * u * jumpT * jumpPeak.x + jumpT * jumpT * jumpEnd.x;
    pos.y = u * u * jumpStart.y + 2 * u * jumpT * jumpPeak.y + jumpT * jumpT * jumpEnd.y;
    pos.z = u * u * jumpStart.z + 2 * u * jumpT * jumpPeak.z + jumpT * jumpT * jumpEnd.z;

    //�v���C���[�̂ق��������悤�ɂ���
    // �^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
    DirectX::XMFLOAT3 targetDir;
    if (jumpT < 0.5f) {
        // �O���͖ړI�n������
        targetDir.x = targetPosition.x - position.x;
        targetDir.z = targetPosition.z - position.z;
    }
    else
    {
        // �㔼�̓v���C���[������
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
    //�ړ������x�N�g����ݒ�
    moveVecX = vx;
    moveVecZ = vz;

    //�ő呬�x�ݒ�
    maxMoveSpeed = speed;

    //DirectX::XMFLOAT3 pos = GetPosition();
    ////�i�s�����̃x�N�g���擾
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

    // �i�s�x�N�g�����[���x�N�g���̏ꍇ�͏�������K�v�Ȃ�
    float length = sqrtf(vx * vx + vz * vz);
    if (length <= 0.001f)
    {
        return;
    }
    // �i�s�x�N�g����P�ʃx�N�g����
    vx /= length;
    vz /= length;

    // ���g�̉�]�l����O���������߂�
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    // ��]�p�����߂邽�߁A2�̒P�ʃx�N�g���̓��ς��v�Z����
    float dot = frontX * vx + frontZ * vz;

    // ���ϒl��-1.0�`1.0�ŕ\������Ă���A2�̒P�ʃx�N�g���̊p�x��
    // �������ق�1.0�ɋ߂Â��Ƃ��������𗘗p���ĉ�]���x�𒲐�����
    float rot = 1.0f - dot;

    // ���E������s�����߂ɂQ�̒P�ʃx�N�g���̊O�ς��v�Z����
    float cross = (frontX * vz) - (frontZ * vx);

    // 2D�̊O�ϒl�����̏ꍇ�����̏ꍇ���ɂ���č��E���肪�s����
    //// ���E������s�����Ƃɂ���č��E��]��I������
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

    // ���`��ԂŌ��݂̊p�x����ڕW�p�x�փX���[�Y�ɕω�������
    float lerpFactor = 0.1f; // ��ԌW���i0.0?1.0�A�������قǂ������j
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

    // XZ���ʂ̑��͂���������
    float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
    if (length > 0.0f)
    {
        // ���C��
        float friction = this->friction * elapsedFrame;

        // ���C�ɂ�鉡�����̌�������
        if (length > friction)
        {
            // �P�ʃx�N�g����
            float vx = velocity.x / length;
            float vz = velocity.z / length;

            velocity.x -= vx * friction;
            velocity.z -= vz * friction;
        }
        // �������̑��x�����C�͈ȉ��ɂȂ����̂ő��͂𖳌���
        else
        {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
    }
    // XZ���ʂ̑��͂���������
    if (length <= maxMoveSpeed)
    {
        // �ړ��x�N�g�����[���x�N�g���łȂ��Ȃ��������
        float moveVecLength = sqrtf(moveVecX * moveVecX + moveVecZ * moveVecZ);
        if (moveVecLength > 0.0f)
        {
            // ������
            float acceleration = this->acceleration * elapsedFrame;

            // �ړ��x�N�g���ɂ���������
            velocity.x += moveVecX * acceleration;
            velocity.z += moveVecZ * acceleration;

            // �ő呬�x����
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
    // �ړ��x�N�g�������Z�b�g
    moveVecX = 0.0f;
    moveVecZ = 0.0f;
}

void RiderEnemy::UpdateHorizontalMove(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetPosition();
    //�ړ�����
    position.x += velocity.x * elapsedTime;
    position.z += velocity.z * elapsedTime;
    SetPosition(position);

    //moveComponent->SetVelocity(velocity);  // �������ł������܂��I�I
}

void RiderEnemy::UpdateVerticalVelocity(float elapsedTime)
{
    //�d�͏���
    velocity.y += gravity * elapsedTime;
}

void RiderEnemy::UpdateVerticalMove(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetPosition();

    //�ړ�����
    position.y += velocity.y * elapsedTime;

    //�n�ʔ���
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