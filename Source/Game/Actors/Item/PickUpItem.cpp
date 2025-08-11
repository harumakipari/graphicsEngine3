#include "PickUpItem.h"
#include "Game/Utils/SpawnValidator.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Managers/ItemManager.h"

void PickUpItem::Initialize(const Transform& transform)
{
    // �`��p�R���|�[�l���g��ǉ�
    skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
    skeltalMeshComponent->SetModel("./Data/Models/Items/PickUpEnergyCore/pick_up_item.gltf");
    //skeltalMeshComponent->model->isModelInMeters = false;
    //skeltalMeshComponent->SetIsVisible(false); // �A�C�e���������Ɉ�t���[���`�悳��Ă��܂�����
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    skeltalMeshComponent->model->emission = 15.0f;
    skeltalMeshComponent->SetIsCastShadow(false);

    SetPosition(transform.GetLocation());
    SetQuaternionRotation(transform.GetRotation());
    SetScale(transform.GetScale());

    // �����蔻��̃R���|�[�l���g��ǉ�
    sphereComponent = this->NewSceneComponent<class SphereComponent>("sphereComponent", "skeltalComponent");
    sphereComponent->SetRadius(0.4f);
    sphereComponent->SetMass(40.0f);
    sphereComponent->SetModelHeight(0.4f);
    sphereComponent->SetLayer(CollisionLayer::PickUpItem);
    sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
    //sphereComponent->SetKinematic(false);
    sphereComponent->Initialize();
    //sphereComponent->SetIsVisibleDebugBox(false);
    //sphereComponent->SetIsVisibleDebugShape(false);
    SpawnValidator::Register(sphereComponent->GetAABB());

};

void PickUpItem::Initialize()
{
    // �`��p�R���|�[�l���g��ǉ�
    skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
    skeltalMeshComponent->SetModel("./Data/Models/Items/PickUpEnergyCore/pick_up_item.gltf");
    skeltalMeshComponent->SetIsVisible(false); // �A�C�e���������Ɉ�t���[���`�悳��Ă��܂�����
    skeltalMeshComponent->SetIsCastShadow(false);
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    skeltalMeshComponent->model->emission = 15.0f;
    SetPosition(tempPosition);    // ���������g����[�[�[

    // �����蔻��̃R���|�[�l���g��ǉ�
    sphereComponent = this->NewSceneComponent<class SphereComponent>("sphereComponent", "skeltalComponent");
    sphereComponent->SetRadius(0.4f);
    sphereComponent->SetMass(40.0f);
    sphereComponent->SetModelHeight(0.4f);
    sphereComponent->SetLayer(CollisionLayer::PickUpItem);
    sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    sphereComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
    //sphereComponent->SetKinematic(false);
    sphereComponent->Initialize();
    //sphereComponent->SetIsVisibleDebugBox(false);
    //sphereComponent->SetIsVisibleDebugShape(false);

    SpawnValidator::Register(sphereComponent->GetAABB());
};

void PickUpItem::Finalize()
{
    SpawnValidator::Unregister(shared_from_this());
}

//�X�V����
void PickUpItem::Update(float deltaTime)
{
    // �A�C�e���ƃv���C���[�̓����蔻�肪�����ɂȂ�̂�����邽��
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.y = 0.3f;
    SetPosition(pos);

    if (itemType == Type::FromProps)
    {// �N���A�C�e����������
        if (const auto& lifeTimeComponent = GetComponent<LifeTimeComponent>())
        {
            // �c��b�����Ԃ��Ă���
            float remainingTimer = lifeTimeComponent->GetRemainingTime();

            // �_�ł܂ł̕b��
            const float blinkThreashold = 4.0f;
            // �_�ŊԊu�b��
            const float blinkInterval = 0.005f;

            static bool visible = false;

            if (remainingTimer <= blinkThreashold)
            {
                blinkTimer += deltaTime;
                if (blinkTimer >= blinkInterval)
                {
                    visible = !visible;
                    blinkTimer = 0.0f;
                    skeltalMeshComponent->SetIsVisible(visible);
                }
            }
            else
            {
                skeltalMeshComponent->SetIsVisible(true);
                blinkTimer = 0.0f;
                visible = true;
            }

            //char b[256];
            //sprintf_s(b, "Item:LifeTime:%f isVisble:%s\n", remainingTimer, visible ? "true" : "false");
            //OutputDebugStringA(b);
        }
    }

    // ���ԋ��E
    if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(GetOwnerScene()->GetActorManager()->GetActorByName("enemy")))
    {
        if (enemy->IsEnemyJumping())
        {
            DirectX::XMFLOAT3 gearPos = enemy->GetJumpPosition();
            // �M�A�̔��a
            float radius = 2.0f;

            // ���݈ʒu�ƃM�A���S��XZ����
            float dx = pos.x - gearPos.x;
            float dz = pos.z - gearPos.z;

            float distSq = dx * dx + dz * dz;
            float radiusSq = radius * radius;

            if (distSq < radiusSq)
            {
                SetPendingDestroy();
                // �X�e�[�W��ɗN������A�C�e���� max �̒l�����߂�
                if (auto itemManager = GameManager::GetItemManager())
                {
                    if (GetType() == PickUpItem::Type::RandomSpawn)
                    {// �����_���X�|�[���̃A�C�e�����E������@�G���A�ɒʒm����
                        const auto& pos = GetPosition();
                        itemManager->DecreaseAreaItemCount(pos);
                    }
                }
            }
        }
    };


#if 0
    if (isOnGround)
    {// �n�ʂɐڒn������
        sphereComponent->SetGravity(false);
        sphereComponent->SetKinematic(true);
    }

#endif // 0
}
