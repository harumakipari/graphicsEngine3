#pragma once

#include <PxPhysicsAPI.h>

struct CollisionTrigger
{
	physx::PxActor* pxActor;
};

struct CollisionContact
{
	physx::PxActor* pxActor;
	physx::PxVec3		pxNormal;
	physx::PxVec3		pxPoint;
	physx::PxF32		pxDepth;
};

struct CollisionHit
{
	physx::PxActor* pxActor;
	physx::PxVec3		pxNormal;
	physx::PxF32		pxDepth;
};

//--------------------------
// NOTE:⑪衝突イベント通知インターフェース
//--------------------------
class CollisionEvent
{
public:
	CollisionEvent() = default;
	virtual ~CollisionEvent() = default;

	virtual void OnTriggerTouchFound(const CollisionTrigger& trigger) {}
	virtual void OnTriggerTouchLost(const CollisionTrigger& trigger) {}

	virtual void OnContactTouchFound(const CollisionContact& contact) {}
	virtual void OnContactTouchPersists(const CollisionContact& contact) {}
	virtual void OnContactTouchLost(const CollisionContact& contact) {}

	virtual void OnCollisionHit(const CollisionHit& hit) {};
};
