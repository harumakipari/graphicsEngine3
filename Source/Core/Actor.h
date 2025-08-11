#ifndef ACTOR_H
#define ACTOR_H

// C++ 標準ライブラリ
#include <string>
#include <DirectXMath.h>
#include <memory>
#include <PxPhysics.h>
#include<assert.h>

#include "Components/Base/Component.h"
#include "Components/Base/SceneComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Transform/Transform.h"
#include "Engine/Utility/Win32Utils.h"
#include "Engine/Debug/Assert.h"
#include "Math/MathHelper.h"

class Scene;

class Actor :public std::enable_shared_from_this <Actor>
{
public:
    Actor()
    {
        OutputDebugStringA(("Actor constructor: ownedSceneComponents_ size=" + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA((", capacity=" + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());
    }
    virtual ~Actor() = default;

    //引数付きコンストラクタ
    Actor(std::string actorName) :actorName(actorName) 
    {
        OutputDebugStringA(("Actor constructor: ownedSceneComponents_ size=" + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA((", capacity=" + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());
    }

    //コピーコンストラクタとコピー代入演算子を禁止にする
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) noexcept = delete;
    Actor& operator=(Actor&&) noexcept = delete;

    std::string& GetName() { return actorName; }

    virtual void Initialize() {};

    virtual void Initialize(const Transform& transform) {}

    // Initialize の後に呼ばれるべき処理 Transformを更新する　キャッシュしているため一フレーム後のTransformをが呼ばれる可能性を防ぐため
    virtual void PostInitialize()
    {
        if (rootComponent_)
        {
            rootComponent_->UpdateComponentToWorld();
        }

        for (const auto& comp : ownedSceneComponents_)
        {
            comp->UpdateComponentToWorld();
        }
    }

    //更新処理
    virtual void Update(float deltaTime) {}

    // 終了処理
    virtual void Finalize() {}

    void SetRootComponent(std::shared_ptr<SceneComponent> root)
    {
        rootComponent_ = root;
    }

    std::shared_ptr<SceneComponent> GetRootComponent()
    {
        return rootComponent_;
    }


    //Actor に新しいコンポーネントを作成し、ユニークな名前で登録し、親子関係を設定して返す関数
    template <class T>
    std::shared_ptr<T> NewSceneComponent(const std::string& name, const std::string& parentName = "")
    {
        // 自分自身が shared_ptr で管理されている前提で、それを渡す
        std::shared_ptr<Actor> sharedThis = shared_from_this(); // Actorは std::enable_shared_from_this 継承が必要
        // Debugチェック1: 自分自身の確認
        _ASSERT_EXPR(sharedThis != nullptr , "shared_from_this() returned nullptr!");

        std::shared_ptr<T> newComponent = std::make_shared<T>(name, sharedThis);
        _ASSERT_EXPR(newComponent != nullptr , "Failed to create new SceneComponent!");
        //std::shared_ptr<T> newComponent = std::make_shared<T>(name, this);

        if constexpr ((std::is_base_of<SceneComponent, T>::value))
        {
            std::shared_ptr<SceneComponent> sceneComponent = std::dynamic_pointer_cast<SceneComponent>(newComponent);
            _ASSERT_EXPR(sceneComponent != nullptr,"Dynamic cast to SceneComponent failed!");
            if (parentName.empty())
            {
                if (rootComponent_)
                {
                    sceneComponent->AttachTo(rootComponent_);
                }
                else
                {
                    SetRootComponent(sceneComponent);
                }
            }
            else
            {
                std::shared_ptr<SceneComponent> parent = std::dynamic_pointer_cast<SceneComponent>(GetSceneComponentByName(parentName));
                sceneComponent->AttachTo(parent);
            }
        }

        _ASSERT_EXPR(reinterpret_cast<void*>(&ownedSceneComponents_) != nullptr, "ownedSceneComponents_ is nullptr!");

        //_ASSERT_EXPR(newComponent.use_count() >= 2, "newComponent use_count is invalid!"); 

        OutputDebugStringA(("Before push_back size: " + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA(("Before push_back capacity: " + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());

        //ownedSceneComponents_.push_back(newComponent);
        ownedSceneComponents_.push_back(std::static_pointer_cast<Component>(newComponent));


        // push_back後も同様に確認
        OutputDebugStringA(("After push_back size: " + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA(("After push_back capacity: " + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());

        // 初期化する
        //newComponent->Initialize();
        newComponent->OnRegister();

        return newComponent;
    }

    // Transform 不要の Component用
    //template <class T>
    //std::shared_ptr<T> NewLogicComponent(const std::string& name)
    //{

    //    std::shared_ptr<Actor> sharedThis = shared_from_this();
    //    std::shared_ptr<T> newComponent = std::make_shared<T>(name, sharedThis);

    //    ownedLogicComponents_.push_back(newComponent);
    //    newComponent->OnRegister();

    //    return newComponent;
    //}

    // 他の Actor の Component を親にしたいとき用
    template <class T>
    std::shared_ptr<T> NewSceneComponentWithParent(const std::string& name, const std::shared_ptr<SceneComponent> explictParent)
    {
        // 自分自身が shared_ptr で管理されている前提で、それを渡す
        std::shared_ptr<Actor> sharedThis = shared_from_this(); // Actorは std::enable_shared_from_this 継承が必要
        std::shared_ptr<T> newComponent = std::make_shared<T>(name, sharedThis);

        if (explictParent)
        {
            newComponent->AttachTo(explictParent);
        }
        else
        {
            _ASSERT(" 他の Actor の Component を親にしようとしているけど null です。");
        }


        // 所有リストに追加
        ownedSceneComponents_.push_back(newComponent);

        // 初期化する
        newComponent->OnRegister();

        return newComponent;
    }


    // 名前からcomponentをゲットする
    //template <class T>
    //std::shared_ptr<T> GetSceneComponentByName(const std::string& name)
    //std::shared_ptr<SceneComponent> GetSceneComponentByName(const std::string& name)
    std::shared_ptr<Component> GetSceneComponentByName(const std::string& name)
    {
        if (name.empty())
        {
            return rootComponent_;
        }

        decltype(nameToSceneComponent_)::const_iterator nameToComponent = nameToSceneComponent_.find(name);
        if (nameToComponent != nameToSceneComponent_.end())
        {
            return nameToComponent->second;
        }

        decltype(ownedSceneComponents_)::const_iterator component = std::find(ownedSceneComponents_.begin(), ownedSceneComponents_.end(), name);
        if (component != ownedSceneComponents_.end())
        {
            nameToSceneComponent_.emplace(name, *component);
            return *component;
        }


        return nullptr;
    }

    //// 名前からcomponentをゲットする
    //std::shared_ptr<Component> GetLogicComponentByName(const std::string& name)
    //{
    //    if (name.empty())
    //    {
    //        _ASSERT("GetLogicComponentByName 関数の引数の名前がありません。");
    //    }


    //    decltype(nameToLogicComponent_)::const_iterator nameToComponent = nameToLogicComponent_.find(name);
    //    if (nameToComponent != nameToLogicComponent_.end())
    //    {
    //        return nameToComponent->second;
    //    }

    //    decltype(ownedLogicComponents_)::const_iterator component = std::find(ownedLogicComponents_.begin(), ownedLogicComponents_.end(), name);
    //    if (component != ownedLogicComponents_.end())
    //    {
    //        nameToLogicComponent_.emplace(name, *component);
    //        return *component;
    //    }

    //    return nullptr;
    //}


    // 名前からcomponentを削除する
    void DestroyComponentByName(const std::string& name)
    {
        if (name.empty())
        {
            _ASSERT(L"この名前のコンポーネントは存在しないため削除できません。");
            return;
        }

        if (rootComponent_ && rootComponent_->name() == name)
        {
            // rootComponentの削除は禁止
            _ASSERT(L"rootComponentは削除できません。");
            return;
        }

        // キャッシュからも探す
        auto itNameSceneComp = nameToSceneComponent_.find(name);
        if (itNameSceneComp != nameToSceneComponent_.end())
        {
            nameToSceneComponent_.erase(itNameSceneComp);
        }

        // キャッシュからも探す
        //auto itNameComp = nameToLogicComponent_.find(name);
        //if (itNameComp != nameToLogicComponent_.end())
        //{
        //    // キャッシュから削除対象を削除（Destroy呼び出しはownedComponents_で行う想定）
        //    nameToLogicComponent_.erase(itNameComp);
        //}


        auto it = std::remove_if(ownedSceneComponents_.begin(), ownedSceneComponents_.end(),
            //[&](const std::shared_ptr<SceneComponent>& comp)
            [&](const std::shared_ptr<Component>& comp)
            {
                if (comp->name() == name) {
                    comp->Destroy(); // 上で定義した Destroy 呼ぶ
                    return true;     // erase 対象にする
                }
                return false;
            });

        ownedSceneComponents_.erase(it, ownedSceneComponents_.end());
        nameToSceneComponent_.erase(name);
    }

    template<typename T>
    T* GetComponent()
    {
        // SceneComponent から探す
        //for (const std::shared_ptr<SceneComponent>& compent : ownedSceneComponents_)
        for (const std::shared_ptr<Component>& compent : ownedSceneComponents_)
        {
            T* casted = dynamic_cast<T*>(compent.get());
            if (casted != nullptr)
            {
                return casted;
            }
        }

        // LogicComponent から探す
        //for (const std::shared_ptr<Component>& compent : ownedLogicComponents_)
        //{
        //    T* casted = dynamic_cast<T*>(compent.get());
        //    if (casted != nullptr)
        //    {
        //        return casted;
        //    }
        //}

        _ASSERT(L"Actor の GetComponent が nullptr を返しています。");
        return nullptr;
    }

    template<class T>       //引数にvectorを入れると、コンポーネントを
    void GetComponents(std::vector<T*>& components)
    {
        components.clear();
        for (auto component : ownedSceneComponents_)
        {
            T* downCastComponent = dynamic_cast<T*>(component.get());
            if (downCastComponent)
            {
                components.push_back(downCastComponent);
            }
        }

        //for (auto component : ownedLogicComponents_)
        //{
        //    T* downCastComponent = dynamic_cast<T*>(component.get());
        //    if (downCastComponent)
        //    {
        //        components.push_back(downCastComponent);
        //    }
        //}
    }


    const DirectX::XMFLOAT4X4& GetWorldTransform()
    {
        //DirectX::XMMATRIX M = rootComponent_ ? rootComponent_->componentToWorld_.ToMatrix() : DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX M = rootComponent_ ? rootComponent_->GetFinalWorldTransform().ToMatrix() : DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&worldTransform, M);
        return worldTransform;
    }


    void DrawImGuiInspector()
    {
#ifdef USE_IMGUI

        if (ImGui::TreeNode(actorName.c_str()))
        {
            // isValid チェックボックスを追加
            ImGui::Checkbox("isValid", &isActive);

            if (rootComponent_)
            {
                ImGui::Text("RootComponent: %s", typeid(*rootComponent_).name());
                rootComponent_->DrawImGuiInspector();
            }

            for (auto& comp : ownedSceneComponents_)
            {
                if (comp != rootComponent_) {
                    ImGui::Text("%s", typeid(*comp).name());
                    comp->DrawImGuiInspector();
                }
            }

            //for (auto& comp : ownedLogicComponents_)
            //{
            //    if (comp != rootComponent_) {
            //        ImGui::Text("%s", typeid(*comp).name());
            //        comp->DrawImGuiInspector();
            //    }
            //}

            DrawImGuiDetails();

            ImGui::TreePop();
        }
#endif
    }

    // 継承押したサブクラスの専用GUI
    virtual void DrawImGuiDetails() {};

    // 位置を取得する関数
    DirectX::XMFLOAT3 GetPosition() const
    {
        if (rootComponent_)
        {
            return rootComponent_->GetRelativeLocation();
        }
        return { 0.0f,0.0f,3.0f };
    }

    // 位置を設定する関数
    void SetPosition(const DirectX::XMFLOAT3& position)
    {
        this->rootComponent_->SetRelativeLocationDirect(position);
        PostInitialize();
    }

    // クォータニオンを取得する関数
    const DirectX::XMFLOAT4& GetQuaternionRotation() const { return rootComponent_->GetRelativeRotation(); }

    // クォータニオンを設定する関数
    void SetQuaternionRotation(const DirectX::XMFLOAT4& rotation)
    {
        // 明示的に検査を追加
        _ASSERT_EXPR(MathHelper::IsValidQuaternion(rotation), L"SetQuaternionRotation: Invalid quaternion");
        this->rootComponent_->SetRelativeRotationDirect(rotation);
    }

    // 角度を取得する関数
    DirectX::XMFLOAT3 GetEulerRotation() const
    {
        return rootComponent_->GetRelativeEulerRotation();
    }

    // 角度を設定する関数
    void SetEulerRotation(const DirectX::XMFLOAT3& eulerRotation)
    {
        this->rootComponent_->SetRelativeEulerRotationDirect(eulerRotation);
    }

    // スケールを取得する関数
    DirectX::XMFLOAT3 GetScale() const { return rootComponent_->GetRelativeScale(); }

    // スケールを設定する関数
    void SetScale(const DirectX::XMFLOAT3& scale)
    {
        this->rootComponent_->SetRelativeScaleDirect(scale);
    }

    void SetActive(bool isActive)
    {
        this->isActive = isActive;
    }

    bool GetActive()
    {
        return isActive;
    }

    // これを呼ぶと次のフレームでこの actor は削除される
    virtual void SetValid(bool isValid)
    {
        this->isValid = isValid;

        char buf[256];
        sprintf_s(buf, "SetValid called: this=%p, isValid=%d\n", this, isValid);
        OutputDebugStringA(buf);
    }

    // 次のフレームでこの actor の Destroy 関数が呼ばれる
    void SetPendingDestroy()
    {
        isPendingDestroy = true;
    }

    bool GetIsValid()
    {
        return this->isValid;
    }

    //Characterで使うためにAnimationIndexを追加
    virtual size_t GetAnimationIndex() const { return 0; }  //デフォルトで0を返す

    //　collisionComponent　が Dynamic の物と当たった時に通る
    virtual void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*//* std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) {}
    //　collisionComponent　が Dynamic の物と当たった時に通る
    virtual void NotifyHit(CollisionComponent* selfComp, CollisionComponent* otherComp, Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) {}

#if 0
    //モデルのジョイントのpositionを取得する関数
    const DirectX::XMFLOAT3& GetJointPosition(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedJointPosition = modelComponent.GetModel()->JointPosition(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedJointPosition;
    }

    //モデルのジョイントのX軸方向のベクトル関数
    const DirectX::XMFLOAT3& GetJointRightVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT3 right{ 0,0,0 };
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //right = modelComponent.GetModel()->GetJointRightVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return right;
    }

    //モデルのジョイントのY軸方向のベクトル関数
    const DirectX::XMFLOAT3& GetJointUpVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT3 up{ 0,0,0 };
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //up = modelComponent.GetModel()->GetJointUpVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return up;
    }

    //モデルのジョイントのZ軸方向のベクトル関数
    const DirectX::XMFLOAT3& GetJointForwardVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedForward = modelComponent.GetModel()->GetJointForwardVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedForward;
    }

    //モデルのジョイントのワールド変換行列を返す関数
    const DirectX::XMFLOAT4X4& GetJointTransform(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedJointTransform = modelComponent.GetModel()->GetJointTransform(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedJointTransform;
    }

#endif
    using HitCallBack = std::function<void(std::pair<CollisionComponent*, CollisionComponent*>)>;

    // 当たった時に通る関数       // hitShapes.firstが自身 hitShapes.secondが相手
    void BroadcastHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)
    {
        for (auto& callback : hitCallbacks_)
        {
            callback(hitShapes);
        }
    }

    // 当たった時に通る関数 キネマティック
    virtual void OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
    {

    }

    void AddHitCallback(HitCallBack callback)
    {
        hitCallbacks_.push_back(callback);
    }

    void RemoveAllHitCallBacks()
    {
        hitCallbacks_.clear();
    }

    void Destroy()
    {
        //// すでに無効なら二重解放を避ける
        //if (!isValid)
        //{
        //    return;
        //}

        //// 存在フラグを無効にする
        //isValid = false;

        // ヒットコールバックをクリア
        RemoveAllHitCallBacks();

        // 全コンポーネントの破棄
        for (auto& comp : ownedSceneComponents_)
        {
            if (comp)
            {
                comp->Destroy();          // PhysXからの除去など内部的なクリーンアップ
                comp->OnUnregister();     // Sceneなどからの登録解除
            }
        }

        // 全コンポーネントの破棄
        //for (auto& comp : ownedLogicComponents_)
        //{
        //    if (comp)
        //    {
        //        comp->Destroy();          // PhysXからの除去など内部的なクリーンアップ
        //        comp->OnUnregister();     // Sceneなどからの登録解除
        //    }
        //}

        Finalize();

        ownedSceneComponents_.clear();
        //ownedLogicComponents_.clear();
        nameToSceneComponent_.clear();
        //nameToLogicComponent_.clear();
        rootComponent_ = nullptr;

        isValid = false;
    }

    // 後に削除するコンポーネントリストに追加
    void ScheduleDestroyComponentByName(const std::string& name)
    {
        pendingDestroyComponentNames.push_back(name);
    }

    // physx の後でやりたい処理
    void PostDestroyComponents()
    {
        for (const auto& name : pendingDestroyComponentNames)
        {
            DestroyComponentByName(name);
        }
        pendingDestroyComponentNames.clear();
    }

    void SetTempPosition(DirectX::XMFLOAT3 pos) { this->tempPosition = pos; }

private:

    std::vector<HitCallBack> hitCallbacks_;

    //actorの名前
    std::string actorName;

    // ワールド変換行列
    DirectX::XMFLOAT4X4 worldTransform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

protected:
    Scene* ownerScene_ = nullptr;   // 自分が属しているScene

public:
    void SetOwnerScene(Scene* scene) { ownerScene_ = scene; }
    Scene* GetOwnerScene() const { return ownerScene_; }

public:
    //アクターが有効かどうか
    bool isActive = true;

    // アクターが存在しているかどうか
    bool isValid = true;

    // アクターの削除予約
    bool isPendingDestroy = false;

public:
    // rootComponent (Transform) 系
    std::shared_ptr<SceneComponent> rootComponent_;

    // Component（Transform不要系）
    //std::vector<std::shared_ptr<Component>> ownedLogicComponents_;

    // SceneComponent (Transform) 系
    std::vector<std::shared_ptr<Component>> ownedSceneComponents_;
    //std::vector<std::shared_ptr<SceneComponent>> ownedSceneComponents_;

    // 名前とコンポーネントをキャッシュしておく
    //std::unordered_map<std::string, std::shared_ptr<SceneComponent>> nameToSceneComponent_;
    std::unordered_map<std::string, std::shared_ptr<Component>> nameToSceneComponent_;

    // 名前とコンポーネントをキャッシュしておく
    //std::unordered_map<std::string, std::shared_ptr<Component>> nameToLogicComponent_;

    // 削除予約用リスト　
    // physx の simulate 途中に pxShape が付いた shapeComponent を削除するのを防ぐため
    std::vector<std::string> pendingDestroyComponentNames;

protected:
    // Physxに正しい位置を送るための初期か前の position の保管場所
    DirectX::XMFLOAT3 tempPosition = { 0.0f,0.0f,0.0f };

    // オイラー角を使うかどうか
    DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
};

static inline bool operator==(const std::shared_ptr<Actor>& actor, const std::string& name)
{
    return actor->GetName() == name;
}

static inline bool operator==(const std::shared_ptr<SceneComponent>& component, const std::string& name)
{
    return component->name() == name;
}

static inline bool operator==(const std::shared_ptr<Component>& component, const std::string& name)
{
    return component->name() == name;
}
#endif //ACTOR_H