#ifndef BEAM_H
#define BEAM_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Effect/EffectComponent.h"

// これを enemy 側にも足す
// boxComponent->SetResponseToLayer(CollisionLayer::Projectile, CollisionComponent::CollisionResponse::Trigger);

class Beam :public Actor
{
public:
    //引数付きコンストラクタ
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
        // 描画用コンポーネントを追加
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Models/Beam/beam.gltf");
        //skeltalMeshComponent->model->isModelInMeters = false;
        SetPosition(tempPosition);
        //skeltalMeshComponent->SetIsVisible(false);
        float t = std::clamp(itemPower / itemMaxPower, 0.0f, 1.0f);
        float s = std::lerp(1.0f, 3.5f, t);
        SetScale(DirectX::XMFLOAT3(s, s, s));
        // 当たり判定球を追加
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

        // エフェクトコンポーネントを追加 生成した瞬間からエフェクト出すから
        effectBeamComponent = this->NewSceneComponent<class EffectComponent>("effectBeamComponet", "skeltalComponent");
        effectBeamComponent->SetEffectType(EffectComponent::EffectType::Beam);
        effectBeamComponent->Activate();

        // 
        effectSparkComponent = this->NewSceneComponent<class EffectComponent>("effectSparkComponet", "skeltalComponent");
        effectSparkComponent->SetEffectType(EffectComponent::EffectType::Spark);
    }



    //更新処理
    void Update(float deltaTime) override
    {
        // ビームの position 
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
    // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (float)
    void SetItemPower(float power) { this->itemPower = power; }
    // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (float)
    float GetItemPower() { return this->itemPower; }
    // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (int)
    void SetItemCount(int count) { this->itemCount = count; }
    // ビームが発射した時にプレイヤーから伝達されるアイテムの個数 (int)
    int GetItemCount() { return this->itemCount; }
    // ビームのパワーの最大値　プレイヤーから伝達される(float)
    void SetItemMaxPower(float maxPower) { this->itemMaxPower = maxPower; }
    //　collisionComponent　が Dynamic の物と当たった時に通る
    void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*/ /*std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse)override;

    // キネマティック同士の当たり判定を検知する
    void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)override;

    void SetDirection(const DirectX::XMFLOAT3& dir)
    {
        direction = dir;
    }
private:
    float tempMass = 0.0f;
    // ビームの保持アイテム個数
    int itemCount = 0;
    // ビームの保持アイテム個数を power に換算したもの
    float itemPower = 0.0f;

    // ビームの最大値
    float itemMaxPower = 0.0f;

    //　ビームが飛ぶ方向
    DirectX::XMFLOAT3 direction = { 0.0f,0.0f,0.0f };

    // 何かに当たった時の瞬間のpositionを保存するために必要
    bool onceSetPosition = false;

    std::unordered_set<Actor*> hitActors_;
};

#endif // BEAM_H
