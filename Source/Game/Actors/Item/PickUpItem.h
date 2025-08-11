#ifndef PICK_UP_ITEM_H
#define PICK_UP_ITEM_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Game/LifeTimeComponent.h"

#include "Game/Actors/Stage/Stage.h"

// �X�e�[�W��ɗN���A�C�e���N���X
class PickUpItem :public Actor
{
public:
    // �f�t�H���g�̓����_���X�|�[���ɂ���
    enum class Type
    {
        RandomSpawn, // �����_���X�|�[���̃A�C�e��
        FromProps,  // �r���⎩�̋@�Ȃǂ���N�����A�C�e��
    };


public:
    //�����t���R���X�g���N�^
    PickUpItem(std::string modelName) :Actor(modelName)
    {
    }

    virtual ~PickUpItem()
    {
        hitActors_.clear();
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override;

    void Finalize()override;

    void Initialize()override;

    std::shared_ptr<SphereComponent> sphereComponent;
    Type GetType() const { return itemType; }

    void SetType(const Type& type) { this->itemType = type; }

    //�X�V����
    void Update(float deltaTime) override;

    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override
    {
        //if (auto stage = dynamic_cast<Stage*>(otherActor))
        //{
        //    isOnGround = true;
        //}
    }
    bool HasAlreadyHit(Actor* actor) const
    {
        return hitActors_.contains(actor);
    }
    void RegisterHit(Actor* actor)
    {
        hitActors_.insert(actor);
    }
private:
    float radius = 0.5f;
    Type itemType = Type::RandomSpawn;  // �f�t�H���g�̓����_���X�|�[���ɂ���
    bool isOnGround = false;
    float blinkTimer = 0.0f;
    std::unordered_set<Actor*> hitActors_;
};

#endif //PICK_UP_ITEM_H
