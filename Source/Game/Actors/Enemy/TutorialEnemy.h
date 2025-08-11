#ifndef TUTORIAL_ENEMY_H
#define TUTORIAL_ENEMY_H

#include "Enemy.h"
#include "Core/ActorManager.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Transform/Transform.h"
#include "Game/Actors/Beam/Beam.h"
#include "Game/Managers/TutorialSystem.h"
#include "Game/Actors/Stage/BossBuilding.h"
#include "Components/Audio/AudioSourceComponent.h"

#include "Widgets/Mask.h"

#include "Engine/Scene/Scene.h"

class TutorialEnemy :public Enemy
{
public:
    TutorialEnemy() = default;
    ~TutorialEnemy() override {}

    TutorialEnemy(const std::string& modelName) :Enemy(modelName) {}

    //コピーコンストラクタとコピー代入演算子を禁止にする
    TutorialEnemy(const TutorialEnemy&) = delete;
    TutorialEnemy& operator=(const TutorialEnemy&) = delete;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        // モデル確認
        skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_idle.gltf");
        skeltalMeshComponent->SetMaterialPS("./Shader/TestPS.cso", "L_emission2");
        skeltalMeshComponent->SetMaterialPS("./Shader/TestPS.cso", "L_boss_emission");
        //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_emission2");
        //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_boss_emission");

        const std::vector<std::string> animationFilenames =
        {
            "./Data/Models/Characters/Enemy/jump_landing2.gltf",
        };
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        skeltalMeshComponent->AppendAnimations(animationFilenames);

        // アニメーションコントローラーを作成
        auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
        controller->AddAnimation("Idle", 0);
        controller->AddAnimation("JumpLanding", 1);
        // アニメーションコントローラーをcharacterに追加
        this->SetAnimationController(controller);
        this->PlayAnimation("Idle");
        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        std::shared_ptr<BoxComponet> boxComponent = this->NewSceneComponent<class BoxComponet>("capsuleComponent", "skeltalComponent");
        boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.5f, 2.1f, 1.0f));
        boxComponent->SetModelHeight(2.1f * 0.5f);
        //boxComponent->SetMass(40.0f);
        boxComponent->SetStatic(true);
        boxComponent->SetLayer(CollisionLayer::Enemy);
        boxComponent->SetResponseToLayer(CollisionLayer::EnemyHand, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::None);
        boxComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->Initialize();
        hp = 210;

        AddHitCallback([&](std::pair<CollisionComponent*, CollisionComponent*> hitPair)
            {
                if (auto beam = std::dynamic_pointer_cast<Beam>(hitPair.second->GetActor()))
                {// ビームに当たったら
                    if (beam->HasAlreadyHit(this))
                    {
                        return;
                    }
                    if (hitPair.first->name() != "capsuleComponent")
                    {
                        return;
                    }
                    // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (int)
                    int beamPower = beam->GetItemCount();
                    hp -= beamPower;
                    if (hp <= 10)
                    {
                        hp = 10;
                    }
                    beam->RegisterHit(this);
                }
            });

        isBuildBoss = false;

        buildAudioComponet = this->NewSceneComponent<AudioSourceComponent>("buildAudioComponet", "skeltalComponent");
        buildAudioComponet->SetSource(L"./Data/Sound/SE/bill_spawn.wav");
    }
    std::shared_ptr<AudioSourceComponent> buildAudioComponet;
    void Update(float deltaTime)override
    {
        float normalizedHP = static_cast<float>(GetHp() / 210.0f);
        if (auto hpGauge = ObjectManager::Find("BossHP"))
        {
            hpGauge->GetComponent<Mask>()->valueX = normalizedHP;
            ObjectManager::Find("BossEnergy")->GetComponent<Mask>()->valueX = 0.0f;
        }

        Character::Update(deltaTime);
        if (TutorialSystem::GetCurrentStep() == TutorialStep::BossBuild && !isPlayAnimation)
        {
            // シェーダーを変更する
            skeltalMeshComponent->model->cpuColor.x = 0.0f;
            PlayAnimation("JumpLanding", false);
            isPlayAnimation = true;
        }
        if (!animationController_->IsPlayAnimation() && !isBuildBoss)
        {
            buildAudioComponet->Play();
            Transform buildTr(DirectX::XMFLOAT3(4.0f, -3.0f, -3.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
            auto bossBuild = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<BossBuilding>("bossBuild", buildTr);
            Transform buildTr1(DirectX::XMFLOAT3(-4.0f, -3.0f, -3.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
            auto bossBuild1 = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<BossBuilding>("bossBuild", buildTr1);
            Transform buildTr2(DirectX::XMFLOAT3(4.0f, -3.0f, 3.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
            auto bossBuild2 = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<BossBuilding>("bossBuild", buildTr2);
            Transform buildTr3(DirectX::XMFLOAT3(-4.0f, -3.0f, 3.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
            auto bossBuild3 = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<BossBuilding>("bossBuild", buildTr3);
            isBuildBoss = true;
            //PlayAnimation("Idle", true);
        }
        if (!animationController_->IsPlayAnimation())
        {
            isFinishBuildAnimation = true;
            PlayAnimation("Idle", true);
        }
    }

    bool IsFinishBuildAnimation()
    {
        return isFinishBuildAnimation;
    }
private:
    // ビル爆弾を湧かせるフラグ
    bool isBuildBoss = false;
    bool isPlayAnimation = false;

    //　ビル生成のアニメーションが終わったフラグ
    bool isFinishBuildAnimation = false;
};

#endif // !TUTORIAL_ENEMY_H
