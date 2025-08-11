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
        {// このオブジェクトを消す
            //convexComponent->DisableCollision();
            //this->DestroyComponentByName("convexComponent");
            //this->SetValid(false);
            this->SetPendingDestroy();
            //this->Destroy();
        }
    }

    {// ビルのせり上がり
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
        float nx = ValueNoise1D(noiseSeedX + time);  // [-1,1] または [0,1] を [-1,1] に変換
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
        {// せりあがりきったら
            //boxComponent->EnableCollision();
            //boxComponent->AddCollisionFilter(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
            //onceCollisionLayer = true;
        }
    }
    if (isDestroyed)
    {// ビルが壊れたら
        // 瓦礫が崩れる音を再生する
        debriSoundComponent->Play();
        if (isLastHitBoss)
        {// ボスによって壊されたら
            // 元々の箱の当たり判定を消す
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            // 瓦礫を当たり判定に入れる
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // 何秒後に瓦礫を消すかを設定する
            ScheduleDeactivate(convexTimer);

            // ビルからアイテムをドロップする
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // ボスに破壊されたときは 5 の固定値とする
            itemSpawner->SpawnItems(itemCountWithBossDestroy, true, itemLifeTimer);

            isDestroyed = false;
            // リザルト用::ビル破壊数
            GameManager::CallBuildBroken();

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
         //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        else if (isLastHitBomb)
        {// 爆弾によって壊されたら
            // 爆発音を再生する
            //explosionSoundComponent->Play();

            // 元々の箱の当たり判定を消す
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // 瓦礫を当たり判定に入れる
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // 何秒後に瓦礫を消すかを設定する
            ScheduleDeactivate(convexTimer);

            //// ビルからアイテムをドロップする
            //auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
            //// ボスに破壊されたときは 5 の固定値とする
            //itemSpawner->SpawnItems(5, true);
            //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);
            isDestroyed = false;

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
         //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        else if (isLastHitBossBuilding)
        {// ボスビルによって壊されたら
                        // 元々の箱の当たり判定を消す
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // 瓦礫を当たり判定に入れる
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // 何秒後に瓦礫を消すかを設定する
            ScheduleDeactivate(convexTimer);

            //// ビルからアイテムをドロップする
            //auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
            //// ボスに破壊されたときは 5 の固定値とする
            //itemSpawner->SpawnItems(5, true);
            //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);
            isDestroyed = false;
        }
        else
        {// ビームによって破壊したら
            // 衝撃波を追加する
            auto shockWave = this->NewSceneComponent<ShockWaveCollisionComponent>("shockWave", "preSkeltalMeshComponent");
            shockWave->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_, restBeamPower);
            shockWaveMeshComponent->Initialize(0.1f, shockWaveRange, shockWaveTime, beamPower_);
            // 元々の箱の当たり判定を消す
            boxComponent->DisableCollision();
            this->ScheduleDestroyComponentByName("boxComponent");

            preSkeltalMeshComponent->SetIsVisible(false);
            preSkeltalMeshComponent->SetIsCastShadow(false);
            //auto& model = preSkeltalMeshComponent->model;
            //model->SetAlpha(0.0f);

            //this->DestroyComponentByName("boxComponent");
            // 瓦礫を当たり判定に入れる
            if (convexComponent)
            {
                convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
                convexComponent->SetKinematic(false);
                convexComponent->SetActive(true);
            }

            // 何秒後に瓦礫を消すかを設定する
            ScheduleDeactivate(convexTimer);

            // ビルからアイテムをドロップする
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 するかも
            int count = static_cast<int>(beamCount * itemPop);
            itemSpawner->SpawnItems(count, true);

            isDestroyed = false;
            // リザルト用::ビル破壊数
            GameManager::CallBuildBroken();

            //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
            //buildlifeTimeComponent->SetLifeTime(5.0f);
        }
        preSkeltalMeshComponent->SetIsCastShadow(false);
    }
}


// 衝撃波に当たった時に呼び出す関数
void Building::CallHitShockWave(float power, int beamItemCount, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
    if (hp > beamItemCount)
    {
        hp -= static_cast<int>(beamItemCount);
    }
    else
    {
        // 壊れたからHPを0にしておく
        hp = 0;
        if (TutorialSystem::GetCurrentStep() == TutorialStep::ThirdAttack)
        {
            //TutorialSystem::SetCurrentStep(TutorialStep::BossBuild);
            TutorialSystem::SetCurrentStep(TutorialStep::MoveCamera);

        }

        // 衝撃波のミッションを達成
        //TutorialSystem::AchievedAction(TutorialStep::BreakBuilds);

        // 瓦礫が崩れる音を再生する
        debriSoundComponent->Play();
        preSkeltalMeshComponent->SetIsVisible(false);
        preSkeltalMeshComponent->SetIsCastShadow(false);
        // リザルト用::ビル破壊数
        GameManager::CallBuildBroken();

        // 元々の箱の当たり判定を消す
        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");

        //this->ScheduleDestroyComponentByName("boxComponent");
        //this->DestroyComponentByName("boxComponent");
        //// ビームを消す
        //beam->SetValid(false);
        // 瓦礫を当たり判定に入れる
        if (convexComponent)
        {
            convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
            convexComponent->SetKinematic(false);
            convexComponent->SetActive(true);
        }

        // 何秒後に瓦礫を消すかを設定する
        ScheduleDeactivate(4.0f);

        // ビルからアイテムをドロップする
        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
        itemSpawner->SpawnItems(beamItemCount, true);

        // エフェクトコンポーネントに伝達
        effectExplosionComponent->SetWorldLocationDirect(GetPosition());
        effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();

        auto buildlifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        buildlifeTimeComponent->SetLifeTime(5.0f);
    }
}


//　collisionComponent　が Dynamic の物と当たった時に通る
void Building::NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)
{
#if 0
    if (hp <= 0)
    {// hp が 0 になったらそもそも通らない
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
    {// もし爆弾だったら
        // エフェクトコンポーネントに伝達
        effectExplosionComponent->SetWorldLocationDirect(hitPos);
        effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
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
    {// ビームじゃなかったら
        return;
    }
    beamCount = beam->GetItemCount();
    int beamPower = static_cast<int>(beam->GetItemPower());
    if (beamPower <= 0)
    {// ビームの power がなかったら
        return;// 後に assertion に変更
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
        // 一回目で一気に破壊されたときに hp　が減らないバグを取るため
        hp -= beamCount;

        // エフェクトコンポーネントに伝達
        effectExplosionComponent->SetWorldLocationDirect(hitPos);
        effectExplosionComponent->SetEffectPower(beam->GetItemPower());
        effectExplosionComponent->SetEffectImpulse(impulse);
        effectExplosionComponent->SetEffectNormal(normal);
        effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
        effectExplosionComponent->Activate();


        // エフェクトコンポーネントに伝達
        effectShockWaveComponent->SetWorldLocationDirect(hitPos);
        effectShockWaveComponent->SetEffectPower(shockWavePower);     //TODO:01 この power は後に buildingHP - itemPower になる予定
        effectShockWaveComponent->SetEffectImpulse(impulse);
        effectShockWaveComponent->SetEffectNormal(normal);
        effectShockWaveComponent->SetEffectType(EffectComponent::EffectType::ShockWave);
        effectShockWaveComponent->Activate();

        isDestroyed = true;
    }

#endif // 0
}

// キネマティック同士の当たり判定を検知する
void Building::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    if (hp <= 0)
    {// hp が 0 になったらそもそも通らない
        return;
    }
    auto bomb = std::dynamic_pointer_cast<Bomb>(hitPair.second->GetActor());
    if (bomb)
    {// もし爆弾だったら
        //if (hitPair.second->name() != "sphereComponent")
        //{
        //    return;
        //}
        // エフェクトコンポーネントに伝達
        effectExplosionComponent->SetWorldLocationDirect(GetPosition());
        effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
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
            effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();
            hp = 0;
            isDestroyed = true;
            isLastHitBossBuilding = true;
        }
    }

    auto beam = std::dynamic_pointer_cast<Beam>(hitPair.second->GetActor());
    if (beam)
    {// ビームなら
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
        {// ビームの power がなかったら
            return;// 後に assertion に変更
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
            // 一回目で一気に破壊されたときに hp　が減らないバグを取るため
            hp -= beamCount;

            char debugBuffer[128];
            sprintf_s(debugBuffer, sizeof(debugBuffer),
                "beam power%f, beamCount%d\n",
                beamPower_, beamCount);
            OutputDebugStringA(debugBuffer);

            OutputDebugStringA("beam is Hit building \n");

            // エフェクトコンポーネントに伝達
            effectExplosionComponent->SetWorldLocationDirect(GetPosition());
            effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
            effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
            effectExplosionComponent->Activate();

            isDestroyed = true;
        }
        beam->RegisterHit(this);
    }
    auto otherActor = hitPair.second->GetOwner();
    auto boss = dynamic_cast<RiderEnemy*>(otherActor);
    if (boss)
    {// もしボスだったら
        if (hitPair.second->name() != "capsuleComponent")
        {// ボスの体じゃなかったら
            return;
        }
        //if (auto node = boss->activeNode)
        {
            //if (node->GetName() == "Dash")
            {// ボスがダッシュ攻撃なら

                hp = 0;

                isDestroyed = true;
                isLastHitBoss = true;
            }
        }
    }
}


