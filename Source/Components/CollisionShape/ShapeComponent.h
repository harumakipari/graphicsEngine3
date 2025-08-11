#ifndef SHAPE_COMPONENT_H
#define SHAPE_COMPONENT_H

// C++ 標準ライブラリ
#include <memory>
#include <optional>
#include <string>

// 他ライブラリ
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// プロジェクトの他のヘッダ
#include "Physics/Collider.h"
#include "Physics/Physics.h"
#include "Physics/CollisionHelper.h"
#include "Components/Base/SceneComponent.h"
#include "RigidBodyComponent.h"

// ConvexMeshComponentClass で使用している
#include "Graphics/Resource/InterleavedGltfModel.h"
#include "Components/Render/MeshComponent.h"
#include "CollisionComponent.h"

class ShapeComponent :public CollisionComponent
{
public:
    // 物理演算を有効にするかどうか
    enum class PhysicsControlMode
    {
        Kinematic,     // SetWorldLocationでTransform制御
        Dynamic,        // 物理演算がTransformを制御（PxRigidDynamicの座標がSceneComponentに反映される）
    };

    // ゲーム内でカプセルがどの軸に長いか
    enum class CapsuleAxis
    {
        x, y, z
    };
protected:
    ShapeComponent::PhysicsControlMode physicsMode = ShapeComponent::PhysicsControlMode::Kinematic; // physics Mode を有効にする

    ShapeComponent::CapsuleAxis capsuleAxis = CapsuleAxis::y;   // デフォルトは Y 軸方向

    //std::shared_ptr<RigidBodyComponent> rigidBody_;// 物理ボディを内包
    std::shared_ptr<SingleRigidBodyComponent> rigidBody_;// 物理ボディを内包

    // 物理形状情報を返すもの
    struct PhysicsShapeInfo
    {
        physx::PxGeometryHolder geometry;   // 全ての形状を保持できるユーティリティ
        physx::PxTransform transform;       //ワールド変換
    };
public:
    ShapeComponent(const std::string& name, std::shared_ptr<Actor> owner) :CollisionComponent(name, owner) {}

    virtual void Initialize()override;

    bool IsKinematic()const
    {
        return physicsMode == PhysicsControlMode::Kinematic;
    }

    // キネマティックにするかどうか　
    void SetKinematic(bool isKinematic)override
    {
        physicsMode = isKinematic ? PhysicsControlMode::Kinematic : PhysicsControlMode::Dynamic;
        if (rigidBody_)
        {
            rigidBody_->SetKinematic(isKinematic);
        }
    }
    // OnTrigger か OnContact どっちに入るか設定する
    //　これは Initialize の前に設定する　デフォルトは false
    void SetTrigger(bool isTrigger)override
    {
        if (rigidBody_)
        {
            rigidBody_->SetTrigger(isTrigger);
        }
    }

    void SetGravity(bool isUseGravity)
    {
        if (rigidBody_)
        {
            rigidBody_->SetGravity(isUseGravity);
        }
    }

    virtual void Tick(float deltaTime) override
    {
        if (rigidBody_)
        {
            rigidBody_->Tick(deltaTime);
        }
    }
    // ゲーム内でカプセルがどの軸に長いか取得
    const ShapeComponent::CapsuleAxis& GetCapusleAxis() const { return capsuleAxis; }
    // ゲーム内でカプセルがどの軸に長いか設定
    void SetCapsuleAxis(const ShapeComponent::CapsuleAxis& axis) { this->capsuleAxis = axis; }

    // 初速を設定する
    void SetIntialVelocity(const DirectX::XMFLOAT3& velocity)
    {
        rigidBody_->SetIntialVelocity(velocity);
    }

    // 当たり判定レイヤーを途中で追加する
    void AddCollisionFilter(CollisionLayer otherLayer, CollisionComponent::CollisionResponse response)
    {
        SetResponseToLayer(otherLayer, response);
        rigidBody_->SetCollisionFilter(GetCollisionLayer(), GetCollisionMask());
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  debugRender").c_str()))
        {
            ImGui::Checkbox("isVisibleDebugBox", &isVisibleDebugBox_);
            ImGui::Checkbox("isVisibleDebugShape", &isVisibleDebugShape_);
            ImGui::TreePop();
        }
#endif
    }
    void SetIsCollisionEnabled(bool enabled) { this->isCollisionEnabled_ = enabled; }

    bool GetIsCollisionEnabled() { return isCollisionEnabled_; }

    virtual AABB GetAABB() const = 0;

    const float& GetRadius() const { return radius_; }

    void SetModelHeight(float modelHeight) { modelHeight_ = modelHeight; }

    const float& GetModelHeight() const { return modelHeight_; }

    DirectX::XMFLOAT3 GetSizeFromAABB(const AABB& aabb)
    {
        return
        {
            aabb.max.x - aabb.min.x,
            aabb.max.y - aabb.min.y,
            aabb.max.z - aabb.min.z
        };
    }

    // 当たった時に衝撃を与える
    void AddImpulse(const DirectX::XMFLOAT3& impulse)override
    {
        if (rigidBody_)
        {
            rigidBody_->AddImpulse(impulse);
        }
    }

    virtual PhysicsShapeInfo GetPhysicsShapeInfo()const = 0;

    virtual void SetMass(float m) { this->mass_ = m; }

    virtual float GetMass() { return this->mass_; }

    void SetStatic(bool isStatic)
    {
        isStatic_ = isStatic;
        if (isStatic)
        {
            physicsMode = PhysicsControlMode::Kinematic;
            mass_ = 0.0f;
            if (rigidBody_)
                rigidBody_->SetKinematic(true);
        }
    }

    bool IsStatic() const { return isStatic_; }

    // 当たり判定を無効にする
    void DisableCollision() override
    {
        isCollide_ = false;
        if (rigidBody_)
        {
            rigidBody_->DisableCollision();
        }
    };

    // 当たり判定を有効にする
    void EnableCollision() override
    {
        isCollide_ = true;
        if (rigidBody_)
        {
            rigidBody_->EnableCollision();
        }
    };

    virtual void OnUnregister()override
    {
        if (rigidBody_)
        {
            rigidBody_->Destroy();
            rigidBody_ = nullptr;
        }
    }

    // デバック箱を描画するか
    bool IsVisibleDebugBox() { return isVisibleDebugBox_; }

    void SetIsVisibleDebugBox(bool isVisibleDebugBox) { this->isVisibleDebugBox_ = isVisibleDebugBox; }

    // デバックの形状を描画するか
    bool IsVisibleDebugShape() { return isVisibleDebugShape_; }

    void SetIsVisibleDebugShape(bool isVisibleDebugShape) { this->isVisibleDebugShape_ = isVisibleDebugShape; }

    const std::string& GetCollisionType()const { return collisionTypeName_; }
protected:
    bool isStatic_ = false;

    bool isVisibleDebugBox_ = true;
    bool isVisibleDebugShape_ = true;

    bool isCollisionEnabled_ = true;

    float radius_ = 0.0f;
    float modelHeight_ = 0.0f; // physxの当たり判定の原点が埋まらないようにするために使用

    // のちに責任を分離
    float mass_ = 1.0f; // defaultで 1.0kg にしておく

    std::string collisionTypeName_ = "";
};


class BoxComponet :public ShapeComponent
{
    DirectX::XMFLOAT3 boxExtent_ = { 1.0f,1.0f,1.0f };

public:
    BoxComponet(const std::string& name, std::shared_ptr<Actor> owner) :ShapeComponent(name, owner)
    {
        collisionTypeName_ = "Box";
    }

    void SetHalfBoxExtent(const DirectX::XMFLOAT3& extent)//boxの半径
    {
        boxExtent_ = extent;
    }

    AABB GetAABB()const override
    {
        AABB aabb{};
        //DirectX::XMFLOAT3 center = attachParent_.lock()->GetWorldPosition();
        DirectX::XMFLOAT3 center = GetComponentLocation();
        aabb.min = { center.x - boxExtent_.x,center.y - boxExtent_.y,center.z - boxExtent_.z };
        aabb.max = { center.x + boxExtent_.x,center.y + boxExtent_.y,center.z + boxExtent_.z };
        return aabb;
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        ShapeComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  box").c_str()))
        {
            ImGui::DragFloat3("boxExtent", &boxExtent_.x, 0.1f);
            ImGui::TreePop();
        }
#endif
    }

    // 動的にボックスの判定を大きくする関数
    void ResizeBox(float newExtentX, float newExtentY, float newExtentZ)
    {
        rigidBody_->ResizeBox(newExtentX, newExtentY, newExtentZ);
        boxExtent_.x = newExtentX;
        boxExtent_.y = newExtentY;
        boxExtent_.z = newExtentZ;
    }

    PhysicsShapeInfo GetPhysicsShapeInfo()const override
    {
        PhysicsShapeInfo info;
        info.geometry = physx::PxBoxGeometry(boxExtent_.x, boxExtent_.y, boxExtent_.z);
        //DirectX::XMFLOAT3 pos = attachParent_.lock()->GetWorldPosition();
        const Transform& worldT = GetComponentWorldTransform();
        DirectX::XMFLOAT3 pos = worldT.GetLocation();
        DirectX::XMFLOAT4 rot = worldT.GetRotation();

        DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&rot);
        q = DirectX::XMQuaternionNormalize(q);
        DirectX::XMStoreFloat4(&rot, q);

        info.transform = physx::PxTransform{
            physx::PxVec3(pos.x,pos.y,pos.z),
            physx::PxQuat(rot.x,rot.y,rot.z,rot.w)
        };

        return info;
    }
private:
};

class SphereComponent :public ShapeComponent
{
public:
    SphereComponent(const std::string& name, std::shared_ptr<Actor> owner) :ShapeComponent(name, owner)
    {
        collisionTypeName_ = "Sphere";
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        ShapeComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  sphere").c_str()))
        {
            ImGui::DragFloat("radius", &radius_, 0.1f);
            ImGui::TreePop();
        }
#endif
    }

    Sphere ToSphere()const
    {
        Sphere s{};
        //s.c = Point(attachParent_.lock()->GetWorldPosition());
        s.c = Point(GetComponentWorldTransform().GetLocation());
        s.r = radius_;
        return s;
    }

    PhysicsShapeInfo GetPhysicsShapeInfo()const override
    {
        PhysicsShapeInfo info;
        info.geometry = physx::PxSphereGeometry(radius_);
        DirectX::XMFLOAT3 pos = GetComponentWorldTransform().GetLocation();
        //DirectX::XMFLOAT3 pos = attachParent_.lock()->GetWorldPosition();
        info.transform = physx::PxTransform{ {pos.x,pos.y,pos.z} };
        return info;
    }

    void SetRadius(float radius)
    {
        this->radius_ = radius;
    }

    AABB GetAABB()const override
    {
        //DirectX::XMFLOAT3 pos = attachParent_.lock()->GetWorldPosition();
        DirectX::XMFLOAT3 pos = GetComponentWorldTransform().GetLocation();
        AABB aabb;
        aabb.min = { pos.x - radius_,pos.y - radius_,pos.z - radius_ };
        aabb.max = { pos.x + radius_,pos.y + radius_,pos.z + radius_ };
        return aabb;
    }

    // 動的に球の当たり判定の半径を変更する関数
    void ResizeSphere(float radius)
    {
        radius_ = radius;
        rigidBody_->ResizeSphere(radius);
    }

    float GetRadius() { return radius_; }
private:
};

class CapsuleComponent :public ShapeComponent
{
    float height = 0.0f;
public:
    CapsuleComponent(const std::string& name, std::shared_ptr<Actor> owner) :ShapeComponent(name, owner)
    {
        collisionTypeName_ = "Capsule";
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        ShapeComponent::DrawImGuiInspector();

        if (ImGui::TreeNode((name_ + "  capsule").c_str()))
        {
            ImGui::DragFloat("radius", &radius_, 0.1f);
            ImGui::DragFloat("height", &height, 0.1f);
            ImGui::TreePop();
        }
#endif
    }


    Capsule ToCapsule()const
    {
        Capsule c{};
        //c.a = Point(attachParent_.lock()->GetWorldPosition().x, attachParent_.lock()->GetWorldPosition().y + height - radius, attachParent_.lock()->GetWorldPosition().z);
        //c.b = DirectX::XMFLOAT3(attachParent_.lock()->GetWorldPosition().x, attachParent_.lock()->GetWorldPosition().y + radius, attachParent_.lock()->GetWorldPosition().z);
        c.a = Point(GetComponentWorldTransform().GetLocation().x, GetComponentWorldTransform().GetLocation().y + height - radius_, GetComponentWorldTransform().GetLocation().z);
        c.b = DirectX::XMFLOAT3(GetComponentWorldTransform().GetLocation().x, GetComponentWorldTransform().GetLocation().y + radius_, GetComponentWorldTransform().GetLocation().z);
        c.r = radius_;
        return c;
    }

    AABB GetAABB() const override
    {
        auto pos = GetComponentWorldTransform().GetLocation();
        //auto pos = attachParent_.lock()->GetWorldPosition();
        float halfHeight = height * 0.5f;
        DirectX::XMFLOAT3 a = { pos.x, pos.y + height , pos.z };
        DirectX::XMFLOAT3 b = { pos.x, pos.y + radius_, pos.z };

        AABB box;
        box.min.x = std::min<float>(a.x, b.x) - radius_;
        box.min.y = std::min<float>(a.y, b.y) - radius_;
        box.min.z = std::min<float>(a.z, b.z) - radius_;

        box.max.x = std::max<float>(a.x, b.x) + radius_;
        box.max.y = std::max<float>(a.y, b.y) + radius_;
        box.max.z = std::max<float>(a.z, b.z) + radius_;

        return box;
    }

    PhysicsShapeInfo GetPhysicsShapeInfo()const override
    {
        PhysicsShapeInfo info;
        info.geometry = physx::PxCapsuleGeometry(radius_, height * 0.5f);
        //DirectX::XMFLOAT3 pos = GetComponentWorldTransform().GetLocation();
        //DirectX::XMFLOAT3 pos = attachParent_.lock()->GetWorldPosition();
        //pos = attachParent_.lock()->GetLocalPosition();

        const Transform& worldT = GetComponentWorldTransform();
        DirectX::XMFLOAT3 pos = worldT.GetLocation();
        DirectX::XMFLOAT4 rot = worldT.GetRotation();

        DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&rot);
        q = DirectX::XMQuaternionNormalize(q);
        DirectX::XMStoreFloat4(&rot, q);

        info.transform = physx::PxTransform{
            physx::PxVec3(pos.x,pos.y,pos.z),
            physx::PxQuat(rot.x,rot.y,rot.z,rot.w)
        };

        //info.transform = physx::PxTransform{ {pos.x,pos.y,pos.z} };

        //if (capsuleAxis == ShapeComponent::CapsuleAxis::y)
        //{
        //    info.transform.q = physx::PxQuat(physx::PxPiDivTwo, physx::PxVec3(0.0f, 0.0f, 1.0f));	//90度回転させて
        //}
        //else if (capsuleAxis == ShapeComponent::CapsuleAxis::x)
        //{
        //    info.transform.q = physx::PxQuat(physx::PxPiDivTwo, physx::PxVec3(0.0f, 1.0f, 0.0f));	//90度回転させて
        //}
        //else
        //{
        //    info.transform.q = physx::PxQuat(physx::PxPiDivTwo, physx::PxVec3(0.0f, 0.0f, 0.0f));	//回転不要
        //}
        return info;
    }

    void SetRadiusAndHeight(float radius, float height) { this->radius_ = radius; this->height = height; }

    float GetRadius() { return radius_; }
    float GetHeight() { return height; }


    // 動的にカプセルの判定を大きくする関数
    void ResizeCapsule(float newRadius, float newHeight)
    {
        rigidBody_->ResizeCapsule(newRadius, newHeight);
        radius_ = newRadius;
        height = newHeight;
    }

private:
};

class ConvexCollisionComponent :public CollisionComponent
{
public:
    ConvexCollisionComponent(const std::string& name, std::shared_ptr<Actor> owner) :CollisionComponent(name, owner) {}

    virtual void Initialize()override
    {
        //InitializePhysics();
    }

    void Tick(float deltaTime)override
    {
        if (rigidBody_)
        {
            rigidBody_->Tick(deltaTime);
        }
    }

    // モデルから ConvexMesh を作成する
    void CreateConvexMeshFromModel(MeshComponent* meshComponent);

    void SetKinematic(bool isKinematic)override
    {
        rigidBody_->SetKinematic(isKinematic);
    }

    void AddToScene()override
    {
        rigidBody_->AddToScene(Physics::Instance().GetScene());
    }

    const std::vector<InterleavedGltfModel::Node>& GetAnimatedNodes() const
    {
        return rigidBody_->GetAnimatedNodes();
    }

    void DisableCollision()override
    {
        rigidBody_->DisableCollision();
    }

    virtual void OnUnregister()override
    {
        if (rigidBody_)
        {
            rigidBody_->Destroy();
            rigidBody_ = nullptr;
        }
    }

    MeshComponent* GetMeshComponent()const { return meshComponent_; }
private:
    std::shared_ptr<MultiRigidBodyComponent> rigidBody_;
    MeshComponent* meshComponent_ = nullptr;
};

// staticBatchinModel にのみ使用可能
class TriangleMeshCollisionComponent :public SceneComponent
{
public:
    TriangleMeshCollisionComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

    virtual void Initialize()override {}

    void Tick(float deltaTime)override {}

    // モデルから TriangleMesh を作成する
    void CreateConvexMeshFromModel(MeshComponent* meshComponent);

private:
    std::shared_ptr<TriangleMeshRigidBodyComponent> rigidBody_;
};
#endif  //SHAPE_COMPONENT_H