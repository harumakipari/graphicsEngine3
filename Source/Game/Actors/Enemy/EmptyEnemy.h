#ifndef EMPTY_ENEMY_H
#define EMPTY_ENEMY_H



#include "Enemy.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Transform/Transform.h"
#include "Game/Actors/Beam/Beam.h"
#include "Game/Managers/TutorialSystem.h"

class EmptyEnemy :public Enemy
{
public:
    EmptyEnemy() = default;
    ~EmptyEnemy() override {}

    EmptyEnemy(const std::string& modelName) :Enemy(modelName) {}

    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
    EmptyEnemy(const EmptyEnemy&) = delete;
    EmptyEnemy& operator=(const EmptyEnemy&) = delete;

    void Initialize(const Transform& transform)override
    {
        std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        // ���f���m�F
        skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_idle.gltf");
        skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_emission2");
        skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_boss_emission");

        skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
        // �A�j���[�V�����R���g���[���[���쐬

        const std::vector<std::string> animationFilenames =
        {
            "./Data/Models/Characters/Enemy/rotate.gltf",
        };

        skeltalMeshComponent->AppendAnimations(animationFilenames);

        auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
        controller->AddAnimation("Idle", 0);
        controller->AddAnimation("Rotate", 1);
        // �A�j���[�V�����R���g���[���[��character�ɒǉ�
        this->SetAnimationController(controller);
        this->PlayAnimation("Idle");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
    }
};



#endif // !EMPTY_ENEMY_H
