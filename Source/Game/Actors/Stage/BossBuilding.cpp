#include "BossBuilding.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Managers/GameManager.h"
#include "Game/Managers/TutorialSystem.h"

void BossBuilding::Update(float deltaTime)
{
    {// �r���̂���オ��
        riseTimer += deltaTime;
        riseTimer = std::min<float>(riseTimer, riseTime);
        float t = std::clamp(riseTimer / riseTime, 0.0f, 1.0f);
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.y = std::lerp(riseStart.y, riseEnd.y, t);

        float time = riseTimer;
        pos.x = riseStart.x + std::sinf(2 * DirectX::XM_PI * shakeFrequency * time) * shakeAmplitude;
        pos.z = riseStart.z + std::cosf(2 * DirectX::XM_PI * shakeFrequency * time) * shakeAmplitude;

        SetPosition(pos);
        static bool onceCollisionLayer = false;
        if (t >= 1.0f && !onceCollisionLayer)
        {// ���肠���肫������
            bombTimerMeshComponent->SetIsVisible(true);
            bombTimerMeshComponentUnder->SetIsVisible(true);
            //boxComponent->EnableCollision();
            //boxComponent->AddCollisionFilter(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
            //onceCollisionLayer = true;
        }
    }
    switch (state)
    {
    case BossBuilding::BuildingState::Idle:
        break;
    case BossBuilding::BuildingState::Exploding:
        // ���������Đ�����
        explosionSoundComponent->Play();

        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");

        preSkeltalMeshComponent->SetIsVisible(false);
        preSkeltalMeshComponent->SetIsCastShadow(false);
        //auto& model = preSkeltalMeshComponent->model;
        //model->SetAlpha(0.0f);

        //this->DestroyComponentByName("boxComponent");
        // ���I�𓖂��蔻��ɓ����
        if (convexComponent)
        {
            convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
            convexComponent->SetKinematic(false);
            convexComponent->SetActive(true);
        }

        TutorialSystem::AchievedAction(TutorialStep::BossBuild);
        //TutorialSystem::AchievedAction(TutorialStep::MoveCamera);

        // ���b��Ɋ��I����������ݒ肷��
        ScheduleDeactivate(convexTimer);

        eraseInAreaComponent->EnableCollision();
        {
            auto lifeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeComponent");
            //auto lifeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeComponent");
            lifeComponent->SetLifeTime(1.0f);
        }
        state = BossBuilding::BuildingState::Exploded;
        break;
    case BossBuilding::BuildingState::Exploded:
        break;
    case BossBuilding::BuildingState::Destroing:
        // ���I������鉹���Đ�����
        debriSoundComponent->Play();
        TutorialSystem::AchievedAction(TutorialStep::BossBuild);
        //TutorialSystem::AchievedAction(TutorialStep::MoveCamera);

        if (isLastHitBoss)
        {// �{�X�ɂ���ĉ󂳂ꂽ��
            // ���X�̔��̓����蔻�������
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);

            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // ���I�𓖂��蔻��ɓ����
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }
            // ���U���g�p::�r���j��
            GameManager::CallBuildBroken();

            // ���b��Ɋ��I����������ݒ肷��
            ScheduleDeactivate(convexTimer);

            // �r������A�C�e�����h���b�v����
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // �{�X�ɔj�󂳂ꂽ�Ƃ��� 5 �̌Œ�l�Ƃ���
            itemSpawner->SpawnItems(5, true);

            //isDestroyed = false;
            state = BossBuilding::BuildingState::Destroyed;

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
         //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        else if (isLastHitBomb)
        {// ���e�ɂ���ĉ󂳂ꂽ��
            // ���������Đ�����
            //explosionSoundComponent->Play();
            // ���X�̔��̓����蔻�������
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);
            //this->DestroyComponentByName("boxComponent");
            // ���I�𓖂��蔻��ɓ����
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // ���b��Ɋ��I����������ݒ肷��
            ScheduleDeactivate(convexTimer);
            state = BossBuilding::BuildingState::Destroyed;
            //isDestroyed = false;
        }
        else if (isLastHitBeam)
        {// �r�[���ɂ���Ĕj�󂵂���
            // �Ռ��g��ǉ�����
            DirectX::XMFLOAT3 pos = GetPosition();
            //Transform transform(pos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(0.01f, 1.0f, 0.01f));
            //auto shockWave = ActorManager::CreateAndRegisterActorWithTransform<ShockWave>("shockWave", transform);
            //shockWave->SetWaveDetails(0.1f, 3.0f, 2.0f, beamPower_);

            if (restBeamPower >= 0)
            {
                auto shockWave = this->NewSceneComponent<ShockWaveCollisionComponent>("shockWave", "preSkeltalMeshComponent");
                shockWave->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_, restBeamPower);
                shockWaveMeshComponent->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_);
            }
            // ���X�̔��̓����蔻�������
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // ���I�𓖂��蔻��ɓ����
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // ���b��Ɋ��I����������ݒ肷��
            ScheduleDeactivate(convexTimer);

            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 ���邩��
            int count = static_cast<int>(std::ceil(beamCount * itemPop));
            itemSpawner->SpawnItems(count, true);

            state = BossBuilding::BuildingState::Destroyed;
            // ���U���g�p::�r���j��
            GameManager::CallBuildBroken();
        }
        else
        {// 
            // ���X�̔��̓����蔻�������
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // ���I�𓖂��蔻��ɓ����
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // ���b��Ɋ��I����������ݒ肷��
            ScheduleDeactivate(convexTimer);

            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 ���邩��
            int count = static_cast<int>(std::ceil(beamCount * itemPop));
            itemSpawner->SpawnItems(count, true);

            state = BossBuilding::BuildingState::Destroyed;
        }
        preSkeltalMeshComponent->SetIsCastShadow(false);
        break;
    case BossBuilding::BuildingState::Destroyed:
        break;
    default:
        break;
    }
    //preSkeltalMeshComponent->model->meshes[0].primitives[0].material = 1;
    //auto& model = preSkeltalMeshComponent->model;
    //model->SetAlpha(1.0f);
    if (shouldDeactivate)
    {
        deactivateTime -= deltaTime;
        if (deactivateTime <= 0.0f)
        {// ���̃I�u�W�F�N�g������
            //convexComponent->DisableCollision();
            //this->DestroyComponentByName("convexComponent");
            //this->SetValid(false);
            this->SetPendingDestroy();
            //this->Destroy();
        }
    }
}


// �Ռ��g�ɓ����������ɌĂяo���֐�
void BossBuilding::CallHitShockWave(float power, int beamItemCount, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
    if (state != BuildingState::Idle)
    {
        return;
    }
    {
        state = BuildingState::Destroing;
        preSkeltalMeshComponent->SetIsVisible(false);
        preSkeltalMeshComponent->SetIsCastShadow(false);
        //boxComponent->DisableCollision();
        eraseInAreaComponent->DisableCollision();
        // ���X�̔��̓����蔻�������
        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");
        this->ScheduleDestroyComponentByName("eraseInAreaComponent");
        //this->DestroyComponentByName("boxComponent");
        // �r�[��������
        //beam->SetValid(false);
        // ���I�𓖂��蔻��ɓ����
        convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
        convexComponent->SetKinematic(false);
        convexComponent->SetActive(true);

        // ���b��Ɋ��I����������ݒ肷��
        ScheduleDeactivate(4.0f);

        //DirectX::XMFLOAT3 pos = GetPosition();
        //pos.y += 0.5f;

        //// �r������A�C�e�����h���b�v����
        //Transform transform(pos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        //auto buildDroppedItem = ActorManager::CreateAndRegisterActorWithTransform<PickUpItem>("buildDroppedItem", transform);
        //auto lifeTimeComponent = buildDroppedItem->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        //lifeTimeComponent->SetLifeTime(5.0f);

        ////float itemPower = beam->GetItemCount();
        //buildDroppedItem->skeltalMeshComponent->SetIsVisible(true); // ������悤�ɂ���

        // �r������A�C�e�����h���b�v����
        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
        itemSpawner->SpawnItems(beamItemCount, true);


        auto buildlifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        buildlifeTimeComponent->SetLifeTime(5.0f);

    }
}


// �L�l�}�e�B�b�N���m�̓����蔻������m����
void BossBuilding::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    if (state != BuildingState::Idle)
    {
        return;
    }
    if (hitPair.second->GetActor()->GetName() == "stage")
    {
        return;
    }
    if (hp <= 0)
    {// hp �� 0 �ɂȂ����炻�������ʂ�Ȃ�
        return;
    }
    if (hitPair.first != boxComponent.get())
    {
        return;
    }
    //std::string a = hitPair.second->GetActor()->GetName() + " is Hit bossBUilding\n";
    //OutputDebugStringA(a.c_str());
    auto bomb = std::dynamic_pointer_cast<Bomb>(hitPair.second->GetActor());
    if (bomb)
    {// �������e��������
        //if (hitPair.second->name() != "sphereComponent")
        //{
        //    return;
        //}
        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectExplosionComponent->SetWorldLocationDirect(GetPosition());
        effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
        //effectExplosionComponent->SetEffectImpulse(impulse);
        //effectExplosionComponent->SetEffectNormal(normal);
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();
        hp = 0;
        isDestroyed = true;
        isLastHitBomb = true;
        state = BuildingState::Destroing;
    }
    auto beam = std::dynamic_pointer_cast<Beam>(hitPair.second->GetActor());
    if (beam)
    {// �r�[���Ȃ�
        //if (lastHitBeam_ == beam.get())
        //{
        //    return;
        //}
        if (beam->HasAlreadyHit(this))
        {
            return;
        }

        beamCount = beam->GetItemCount();
        int beamPower = static_cast<int>(beam->GetItemPower());
        if (beamPower <= 0)
        {// �r�[���� power ���Ȃ�������
            return;// ��� assertion �ɕύX
        }
        beamPower_ = beam->GetItemPower();
        lastHitBeam_ = beam.get();
        {
            restBeamPower = beamCount - hp;
            // ���ڂň�C�ɔj�󂳂ꂽ�Ƃ��� hp�@������Ȃ��o�O����邽��
            hp -= beamCount;
            // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
            effectExplosionComponent->SetWorldLocationDirect(GetPosition());
            effectExplosionComponent->SetEffectPower(beam->GetItemPower());
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();

            char debugBuffer[128];
            sprintf_s(debugBuffer, sizeof(debugBuffer),
                "beam power%f, beamCount%d\n",
                beamPower_, beamCount);
            OutputDebugStringA(debugBuffer);

            OutputDebugStringA("beam is Hit building \n");
            state = BuildingState::Destroing;
            isLastHitBeam = true;
            isDestroyed = true;
        }
        beam->RegisterHit(this);
    }
    auto otherActor = hitPair.second->GetOwner();
    auto boss = dynamic_cast<RiderEnemy*>(otherActor);
    if (boss)
    {// �����{�X��������
        if (hitPair.second->name() != "capsuleComponent")
        {// �{�X�̑̂���Ȃ�������
            return;
        }
        //if (auto node = boss->activeNode)
        {
            //if (node->GetName() == "Dash")
            {// �{�X���_�b�V���U���Ȃ�
                // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
                DirectX::XMFLOAT3 hitPos = GetPosition();   // �r���̍��W
                hitPos.y += 0.3f;
                effectExplosionComponent->SetWorldLocationDirect(hitPos);
                //effectExplosionComponent->SetWorldLocationDirect(hitPos);
                effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
                //effectExplosionComponent->SetEffectImpulse(impulse);
                //effectExplosionComponent->SetEffectNormal(normal);
                effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
                effectExplosionComponent->Activate();

                hp = 0;
                state = BuildingState::Destroing;
                isDestroyed = true;
                isLastHitBoss = true;
            }
        }
    }

}
