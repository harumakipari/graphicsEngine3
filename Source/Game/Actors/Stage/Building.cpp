#include "Building.h"

#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Managers/GameManager.h"


void Building::Update(float deltaTime)
{
    switch (hp)
    {
    case 0:
        break;
    case 1:
        preSkeltalMeshComponent->model->meshes[0].primitives[0].material = 2;
        break;
    case 2:
        preSkeltalMeshComponent->model->meshes[0].primitives[0].material = 1;
        break;
    case 3:
        preSkeltalMeshComponent->model->meshes[0].primitives[0].material = 0;
        break;
    case 4:
        break;
    }

    //preSkeltalMeshComponent->model->meshes[0].primitives[0].material = materialNum;
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

    {// �r���̂���オ��
        float noiseSeedX = MathHelper::RandomRange(0.0f, 1000.0f);
        float noiseSeedZ = MathHelper::RandomRange(1000.0f, 2000.0f);
        float noiseSpeed = 1.0f;
        riseTimer += deltaTime;
        riseTimer = std::min<float>(riseTimer, riseTime);
        float t = std::clamp(riseTimer / riseTime, 0.0f, 1.0f);
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.y = std::lerp(riseStart.y, riseEnd.y, t);

        //float time = riseTimer;
        float time = riseTimer;
        float nx = ValueNoise1D(noiseSeedX + time);  // [-1,1] �܂��� [0,1] �� [-1,1] �ɕϊ�
        float nz = ValueNoise1D(noiseSeedZ + time);

        //shakeFrequency = MathHelper::RandomRange(8.0f, 13.0f);
        //shakeAmplitude = MathHelper::RandomRange(0.05f,0.1f);
        //pos.x = riseStart.x + nx * shakeAmplitude;
        //pos.z = riseStart.z + nz * shakeAmplitude;
        pos.x = riseStart.x + std::sinf(2 * DirectX::XM_PI * shakeFrequency * time) * shakeAmplitude;
        pos.z = riseStart.z + std::cosf(2 * DirectX::XM_PI * shakeFrequency * time) * shakeAmplitude;

        SetPosition(pos);
        static bool onceCollisionLayer = false;
        if (t >= 1.0f && !onceCollisionLayer)
        {// ���肠���肫������
            //boxComponent->EnableCollision();
            //boxComponent->AddCollisionFilter(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
            //onceCollisionLayer = true;
        }
    }
    if (isDestroyed)
    {// �r������ꂽ��
        // ���I������鉹���Đ�����
        debriSoundComponent->Play();
        if (isLastHitBoss)
        {// �{�X�ɂ���ĉ󂳂ꂽ��
            // ���X�̔��̓����蔻�������
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            // ���I�𓖂��蔻��ɓ����
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // ���b��Ɋ��I����������ݒ肷��
            ScheduleDeactivate(convexTimer);

            // �r������A�C�e�����h���b�v����
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // �{�X�ɔj�󂳂ꂽ�Ƃ��� 5 �̌Œ�l�Ƃ���
            itemSpawner->SpawnItems(itemCountWithBossDestroy, true, itemLifeTimer);

            isDestroyed = false;
            // ���U���g�p::�r���j��
            GameManager::CallBuildBroken();

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

            //// �r������A�C�e�����h���b�v����
            //auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
            //// �{�X�ɔj�󂳂ꂽ�Ƃ��� 5 �̌Œ�l�Ƃ���
            //itemSpawner->SpawnItems(5, true);
            //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);
            isDestroyed = false;

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
         //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        else if (isLastHitBossBuilding)
        {// �{�X�r���ɂ���ĉ󂳂ꂽ��
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

            //// �r������A�C�e�����h���b�v����
            //auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
            //// �{�X�ɔj�󂳂ꂽ�Ƃ��� 5 �̌Œ�l�Ƃ���
            //itemSpawner->SpawnItems(5, true);
            //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);
            isDestroyed = false;
        }
        else
        {// �r�[���ɂ���Ĕj�󂵂���
            // �Ռ��g��ǉ�����
            auto shockWave = this->NewSceneComponent<ShockWaveCollisionComponent>("shockWave", "preSkeltalMeshComponent");
            shockWave->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_, restBeamPower);
            shockWaveMeshComponent->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_);
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

            // �r������A�C�e�����h���b�v����
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 ���邩��
            int count = static_cast<int>(beamCount * itemPop);
            itemSpawner->SpawnItems(count, true);

            isDestroyed = false;
            // ���U���g�p::�r���j��
            GameManager::CallBuildBroken();

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
            //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        preSkeltalMeshComponent->SetIsCastShadow(false);
    }
}


// �Ռ��g�ɓ����������ɌĂяo���֐�
void Building::CallHitShockWave(float power, int beamItemCount, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
    if (hp > beamItemCount)
    {
        hp -= static_cast<int>(beamItemCount);
    }
    else
    {
        // ��ꂽ����HP��0�ɂ��Ă���
        hp = 0;
        if (TutorialSystem::GetCurrentStep() == TutorialStep::ThirdAttack)
        {
            //TutorialSystem::SetCurrentStep(TutorialStep::BossBuild);
            TutorialSystem::SetCurrentStep(TutorialStep::MoveCamera);

        }

        // �Ռ��g�̃~�b�V������B��
        //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);

        // ���I������鉹���Đ�����
        debriSoundComponent->Play();
        preSkeltalMeshComponent->SetIsVisible(false);
        preSkeltalMeshComponent->SetIsCastShadow(false);
        // ���U���g�p::�r���j��
        GameManager::CallBuildBroken();

        // ���X�̔��̓����蔻�������
        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");

        //this->ScheduleDestroyComponentByName("boxComponent");
        //this->DestroyComponentByName("boxComponent");
        //// �r�[��������
        //beam->SetValid(false);
        // ���I�𓖂��蔻��ɓ����
        if (convexComponent)
        {
            convexComponent->AddToScene(); // ������ physx �� scene �ɒǉ�����@�����܂ł͕������Z�̍l���ɓ��ꂽ���Ȃ�����
            convexComponent->SetKinematic(false);
            convexComponent->SetActive(true);
        }

        // ���b��Ɋ��I����������ݒ肷��
        ScheduleDeactivate(4.0f);

        // �r������A�C�e�����h���b�v����
        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
        itemSpawner->SpawnItems(beamItemCount, true);

        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectExplosionComponent->SetWorldLocationDirect(GetPosition());
        effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();

        auto buildlifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        buildlifeTimeComponent->SetLifeTime(5.0f);
    }
}


//�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
void Building::NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
#if 0
    if (hp <= 0)
    {// hp �� 0 �ɂȂ����炻�������ʂ�Ȃ�
        return;
    }
    //auto stageProp = dynamic_cast<StageProp*>(otherActor);
    //if (stageProp)
    //{
    //    hp--;
    //    return;
    //}
    auto bomb = dynamic_cast<Bomb*>(otherActor);
    if (bomb)
    {// �������e��������
        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectExplosionComponent->SetWorldLocationDirect(hitPos);
        effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
        effectExplosionComponent->SetEffectImpulse(impulse);
        effectExplosionComponent->SetEffectNormal(normal);
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();

        hp = 0;

        isDestroyed = true;
        isLastHitBomb = true;
        return;
    }

    auto beam = dynamic_cast<Beam*>(otherActor);
    if (!beam)
    {// �r�[������Ȃ�������
        return;
    }
    beamCount = beam->GetItemCount();
    int beamPower = static_cast<int>(beam->GetItemPower());
    if (beamPower <= 0)
    {// �r�[���� power ���Ȃ�������
        return;// ��� assertion �ɕύX
    }
    beamPower_ = beam->GetItemPower();

    if (hp > beamPower)
    {
        hp -= beamCount;
        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "beam power%f, beamCount%d\n",
            beamPower_, beamCount);
        OutputDebugStringA(debugBuffer);

        OutputDebugStringA("beam is Hit building \n");
    }
    else /*if (isDestroyed)*/
    {
        // ���ڂň�C�ɔj�󂳂ꂽ�Ƃ��� hp�@������Ȃ��o�O����邽��
        hp -= beamCount;

        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectExplosionComponent->SetWorldLocationDirect(hitPos);
        effectExplosionComponent->SetEffectPower(beam->GetItemPower());
        effectExplosionComponent->SetEffectImpulse(impulse);
        effectExplosionComponent->SetEffectNormal(normal);
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();


        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectShockWaveComponent->SetWorldLocationDirect(hitPos);
        effectShockWaveComponent->SetEffectPower(shockWavePower);     //TODO:01 ���� power �͌�� buildingHP - itemPower �ɂȂ�\��
        effectShockWaveComponent->SetEffectImpulse(impulse);
        effectShockWaveComponent->SetEffectNormal(normal);
        effectShockWaveComponent->SetEffectType(EffectComponent::EffectType::ShockWave);
        effectShockWaveComponent->Activate();

        isDestroyed = true;
    }

#endif // 0
}

// �L�l�}�e�B�b�N���m�̓����蔻������m����
void Building::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    if (hp <= 0)
    {// hp �� 0 �ɂȂ����炻�������ʂ�Ȃ�
        return;
    }
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
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();

        hp = 0;

        isDestroyed = true;
        isLastHitBomb = true;
    }

    auto bossBuilding = std::dynamic_pointer_cast<BossBuilding>(hitPair.second->GetActor());
    if (bossBuilding)
    {
        if (hitPair.second->name() == "eraseInAreaComponent")
        {
            effectExplosionComponent->SetWorldLocationDirect(GetPosition());
            effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();
            hp = 0;
            isDestroyed = true;
            isLastHitBossBuilding = true;
        }
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
        TutorialSystem::AchievedAction(TutorialStep::CreateBuild);
        if (hp > beamPower)
        {
            hp -= beamCount;
            char debugBuffer[128];
            sprintf_s(debugBuffer, sizeof(debugBuffer),
                "beam power%f, beamCount%d\n",
                beamPower_, beamCount);
            OutputDebugStringA(debugBuffer);

            OutputDebugStringA("beam is Hit building \n");
        }
        else /*if (isDestroyed)*/
        {
            restBeamPower = beamCount - hp;
            // ���ڂň�C�ɔj�󂳂ꂽ�Ƃ��� hp�@������Ȃ��o�O����邽��
            hp -= beamCount;

            char debugBuffer[128];
            sprintf_s(debugBuffer, sizeof(debugBuffer),
                "beam power%f, beamCount%d\n",
                beamPower_, beamCount);
            OutputDebugStringA(debugBuffer);

            OutputDebugStringA("beam is Hit building \n");

            // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
            effectExplosionComponent->SetWorldLocationDirect(GetPosition());
            effectExplosionComponent->SetEffectPower(5.0f); // �����萔�l�ɂ���
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();

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

                hp = 0;

                isDestroyed = true;
                isLastHitBoss = true;
            }
        }
    }
}


