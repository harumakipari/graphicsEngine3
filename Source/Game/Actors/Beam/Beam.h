#ifndef BEAM_H
#define BEAM_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Effect/EffectComponent.h"

// ����� enemy ���ɂ�����
// boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);

class Beam :public Actor
{
public:
    //�����t���R���X�g���N�^
    Beam(std::string actorName) :Actor(actorName) {}

    virtual ~Beam()
    {
        hitActors_.clear();
    }

    std::shared_ptr<EffectComponent> effectBeamComponent;
    std::shared_ptr<EffectComponent> effectSparkComponent;
    std::shared_ptr<SphereComponent> sphereComponent;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize()override
    {
        // �`��p�R���|�[�l���g��ǉ�
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Models/Beam/beam.gltf");
        //skeltalMeshComponent->model->isModelInMeters = false;
        SetPosition(tempPosition);
        //skeltalMeshComponent->SetIsVisible(false);
        float t = std::clamp(itemPower / itemMaxPower, 0.0f, 1.0f);
        float s = std::lerp(1.0f, 3.5f, t);
        SetScale(DirectX::XMFLOAT3(s, s, s));
        // �����蔻�苅��ǉ�
        sphereComponent = this->NewSceneComponent<class SphereComponent>("sphereComponent");
        //float r = std::lerp(0.35f, 1.225f, t);
        float r = std::lerp(0.55f, 1.225f, t);
        sphereComponent->SetRadius(r * 0.5f);
        sphereComponent->SetMass(tempMass);
        sphereComponent->SetLayer(CollisionLayer::Projectile);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
        //sphereComponent->SetKinematic(false);
        sphereComponent->Initialize();
        //sphereComponent->SetIsVisibleDebugBox(false);
        //sphereComponent->SetIsVisibleDebugShape(false);

        // �G�t�F�N�g�R���|�[�l���g��ǉ� ���������u�Ԃ���G�t�F�N�g�o������
        effectBeamComponent = this->NewSceneComponent<class EffectComponent>("effectBeamComponet", "skeltalComponent");
        effectBeamComponent->SetEffectType(EffectComponent::EffectType::Beam);
        effectBeamComponent->Activate();

        // 
        effectSparkComponent = this->NewSceneComponent<class EffectComponent>("effectSparkComponet", "skeltalComponent");
        effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
    }



    //�X�V����
    void Update(float deltaTime) override
    {
        // �r�[���� position 
        effectBeamComponent->SetWorldLocationDirect(GetPosition());
        effectBeamComponent->SetEffectPower(itemPower);
        effectBeamComponent->SetEffectMaxPower(itemMaxPower);

        float speed = 10.0f;
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.x += direction.x * speed * deltaTime;
        pos.y += direction.y * speed * deltaTime;
        pos.z += direction.z * speed * deltaTime;
        SetPosition(pos);
        int a = 0;
    }

    bool HasAlreadyHit(Actor* actor) const
    {
        return hitActors_.contains(actor);
    }
    void RegisterHit(Actor* actor)
    {
        hitActors_.insert(actor);
    }

    void SetTempMass(float mass)
    {
        this->tempMass = mass;
    }
    // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (float)
    void SetItemPower(float power) { this->itemPower = power; }
    // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (float)
    float GetItemPower() { return this->itemPower; }
    // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (int)
    void SetItemCount(int count) { this->itemCount = count; }
    // �r�[�������˂������Ƀv���C���[����`�B�����A�C�e���̌� (int)
    int GetItemCount() { return this->itemCount; }
    // �r�[���̃p���[�̍ő�l�@�v���C���[����`�B�����(float)
    void SetItemMaxPower(float maxPower) { this->itemMaxPower = maxPower; }
    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override;

    // �L�l�}�e�B�b�N���m�̓����蔻������m����
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override;

    void SetDirection(const DirectX::XMFLOAT3& dir)
    {
        direction = dir;
    }
private:
    float tempMass = 0.0f;
    // �r�[���̕ێ��A�C�e����
    int itemCount = 0;
    // �r�[���̕ێ��A�C�e������ power �Ɋ��Z��������
    float itemPower = 0.0f;

    // �r�[���̍ő�l
    float itemMaxPower = 0.0f;

    //�@�r�[������ԕ���
    DirectX::XMFLOAT3 direction = { 0.0f,0.0f,0.0f };

    // �����ɓ����������̏u�Ԃ�position��ۑ����邽�߂ɕK�v
    bool onceSetPosition = false;

    std::unordered_set<Actor*> hitActors_;
};

#endif // BEAM_H
