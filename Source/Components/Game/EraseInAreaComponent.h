#ifndef ERASE_IN_AREA_COMPONENT_H
#define ERASE_IN_AREA_COMPONENT_H

#include <unordered_set>
#include <DirectXMath.h>
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Base/SceneComponent.h"
#include "Core/Actor.h"

class EraseInAreaComponent :public SphereComponent
{
public:
    EraseInAreaComponent(const std::string& name, std::shared_ptr<Actor> owner) : SphereComponent(name, owner) {}

    // �Փ˃C�x���g
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)override;

    
private:
    int damage = 10;  // ����͋����ɂ��Ȃ��ƃ_���[�W���Z�𔼕��ɕ������Ƃ��Ƀo�O��
    std::unordered_set<Actor*> alreadyAffected;

};

#endif // !ERASE_IN_AREA_COMPONENT_H
