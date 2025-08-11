#ifndef DEFFERD_PHYSICS_OPERATION_H
#define DEFFERD_PHYSICS_OPERATION_H


// Physics �� simulate ��Ɏ��s���邽�߂ɗ\�񂷂�d�g�݂̃N���X

class CollisionComponent;

class DefferdPhysicsOperation
{
public:
    enum class Type
    {
        AddScene,
        DisableCollision,
        DestroyComponent,
        SetKinematicFalse,
        SetActive,
        RemoveRigidActor
    };

    Type type_;
    CollisionComponent* target_;

    DefferdPhysicsOperation(Type type, CollisionComponent* target) :type_(type), target_(target) {}

    physx::PxRigidActor* actor_ = nullptr;

    DefferdPhysicsOperation(Type type, physx::PxRigidActor* actor)
        : type_(type), actor_(actor) {
    }
};


#endif // !DEFFERD_PHYSICS_OPERATION_H
