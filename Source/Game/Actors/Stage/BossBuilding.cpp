#include "BossBuilding.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Actors/Stage/Bomb.h"
#include "Game/Managers/GameManager.h"
#include "Game/Managers/TutorialSystem.h"

void BossBuilding::Update(float deltaTime)
{
    {// ビルのせり上がり
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
        {// せりあがりきったら
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
        // 爆発音を再生する
        explosionSoundComponent->Play();

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

        TutorialSystem::AchievedAction(TutorialStep::BossBuild);
        //TutorialSystem::AchievedAction(TutorialStep::MoveCamera);

        // 何秒後に瓦礫を消すかを設定する
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
        // 瓦礫が崩れる音を再生する
        debriSoundComponent->Play();
        TutorialSystem::AchievedAction(TutorialStep::BossBuild);
        //TutorialSystem::AchievedAction(TutorialStep::MoveCamera);

        if (isLastHitBoss)
        {// ボスによって壊されたら
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
            // リザルト用::ビル破壊数
            GameManager::CallBuildBroken();

            // 何秒後に瓦礫を消すかを設定する
            ScheduleDeactivate(convexTimer);

            // ビルからアイテムをドロップする
            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // ボスに破壊されたときは 5 の固定値とする
            itemSpawner->SpawnItems(5, true);

            //isDestroyed = false;
            state = BossBuilding::BuildingState::Destroyed;

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
            state = BossBuilding::BuildingState::Destroyed;
            //isDestroyed = false;
        }
        else if (isLastHitBeam)
        {// ビームによって破壊したら
            // 衝撃波を追加する
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

            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 するかも
            int count = static_cast<int>(std::ceil(beamCount * itemPop));
            itemSpawner->SpawnItems(count, true);

            state = BossBuilding::BuildingState::Destroyed;
            // リザルト用::ビル破壊数
            GameManager::CallBuildBroken();
        }
        else
        {// 
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

            auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", preSkeltalMeshComponent);
            // beamCount * 2 するかも
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
        {// このオブジェクトを消す
            //convexComponent->DisableCollision();
            //this->DestroyComponentByName("convexComponent");
            //this->SetValid(false);
            this->SetPendingDestroy();
            //this->Destroy();
        }
    }
}


// 衝撃波に当たった時に呼び出す関数
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
        // 元々の箱の当たり判定を消す
        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");
        this->ScheduleDestroyComponentByName("eraseInAreaComponent");
        //this->DestroyComponentByName("boxComponent");
        // ビームを消す
        //beam->SetValid(false);
        // 瓦礫を当たり判定に入れる
        convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
        convexComponent->SetKinematic(false);
        convexComponent->SetActive(true);

        // 何秒後に瓦礫を消すかを設定する
        ScheduleDeactivate(4.0f);

        //DirectX::XMFLOAT3 pos = GetPosition();
        //pos.y += 0.5f;

        //// ビルからアイテムをドロップする
        //Transform transform(pos, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        //auto buildDroppedItem = ActorManager::CreateAndRegisterActorWithTransform<PickUpItem>("buildDroppedItem", transform);
        //auto lifeTimeComponent = buildDroppedItem->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        //lifeTimeComponent->SetLifeTime(5.0f);

        ////float itemPower = beam->GetItemCount();
        //buildDroppedItem->skeltalMeshComponent->SetIsVisible(true); // 見えるようにする

        // ビルからアイテムをドロップする
        auto itemSpawner = this->NewSceneComponentWithParent<ItemSpawnerComponent>("itemSpawnerComponent", skeltalMeshComponent);
        itemSpawner->SpawnItems(beamItemCount, true);


        auto buildlifeTimeComponent = this->NewSceneComponent<LifeTimeComponent>("lifeTimeComponent");
        //auto buildlifeTimeComponent = this->NewLogicComponent<LifeTimeComponent>("lifeTimeComponent");
        buildlifeTimeComponent->SetLifeTime(5.0f);

    }
}


// キネマティック同士の当たり判定を検知する
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
    {// hp が 0 になったらそもそも通らない
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
    {// もし爆弾だったら
        //if (hitPair.second->name() != "sphereComponent")
        //{
        //    return;
        //}
        // エフェクトコンポーネントに伝達
        effectExplosionComponent->SetWorldLocationDirect(GetPosition());
        effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
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
        {
            restBeamPower = beamCount - hp;
            // 一回目で一気に破壊されたときに hp　が減らないバグを取るため
            hp -= beamCount;
            // エフェクトコンポーネントに伝達
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
    {// もしボスだったら
        if (hitPair.second->name() != "capsuleComponent")
        {// ボスの体じゃなかったら
            return;
        }
        //if (auto node = boss->activeNode)
        {
            //if (node->GetName() == "Dash")
            {// ボスがダッシュ攻撃なら
                // エフェクトコンポーネントに伝達
                DirectX::XMFLOAT3 hitPos = GetPosition();   // ビルの座標
                hitPos.y += 0.3f;
                effectExplosionComponent->SetWorldLocationDirect(hitPos);
                //effectExplosionComponent->SetWorldLocationDirect(hitPos);
                effectExplosionComponent->SetEffectPower(5.0f); // ここ定数値にする
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
