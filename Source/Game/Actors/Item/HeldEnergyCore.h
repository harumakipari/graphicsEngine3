#ifndef HELD_ENERGY_CORE_H
#define HELD_ENERGY_CORE_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

// �v���C���[����������A�C�e���N���X
class HeldEnergyCore :public Actor
{
public:
    //�����t���R���X�g���N�^
    HeldEnergyCore(std::string modelName) :Actor(modelName) {}

    virtual ~HeldEnergyCore() = default;

    void Initialize()override
    {
        // �e���O�����ɐݒ肵�Ă���
        if (!rootComponent_)
        {
            _ASSERT("�e�̃R���|�[�l���g���ݒ肳��Ă��܂���B");
        }
        // �`��p�R���|�[�l���g��ǉ�
        std::shared_ptr<SkeltalMeshComponent>skeltalMeshComponent = this->NewSceneComponentWithParent<class SkeltalMeshComponent>("skeltalComponent", rootComponent_ ? rootComponent_ : nullptr);
        skeltalMeshComponent->SetModel("./Data/Models/Items/HeldEnergyCore/heldEnergyCore.gltf");
        //skeltalMeshComponent->model->isModelInMeters = false;
        SetScale(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
        //skeltalMeshComponent->SetIsVisible(false); // �A�C�e���������Ɉ�t���[���`�悳��Ă��܂�����
        skeltalMeshComponent->SetRelativeLocationDirect(tempPosition); // player �̉��ɐ������� position
        //// �����蔻��̃R���|�[�l���g��ǉ�
        //std::shared_ptr<SphereComponent> sphereComponent = this->NewComponent<class SphereComponent>("sphereComponent", rootComponent_ ? rootComponent_ : nullptr);
        //sphereComponent->SetRadius(0.1f);
        //sphereComponent->SetModelHeight(0.1f);
        //sphereComponent->SetMass(40.0f);
        //sphereComponent->SetRelativeLocationDirect(tempPosition); // player �̉��ɐ������� position
        //sphereComponent->SetLayer(CollisionLayer::HeldItem);    // player �̉��ɏ�������A�C�e��
        //// �����o�����肷�����
        //sphereComponent->SetResponseToLayer(CollisionLayer::HeldItem, CollisionComponent::CollisionResponse::None); // �n��
        //sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::None); // �n��
        //sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block); // �n��
        //sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block); // ���I
        //sphereComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);  // �r��
        //sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);  // �r��
        //sphereComponent->Initialize();


    }

    // �X�V����
    void Update(float deltaTime)override {}

    float radius = 0.5f;
};

#endif //HELD_ENERGY_CORE_H