#ifndef BOSS_BUILDING_H
#define BOSS_BUILDING_H

#include <chrono>
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Physics/Physics.h"
#include "Physics/DefferdPhysicsOperation.h"

#include "Components/Effect/EffectComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Game/LifeTimeComponent.h"
#include "Components/Game/EraseInAreaComponent.h"
#include "Components/Game/TimerActionComponent.h"
#include "Components/Game/ShockWaveCollisionComponent.h"
#include "Components/Transform/Transform.h"
#include "Components/Audio/AudioSourceComponent.h"

#include "Game/Actors/Beam/Beam.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Effect/ShockWave.h"

#include "Game/Utils/SpawnValidator.h"
#include "Game/Utils/ShockWaveTargetRegistry.h"
#include "Stage.h"

class BossBuilding :public Actor
{
public:
    enum class BuildingState
    {
        Idle,
        Exploding,
        Exploded,
        Destroing,
        Destroyed,
    };

    enum class Type
    {
        Special,    // 必殺技で変わる
    };
private:
    BuildingState state = BuildingState::Idle;
public:
    BossBuilding(std::string modelName) :Actor(modelName)
    {
        hp = 1;
    }

    std::shared_ptr<BuildMeshComponent> preSkeltalMeshComponent;
    //std::shared_ptr<InstancedStaticMeshComponent> preSkeltalMeshComponent;
    std::shared_ptr<EffectComponent> effectExplosionComponent;
    std::shared_ptr<EffectComponent> effectShockWaveComponent;
    std::shared_ptr<ShockWaveModelComponent> shockWaveMeshComponent;
    std::shared_ptr<ShockWaveModelComponent> bombTimerMeshComponent;
    std::shared_ptr<BoxComponet> boxComponent;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    std::shared_ptr<AudioSourceComponent> explosionSoundComponent;
    std::shared_ptr<AudioSourceComponent> debriSoundComponent;
    std::shared_ptr<SkeltalMeshComponent> bombTimerMeshComponentUnder;
    void Initialize(const Transform& transform)override
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        // 最初に描画される壊れる前のモデル
        preSkeltalMeshComponent = this->NewSceneComponent<class BuildMeshComponent>("preSkeltalMeshComponent");
        preSkeltalMeshComponent->SetModel("./Data/Models/Building/bomb_bill.gltf", false);
        //preSkeltalMeshComponent->SetModel("./Data/Effect/Models/bom_effect_out.gltf", false);
        preSkeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        CreatePsFromCSO(Graphics::GetDevice(), "./Data/Shaders/BuildingPS.cso", preSkeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        auto& model = preSkeltalMeshComponent->model;
        for (auto& material : model->materials)
        {// material を全て BLEND に変更する
            material.data.alphaMode = 2; // BLEND
        }
        float radius = 0.5f;
        float height = 3.0f;
        auto t2 = std::chrono::high_resolution_clock::now();
        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        // 最初の生成位置
        riseStart = transform.GetLocation();
        // 最終的な位置
        riseEnd = { riseStart.x,0.0f,riseStart.z };
        //auto t3 = std::chrono::high_resolution_clock::now();
        // 瓦礫に使用するモデル
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent", "preSkeltalMeshComponent");
        skeltalMeshComponent->SetModel("./Data/Models/Building/bomb_bill_hahen3.gltf", true);
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        skeltalMeshComponent->SetRelativeLocationDirect(riseEnd);
        skeltalMeshComponent->SetIsVisible(false);
        auto t4 = std::chrono::high_resolution_clock::now();
        // エフェクトコンポーネントを追加
        effectExplosionComponent = this->NewSceneComponent<class EffectComponent>("effectExplosionComponet", "preSkeltalMeshComponent");

        // 最初の壊れる前の箱の当たり判定
        boxComponent = this->NewSceneComponent<class BoxComponet>("boxComponent", "preSkeltalMeshComponent");
        boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(0.8f, 1.7f, 0.8f));
        boxComponent->SetModelHeight(height * 0.5f);
        boxComponent->SetStatic(true);
        boxComponent->SetLayer(CollisionLayer::Building);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->Initialize();
        //boxComponent->UpdateTransformImmediate();
        auto t5 = std::chrono::high_resolution_clock::now();

        AABB aabb = boxComponent->GetAABB();
        aabb.min.y = 0.0f;
        aabb.max.y = 3.0f;
        // ビルを登録しておいてそこにはアイテムを生成しないために
        //SpawnValidator::Register(aabb);

        // 衝撃波の相手を決める
        ShockWaveTargetRegistry::Register(shared_from_this());

        // 瓦礫
        convexComponent = this->NewSceneComponent<class ConvexCollisionComponent>("convexComponent", "preSkeltalMeshComponent");
        convexComponent->SetLayer(CollisionLayer::Convex);
        //convexComponent->SetResponseToLayer(CollisionLayer::Player , CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        convexComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::None);
        convexComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::None);
        convexComponent->SetActive(false);
        convexComponent->CreateConvexMeshFromModel(skeltalMeshComponent.get());
        auto t6 = std::chrono::high_resolution_clock::now();

        //        
        shockWaveMeshComponent = this->NewSceneComponent<class ShockWaveModelComponent>("shockWaveMeshComponent", "preSkeltalMeshComponent");
        shockWaveMeshComponent->SetModel("./Data/Effect/Models/blast_effect_test2.gltf");
        shockWaveMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        shockWaveMeshComponent->SetRelativeScaleDirect(DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f));
        // (2.5f, 1.0f, 2.5f)

        // 爆弾機能
        eraseInAreaComponent = this->NewSceneComponent<class EraseInAreaComponent>("eraseInAreaComponent", "preSkeltalMeshComponent");
        eraseInAreaComponent->SetRadius(2.5f);
        eraseInAreaComponent->SetMass(40.0f);
        eraseInAreaComponent->SetLayer(CollisionLayer::EraseInArea);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Trigger);
        eraseInAreaComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
        eraseInAreaComponent->Initialize();
        eraseInAreaComponent->DisableCollision();
        //eraseInAreaComponent->SetIsVisibleDebugBox(false);
        //eraseInAreaComponent->SetIsVisibleDebugShape(false);
        // 
        auto t7 = std::chrono::high_resolution_clock::now();

        bombTimerMeshComponentUnder = this->NewSceneComponent<class SkeltalMeshComponent>("bombTimerMeshComponentUnder", "preSkeltalMeshComponent");
        bombTimerMeshComponentUnder->SetModel("./Data/Effect/Models/bom_effect_out.gltf", true);
        bombTimerMeshComponentUnder->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        bombTimerMeshComponentUnder->SetIsCastShadow(false);
        bombTimerMeshComponentUnder->SetIsVisible(false);
        //bombTimerMeshComponentUnder->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
        //bombTimerMeshComponentUnder->SetRelativeScaleDirect(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));

        //explodeTimer = MathHelper::RandomRange(8.0f, 10.0f);
        explodeTimer = MathHelper::RandomRange(3.0f, 5.0f);

        bombTimerMeshComponent= this->NewSceneComponent<class ShockWaveModelComponent>("bombTimerMeshComponent", "preSkeltalMeshComponent");
        bombTimerMeshComponent->SetModel("./Data/Effect/Models/bom_effect_in.gltf");
        bombTimerMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        bombTimerMeshComponent->SetRelativeScaleDirect(DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f));
        bombTimerMeshComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 1.0f, 0.02f));
        bombTimerMeshComponent->Initialize(0.1f, 2.15f, explodeTimer, 0.0f);
        bombTimerMeshComponent->SetIsCastShadow(false);
        bombTimerMeshComponent->SetIsVisible(false);


        //std::shared_ptr<TimerActionComponent> timerActionComponent = this->NewLogicComponent<class TimerActionComponent>("timerActionComponent");
        std::shared_ptr<TimerActionComponent> timerActionComponent = this->NewSceneComponent<class TimerActionComponent>("timerActionComponent");
        timerActionComponent->SetTimer(explodeTimer, [&]()
            {
                if (state == BuildingState::Idle)
                {
                    state = BuildingState::Exploding;
                    // エフェクトコンポーネントに伝達
                    DirectX::XMFLOAT3 pos = GetPosition();
                    pos.y += 0.5f;
                    effectExplosionComponent->SetWorldLocationDirect(pos);
                    effectExplosionComponent->SetEffectPower(5.0f);
                    //effectExplosionComponent->SetEffectImpulse(impulse);
                    //effectExplosionComponent->SetEffectNormal(normal);
                    effectExplosionComponent->SetEffectType(EffectComponent::EffectType::Explosion);
                    effectExplosionComponent->Activate();
                }
            });

        //auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        //auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t2).count();
        //auto d3 = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
        //auto d4 = std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5).count();
        //auto d5 = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count();
        //auto d6 = std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t7).count();

        //char buf[256];
        //sprintf_s(buf,
        //    "Load PreMesh:%lld ms, Load debris mesh:%lld ms, Box collider init:%lld ms, Convex collider build:%lld ms, Effect & timer setup:%lld ms, Finalize:%lld ms\n",
        //    d1, d2, d3, d4, d5, d6);
        //OutputDebugStringA(buf);

        // 爆発音オーディオコンポーネント追加
        explosionSoundComponent = this->NewSceneComponent<AudioSourceComponent>("explosionSoundComponent", "preSkeltalMeshComponent");
        explosionSoundComponent->SetSource(L"./Data/Sound/SE/explosion.wav");

        // 瓦礫音のオーディオコンポーネント追加
        debriSoundComponent = this->NewSceneComponent<AudioSourceComponent>("debriSoundComponent", "preSkeltalMeshComponent");
        debriSoundComponent->SetSource(L"./Data/Sound/SE/debri.wav");

    }

    bool GetDestroy() { return hp <= 0; }

    void Finalize()override
    {
        ShockWaveTargetRegistry::Unregister(shared_from_this());
        //SpawnValidator::Unregister(shared_from_this());
    }

    // 爆弾機能
    std::shared_ptr<EraseInAreaComponent> eraseInAreaComponent;
    // 瓦礫コンポーネント
    std::shared_ptr<ConvexCollisionComponent> convexComponent;

    void Update(float deltaTime)override;

    // キネマティック同士の当たり判定を検知する
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override;

    // 衝撃波に当たった時に呼び出す関数
    void CallHitShockWave(float power, int beamItemCount, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse);

    void NotifyHit(CollisionComponent* selfComp, CollisionComponent* otherComp, Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
        //if (selfComp->name() == "shockWave")
        //{// 衝撃波と当たったら
        //    //if (auto stage = std::dynamic_pointer_cast<StageProp>(otherComp->GetActor()))
        //    {
        //        otherComp->SetKinematic(false);
        //        //hitPair.second->AddImpulse(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        //    }

        //}

        //if (selfComp->name() != "convexComponent")
        //{
        //    OutputDebugStringA(selfComp->name().c_str());
        //}
        //OutputDebugStringA(otherComp->name().c_str());

        //if (selfcomp->name() == "shockwave"/* && otheractor != this*/)
        //if (auto act = dynamic_cast<StageProp*>(otherActor))
        {
            //DirectX::XMFLOAT3 imp = { impulse.x * 50.0f,impulse.y * 50.0f,impulse.z * 50.0f };
            //otherComp->SetKinematic(false);
            //otherComp->AddImpulse(imp);
            //int a = 0;
        }
    }


    //　collisionComponent　が Dynamic の物と当たった時に通る
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override {}

    // dissolveRate　0.0f ~ 1.0f
    float GetDissolveRate() { return (convexTimer - deactivateTime) / convexTimer; }

    void SetExplodeTime(float explodeTime) { this->explodeTimer = explodeTime; }
private:
    // ビルの耐久値
    int hp = 2;

    Beam* lastHitBeam_ = nullptr;

    bool isDestroyed = false;
    bool isLastHitBomb = false;
    float deactivateTime = 1.0f;
    bool shouldDeactivate = false;
    bool isLastHitBeam = false;
    float shockWavePower = 0.0f;

    float beamPower_ = 0.0f;

    // ビームに何個アイテムがくっついているか
    int beamCount = 0;

    int restBeamPower = 0;

    void ScheduleDeactivate(float timer)
    {
        shouldDeactivate = true;
        deactivateTime = timer;
    }

    // ボスに壊されたのか
    bool isLastHitBoss = false;

    // ビルのせり上がりに使用する変数
    float riseTime = 1.0f;
    float riseTimer = 0.0f;
    DirectX::XMFLOAT3 riseStart = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 riseEnd = { 0.0f,0.0f,0.0f };
    float shakeAmplitude = 0.1f;    // 振幅
    float shakeFrequency = 10.0f;   // 振動の速さ（Hz）

    // 何秒後に瓦礫を消すか
    const float convexTimer = 4.0f;
    // ボスに突進などで破壊された時のビルからのアイテムポップ数
    const int itemCountWithBossDestroy = 3;
    // ボスに突進などで破壊された時のビルからのアイテムのライフタイム
    const float itemLifeTimer = 6.0f;
    // 衝撃波
    const float shockWaveRange = 3.5f;
    const float shockWaveTime = 0.5f;
    // ビームで破壊した時に何倍でアイテムをポップさせるか
    const float itemPop = 1.0f;
    // 何秒でビルが爆発するか
    float explodeTimer = 5.0f;
};

#endif // !BOSS_BUILDING_H
