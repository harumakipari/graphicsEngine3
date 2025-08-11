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

    // 衝突イベント
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)override;

    
private:
    int damage = 10;  // これは偶数にしないとダメージ換算を半分に分けたときにバグる
    std::unordered_set<Actor*> alreadyAffected;

};

#endif // !ERASE_IN_AREA_COMPONENT_H
