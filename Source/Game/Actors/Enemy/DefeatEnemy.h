#ifndef DEGEAT_ENEMY_H
#define DEGEAT_ENEMY_H
#include "Enemy.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Transform/Transform.h"

class DefeatEnemy :public Enemy
{
public:
    DefeatEnemy() = default;
    ~DefeatEnemy() override {}

    DefeatEnemy(const std::string& modelName) :Enemy(modelName) {}

    //コピーコンストラクタとコピー代入演算子を禁止にする
    DefeatEnemy(const DefeatEnemy&) = delete;
    DefeatEnemy& operator=(const DefeatEnemy&) = delete;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        // モデル確認
        skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_defeat.gltf");
        skeltalMeshComponent->SetMaterialPS("./Shader/TestPS.cso", "L_emission2");
        skeltalMeshComponent->SetMaterialPS("./Shader/TestPS.cso", "L_boss_emission");
        skeltalMeshComponent->model->cpuColor.x = 0.0f;
        //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_emission2");
        //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_boss_emission");
        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        //skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_defeat.gltf");
        const std::vector<std::string> animationFilenames =
        {
            "./Data/Models/Characters/Enemy/boss_defeat.gltf",
            //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Ability_E_InMotion.glb",
            //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Primary_Attack_Fast_A.glb",
        };
        skeltalMeshComponent->AppendAnimations(animationFilenames);
        // アニメーションコントローラーを作成
        auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
        controller->AddAnimation("DefeatKari", 0);
        controller->AddAnimation("Defeat", 1);
        //controller->AddAnimation("Defeat", 0);
        // アニメーションコントローラーをcharacterに追加
        this->SetAnimationController(controller);
        //this->StopAnimation();
        this->PlayAnimation("Defeat", false);

        bossJointComponent = this->NewSceneComponent<SphereComponent>("bossJointComponent", "skeltalComponent");
        bossJointComponent->SetRadius(1.0f);
        DirectX::XMFLOAT3 bossJoint = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
        bossJointComponent->SetRelativeLocationDirect(bossJoint);

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        //SetScale(transform.GetScale());
        SetScale({1.0f,1.0f,1.0f});
    }

    void Update(float deltaTime)override
    {
        DirectX::XMFLOAT3 bossHand = skeltalMeshComponent->model->GetJointLocalPosition("spine2_FK", skeltalMeshComponent->model->GetNodes());
        bossJointComponent->SetRelativeLocationDirect(bossHand);

        Character::Update(deltaTime);

        elapsedTime += deltaTime;
        if (elapsedTime >= 4.09f)
        {
            skeltalMeshComponent->model->cpuColor.x = 1.0f;
        }

        if (!animationController_->IsPlayAnimation())
        {// アニメーションが終わったら
            if (onAnimationFinished)
            {
                onAnimationFinished();
            }
            isFinish = true;
            onAnimationFinished = nullptr;
        }

    }
    std::shared_ptr<SphereComponent> bossJointComponent = nullptr;
    std::function<void()> onAnimationFinished = nullptr;
    //bool onAnimationFinished

    bool isFinish = false;
private:
    float elapsedTime = 0.0f;
};

#endif // !DEGEAT_ENEMY_H
