#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

// C++ �W�����C�u����
#include <cstdint>
#include <string>
#include <unordered_map>

// �����C�u����

// �v���W�F�N�g�̑��̃w�b�_
#include "Components/Base/SceneComponent.h"
#include "Physics/CollisionHelper.h"
#include "Physics/Collider.h"

class CollisionComponent :public SceneComponent
{
public:
    enum class CollisionResponse
    {
        None,    // ���S�ɖ���
        Trigger, // �Փ˒ʒm�����i�����o�����Ȃ��j
        Block    // �ʒm�{�����o��
    };

    CollisionComponent(const std::string& name, std::shared_ptr<Actor> owner) : SceneComponent(name, owner) {}

    // ���݂̐ݒ���擾
    uint32_t GetCollisionLayer() const { return collisionLayer_; }

    uint32_t GetCollisionMask() const { return collisionMask_; }

    void OnRegister()override;


    // �������C���[��ݒ�i��FPlayer�AEnemy�AConvex�Ȃǁj
    void SetLayer(CollisionLayer layer)
    {
        collisionLayer_ = CollisionHelper::ToBit(layer);
    }

    // ����ɑ΂��锽����o�^ (�����F����A����)
    void SetResponseToLayer(CollisionLayer otherLayer, CollisionResponse response)
    {
        uint32_t bit = CollisionHelper::ToBit(otherLayer);
        // ����Ɣ�����o�^
        responseTable_[bit] = response;

        if (response != CollisionResponse::None)
        {
            collisionMask_ |= bit;  // �ՓˑΏۂƂ��ēo�^
        }
        else
        {
            collisionMask_ &= ~bit; // �Ώۂ��珜�O
        }
    }

    // �����̑���Ƃ͂ǂ�Ȕ��������邩���擾����֐�
    CollisionResponse GetResponseTo(const CollisionComponent* other)const
    {
        auto it = responseTable_.find(other->GetCollisionLayer());
        if (it != responseTable_.end())
        {
            return it->second;
        }
        return CollisionResponse::None;
    }

    // �Փ˔���Ώۂ��H
    bool ShouldCollideWith(const CollisionComponent* other)const
    {
        return GetResponseTo(other) != CollisionResponse::None;
    }

    // �����o������������Ώۂ��H
    bool ShouldPushOnCollisionWith(const CollisionComponent* other)const
    {
        return GetResponseTo(other) == CollisionResponse::Block;
    }

    // �Փ˃C�x���g
    virtual void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes);

    // ����CollisionComponent�Ƃ̏Փ˒ʒm
    virtual void OnCollisionEnter(CollisionComponent* other, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) {}

    // �����������ɏՌ���^����
    virtual void AddImpulse(const DirectX::XMFLOAT3& impulse) {}

    //------- physics �Œx���������邽�߂̉��z�֐�-------//
    // �V�[���ɕ���������ǉ�����
    virtual void AddToScene() {};
    virtual void SetKinematic(bool isKinematic) {};
    // �����蔻��𖳌��ɂ���
    virtual void DisableCollision() {};
    // �����蔻���L���ɂ���
    virtual void EnableCollision() {};
    // �����蔻�肪�L�����ǂ������擾����
    bool IsCollide() { return this->isCollide_; }
    // OnTrigger �� OnContact �ǂ����ɓ��邩�ݒ肷��
    virtual void SetTrigger(bool isTrigger) {};
    //virtual void Destroy()override {};
protected:
    uint32_t collisionLayer_ = 0;
    uint32_t collisionMask_ = 0;
    std::unordered_map<uint32_t, CollisionResponse> responseTable_;// ����A����

    bool isCollide_ = true;
};


#endif