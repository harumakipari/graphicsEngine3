#include "CollisionComponent.h"


#include "Physics/PhysicsUtility.h"
#include "Physics/CollisionSystem.h"
#include "Core/Actor.h"

void CollisionComponent::OnRegister()
{
    std::shared_ptr<CollisionComponent> sharedThis =
        std::static_pointer_cast<CollisionComponent>(shared_from_this());
    CollisionSystem::RegisterCollisionComponent(sharedThis);
}


// è’ìÀÉCÉxÉìÉg
void CollisionComponent::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)
{
    //CollisionComponent* self = hitShapes.first;
    //if (self && self->GetOwner())
    //{
    //    self->GetOwner()->BroadcastHit(hitShapes);
    //}
}
