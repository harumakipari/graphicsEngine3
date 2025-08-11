#pragma once

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
#include "Game/Managers/TutorialSystem.h"


//#include "Objects/StageProp.h"
class Building :public Actor
{
public:
    //static inline std::shared_ptr<InstancedStaticMeshComponent> preSkeltalMeshComponent;
    Building(std::string modelName) :Actor(modelName)
    {
        for (int i = 0; i < NOISE_SIZE; ++i)
        {
            noiseTable[i] = MathHelper::RandomRange(-1.0f, 1.0f);
        }
    }

    int materialNum = 0;

    std::shared_ptr<BuildMeshComponent> preSkeltalMeshComponent;
    //std::shared_ptr<InstancedStaticMeshComponent> preSkeltalMeshComponent;
    std::shared_ptr<EffectComponent> effectExplosionComponent;
    std::shared_ptr<EffectComponent> effectShockWaveComponent;
    std::shared_ptr<ShockWaveModelComponent> shockWaveMeshComponent;
    std::shared_ptr<BoxComponet> boxComponent;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        // 最初に描画される壊れる前のモデル
        preSkeltalMeshComponent = this->NewSceneComponent<class BuildMeshComponent>("preSkeltalMeshComponent");
        preSkeltalMeshComponent->SetModel("./Data/Models/Building/build_materials.gltf", false);
        preSkeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        CreatePsFromCSO(Graphics::GetDevice(), "./Data/Shaders/BuildingPS.cso", preSkeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        auto& model = preSkeltalMeshComponent->model;
        for (auto& material : model->materials)
        {// material を全て BLEND に変更する
            material.data.alphaMode = 2; // BLEND
        }
        //preSkeltalMeshComponent->SetIsVisible(false);
        float radius = 0.5f;
        float height = 3.0f;

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
        SetEulerRotation(DirectX::XMFLOAT3(0.0f, 180.0f, 0.0f));

        // 最初の生成位置
        riseStart = transform.GetLocation();
        // 最終的な位置
        riseEnd = { riseStart.x,0.0f,riseStart.z };

        // ビルのがれきに使用するモデル
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent", "preSkeltalMeshComponent");
        skeltalMeshComponent->SetModel("./Data/Models/TestCollision/test_hahen1.gltf", true);
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        skeltalMeshComponent->SetRelativeLocationDirect(riseEnd);
        skeltalMeshComponent->SetIsVisible(false);

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
        boxComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->Initialize();
        // 当たり判定を消しておく
        //boxComponent->DisableCollision();
        boxComponent->UpdateTransformImmediate();

        AABB aabb = boxComponent->GetAABB();
        aabb.min.y = 0.0f;
        aabb.max.y = 3.0f;
        // ビルを登録しておいてそこにはアイテムを生成しないために
        //SpawnValidator::Register(shared_from_this());
        //SpawnValidator::Register(aabb);

        // 衝撃波の相手を決める
        ShockWaveTargetRegistry::Register(shared_from_this());

        // 瓦礫
        convexComponent = this->NewSceneComponent<class ConvexCollisionComponent>("convexComponent", "preSkeltalMeshComponent");
        convexComponent->SetLayer(CollisionLayer::Convex);
        convexComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::None);
        convexComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        convexComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::None);
        convexComponent->SetActive(false);
        convexComponent->CreateConvexMeshFromModel(skeltalMeshComponent.get());

        //        
        shockWaveMeshComponent = this->NewSceneComponent<class ShockWaveModelComponent>("shockWaveMeshComponent", "preSkeltalMeshComponent");
        //shockWaveMeshComponent->SetModel("./Data/Effect/Models/blast_effect_test.gltf");  // blend
        shockWaveMeshComponent->SetModel("./Data/Effect/Models/blast_effect_test2.gltf"); // opaque
        //shockWaveMeshComponent->SetModel("./Data/Effect/Models/ring.gltf");
        shockWaveMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        //shockWaveMeshComponent->SetRelativeScaleDirect(DirectX::XMFLOAT3(2.5f, 1.0f, 2.5f));
        shockWaveMeshComponent->SetRelativeScaleDirect(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
        shockWaveMeshComponent->SetIsCastShadow(false);
        // (2.5f, 1.0f, 2.5f)

        //// 爆発音オーディオコンポーネント追加
        //explosionSoundComponent = this->NewSceneComponent<AudioSourceComponent>("explosionSoundComponent", "preSkeltalMeshComponent");
        //explosionSoundComponent->SetSource(L"./Data/Sound/SE/explosion.wav");

        // 瓦礫音のオーディオコンポーネント追加
        debriSoundComponent = this->NewSceneComponent<AudioSourceComponent>("debriSoundComponent", "preSkeltalMeshComponent");
        debriSoundComponent->SetSource(L"./Data/Sound/SE/debri.wav");
    }
    //std::shared_ptr<AudioSourceComponent> explosionSoundComponent;
    std::shared_ptr<AudioSourceComponent> debriSoundComponent;
    void SetMoveTarget(DirectX::XMFLOAT3 target)
    {
        riseEnd = target;
        riseTimer = 0.0f;
    }

    // ビルをスイッチするときに呼ばれる関数
    void SwitchBuilding()
    {
        // 元々の箱の当たり判定を消す
        boxComponent->DisableCollision();
        this->ScheduleDestroyComponentByName("boxComponent");

        preSkeltalMeshComponent->SetIsVisible(false);
        preSkeltalMeshComponent->SetIsCastShadow(false);
        //auto& model = preSkeltalMeshComponent->model;
        //model->SetAlpha(0.0f);

        //this->DestroyComponentByName("boxComponent");
        // 瓦礫を当たり判定に入れる
        convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
        convexComponent->SetKinematic(false);
        convexComponent->SetActive(true);

        // 何秒後に瓦礫を消すかを設定する
        ScheduleDeactivate(convexTimer);
    }

    // 瓦礫コンポーネント
    std::shared_ptr<ConvexCollisionComponent> convexComponent;

    //
    void Finalize()override
    {
        // ビルを登録しておいてそこにはアイテムを生成しないために
        //AABB aabb = boxComponent->GetAABB();
        //SpawnValidator::Unregister(aabb);
        ShockWaveTargetRegistry::Unregister(shared_from_this());
        //SpawnValidator::Unregister(shared_from_this());
    }
    // 継承押したサブクラスの専用GUI
    void DrawImGuiDetails() override
    {
#ifdef USE_IMGUI

        ImGui::SliderInt("materialNum", &materialNum, 0, 2);
        ImGui::DragInt("hp", &hp);
#endif
    }


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
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override;

    void StartBuildDown(float downTime)
    {
        downTimer = 0.0f;
        downTime = downTime;
        downStart = GetPosition();
        downEnd = { riseStart.x,-5.0f,riseStart.z };
        isStartDown = true;
    }
    // dissolveRate　0.0f ~ 1.0f
    float GetDissolveRate() { return (convexTimer - deactivateTime) / convexTimer; }

    bool GetDestroy() { return hp <= 0; }
    //bool GetDestroy() { return isDestroyed; }
private:
    // ビルの耐久値
    int hp = 3;

    Beam* lastHitBeam_ = nullptr;

    bool isDestroyed = false;



    float deactivateTime = 1.0f;
    bool shouldDeactivate = false;

    float shockWavePower = 0.0f;

    float beamPower_ = 0.0f;

    // ビームに何個アイテムがくっついているか
    int beamCount = 0;

    void ScheduleDeactivate(float timer)
    {
        shouldDeactivate = true;
        deactivateTime = timer;
    }

    // ボスに壊されたのか
    bool isLastHitBoss = false;

    // 爆弾に壊されたのか
    bool isLastHitBomb = false;

    // 爆発ビルに壊されたのか
    bool isLastHitBossBuilding = false;

    // ビルのせり上がりに使用する変数
    float riseTime = 1.0f;
    float riseTimer = 0.0f;
    DirectX::XMFLOAT3 riseStart = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 riseEnd = { 0.0f,0.0f,0.0f };
    float shakeAmplitude = 0.1f;    // 振幅
    float shakeFrequency = 10.0f;   // 振動の速さ（Hz）

    // ビルが下に下がるのに使用するフラグ
    bool isStartDown = false;
    float downTime = 1.0f;
    float downTimer = 0.0f;
    DirectX::XMFLOAT3 downStart = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 downEnd = { 0.0f,0.0f,0.0f };

    static const int NOISE_SIZE = 256;
    float noiseTable[NOISE_SIZE];
    float smoothstep(float t)
    {
        return t * t * (3 - 2 * t);
    }
    float ValueNoise1D(float x)
    {
        int xi = static_cast<int>(std::floor(x)) & (NOISE_SIZE - 1);
        int xi2 = (xi + 1) & (NOISE_SIZE - 1);
        float t = x - std::floor(x);
        float v0 = noiseTable[xi];
        float v1 = noiseTable[xi2];
        float u = smoothstep(t);
        return (1 - u) * v0 + u * v1;
    }

    // 何秒後に瓦礫を消すか
    const float convexTimer = 4.0f;
    // ボスに突進などで破壊された時のビルからのアイテムポップ数
    int itemCountWithBossDestroy = 5;
    // ボスに突進などで破壊された時のビルからのアイテムのライフタイム
    float itemLifeTimer = 10.0f;
    // 衝撃波
    float shockWaveRange = 3.5f;
    float shockWaveTime = 0.5f;
    // ビームで破壊した時に何倍でアイテムをポップさせるか
    float itemPop = 1.1f;
    int restBeamPower = 0;
};

