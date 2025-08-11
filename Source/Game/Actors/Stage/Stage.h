#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
class Stage :public Actor
{
public:
    Stage(std::string modelName) :Actor(modelName)
    {
        //auto modelComponent = std::make_shared<StaticMeshComponent>(this, "..\\glTF-Sample-Models-main\\original\\ExampleStage.gltf");
        //AddComponent(modelComponent);

        //scale = { 10.0f,10.0f,10.0f };
        //position.y = -3.0f;
    }

    void Initialize()override
    {
        std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->NewSceneComponent<class StaticMeshComponent>("staticMeshComponent");
        //staticMeshComponent->SetModel("./Assets/Models/Stage/ExampleStage.gltf", true);
        staticMeshComponent->SetModel("./Data/Models/Stage/stage.gltf", true);
        //staticMeshComponent->model->isModelInMeters = false;
        staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        SetEulerRotation(DirectX::XMFLOAT3(0.0f, 180.0f, 0.0f));
        //std::shared_ptr<SkeltalMeshComponent> staticMeshComponent = this->NewComponent<class SkeltalMeshComponent>("staticMeshComponent");
        //staticMeshComponent->SetModel("./Assets/Models/Stage/ExampleStage.gltf", true);
        staticMeshComponent->SetRelativeScaleDirect(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
        staticMeshComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
        // �����蔻��̃R���|�[�l���g��ǉ�
        std::shared_ptr<BoxComponet> boxComponent = this->NewSceneComponent<class BoxComponet>("boxComponent", "staticMeshComponent");
        boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(20.0f, 0.2f, 20.0f));
        //boxComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 0.2f, 0.0f));
        boxComponent->SetModelHeight(1.0f * 0.5f);
        SetPosition(DirectX::XMFLOAT3(0.0f, -0.0f, 0.0f));
        boxComponent->SetStatic(true);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        //boxComponent->SetKinematic(false); // dynamic �ɂ���
        //boxComponent->AddCollisionWith(CollisionLayer::Player);
        //boxComponent->AddCollisionWith(CollisionLayer::WorldStatic);
        //boxComponent->AddCollisionWith(CollisionLayer::Convex);
        boxComponent->Initialize();


        // ��O�̕Ǔ����蔻��̃R���|�[�l���g��ǉ�
        std::shared_ptr<BoxComponet> wallFrontComponent = this->NewSceneComponent<class BoxComponet>("wallFrontComponent", "staticMeshComponent");
        wallFrontComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(20.0f, 6.0f, 1.0f));
        wallFrontComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 0.0f, 15.6f));
        wallFrontComponent->SetModelHeight(6.0f * 0.5f);
        wallFrontComponent->SetStatic(true);
        wallFrontComponent->SetLayer(CollisionLayer::WorldStatic);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        wallFrontComponent->Initialize();

        // ���̕Ǔ����蔻��̃R���|�[�l���g��ǉ�
        std::shared_ptr<BoxComponet> wallBackComponent = this->NewSceneComponent<class BoxComponet>("wallBackComponent", "staticMeshComponent");
        wallBackComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(20.0f, 6.0f, 1.0f));
        wallBackComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(0.0f, 0.0f, -15.6f));
        wallBackComponent->SetModelHeight(6.0f * 0.5f);
        wallBackComponent->SetStatic(true);
        wallBackComponent->SetLayer(CollisionLayer::WorldStatic);
        wallBackComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        wallBackComponent->Initialize();

        // �E�̕Ǔ����蔻��̃R���|�[�l���g��ǉ�
        std::shared_ptr<BoxComponet> rightWallComponent = this->NewSceneComponent<class BoxComponet>("rightWallComponent", "staticMeshComponent");
        rightWallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.0f, 6.0f, 16.0f));
        rightWallComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(20.5f, 0.0f, 0.6f));
        rightWallComponent->SetModelHeight(6.0f * 0.5f);
        rightWallComponent->SetStatic(true);
        rightWallComponent->SetLayer(CollisionLayer::WorldStatic);
        rightWallComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        rightWallComponent->Initialize();

        // ���̕Ǔ����蔻��̃R���|�[�l���g��ǉ�
        std::shared_ptr<BoxComponet> leftWallComponent = this->NewSceneComponent<class BoxComponet>("leftWallComponent", "staticMeshComponent");
        leftWallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.0f, 6.0f, 16.0f));
        leftWallComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-20.5f, 0.0f, 0.6f));
        leftWallComponent->SetModelHeight(6.0f * 0.5f);
        leftWallComponent->SetStatic(true);
        leftWallComponent->SetLayer(CollisionLayer::WorldStatic);
        leftWallComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        leftWallComponent->Initialize();



        //std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->NewSceneComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent", "staticMeshComponent");
        //triangleMeshComponent->CreateConvexMeshFromModel(staticMeshComponent.get());
    }

    void Update(float elapsedTime)override {}
};