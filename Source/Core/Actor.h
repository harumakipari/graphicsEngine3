#ifndef ACTOR_H
#define ACTOR_H

// C++ �W�����C�u����
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

    //�����t���R���X�g���N�^
    Actor(std::string actorName) :actorName(actorName) 
    {
        OutputDebugStringA(("Actor constructor: ownedSceneComponents_ size=" + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA((", capacity=" + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());
    }

    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) noexcept = delete;
    Actor& operator=(Actor&&) noexcept = delete;

    std::string& GetName() { return actorName; }

    virtual void Initialize() {};

    virtual void Initialize(const Transform& transform) {}

    // Initialize �̌�ɌĂ΂��ׂ����� Transform���X�V����@�L���b�V�����Ă��邽�߈�t���[�����Transform�����Ă΂��\����h������
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

    //�X�V����
    virtual void Update(float deltaTime) {}

    // �I������
    virtual void Finalize() {}

    void SetRootComponent(std::shared_ptr<SceneComponent> root)
    {
        rootComponent_ = root;
    }

    std::shared_ptr<SceneComponent> GetRootComponent()
    {
        return rootComponent_;
    }


    //Actor �ɐV�����R���|�[�l���g���쐬���A���j�[�N�Ȗ��O�œo�^���A�e�q�֌W��ݒ肵�ĕԂ��֐�
    template <class T>
    std::shared_ptr<T> NewSceneComponent(const std::string& name, const std::string& parentName = "")
    {
        // �������g�� shared_ptr �ŊǗ�����Ă���O��ŁA�����n��
        std::shared_ptr<Actor> sharedThis = shared_from_this(); // Actor�� std::enable_shared_from_this �p�����K�v
        // Debug�`�F�b�N1: �������g�̊m�F
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


        // push_back������l�Ɋm�F
        OutputDebugStringA(("After push_back size: " + std::to_string(ownedSceneComponents_.size()) + "\n").c_str());
        OutputDebugStringA(("After push_back capacity: " + std::to_string(ownedSceneComponents_.capacity()) + "\n").c_str());

        // ����������
        //newComponent->Initialize();
        newComponent->OnRegister();

        return newComponent;
    }

    // Transform �s�v�� Component�p
    //template <class T>
    //std::shared_ptr<T> NewLogicComponent(const std::string& name)
    //{

    //    std::shared_ptr<Actor> sharedThis = shared_from_this();
    //    std::shared_ptr<T> newComponent = std::make_shared<T>(name, sharedThis);

    //    ownedLogicComponents_.push_back(newComponent);
    //    newComponent->OnRegister();

    //    return newComponent;
    //}

    // ���� Actor �� Component ��e�ɂ������Ƃ��p
    template <class T>
    std::shared_ptr<T> NewSceneComponentWithParent(const std::string& name, const std::shared_ptr<SceneComponent> explictParent)
    {
        // �������g�� shared_ptr �ŊǗ�����Ă���O��ŁA�����n��
        std::shared_ptr<Actor> sharedThis = shared_from_this(); // Actor�� std::enable_shared_from_this �p�����K�v
        std::shared_ptr<T> newComponent = std::make_shared<T>(name, sharedThis);

        if (explictParent)
        {
            newComponent->AttachTo(explictParent);
        }
        else
        {
            _ASSERT(" ���� Actor �� Component ��e�ɂ��悤�Ƃ��Ă��邯�� null �ł��B");
        }


        // ���L���X�g�ɒǉ�
        ownedSceneComponents_.push_back(newComponent);

        // ����������
        newComponent->OnRegister();

        return newComponent;
    }


    // ���O����component���Q�b�g����
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

    //// ���O����component���Q�b�g����
    //std::shared_ptr<Component> GetLogicComponentByName(const std::string& name)
    //{
    //    if (name.empty())
    //    {
    //        _ASSERT("GetLogicComponentByName �֐��̈����̖��O������܂���B");
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


    // ���O����component���폜����
    void DestroyComponentByName(const std::string& name)
    {
        if (name.empty())
        {
            _ASSERT(L"���̖��O�̃R���|�[�l���g�͑��݂��Ȃ����ߍ폜�ł��܂���B");
            return;
        }

        if (rootComponent_ && rootComponent_->name() == name)
        {
            // rootComponent�̍폜�͋֎~
            _ASSERT(L"rootComponent�͍폜�ł��܂���B");
            return;
        }

        // �L���b�V��������T��
        auto itNameSceneComp = nameToSceneComponent_.find(name);
        if (itNameSceneComp != nameToSceneComponent_.end())
        {
            nameToSceneComponent_.erase(itNameSceneComp);
        }

        // �L���b�V��������T��
        //auto itNameComp = nameToLogicComponent_.find(name);
        //if (itNameComp != nameToLogicComponent_.end())
        //{
        //    // �L���b�V������폜�Ώۂ��폜�iDestroy�Ăяo����ownedComponents_�ōs���z��j
        //    nameToLogicComponent_.erase(itNameComp);
        //}


        auto it = std::remove_if(ownedSceneComponents_.begin(), ownedSceneComponents_.end(),
            //[&](const std::shared_ptr<SceneComponent>& comp)
            [&](const std::shared_ptr<Component>& comp)
            {
                if (comp->name() == name) {
                    comp->Destroy(); // ��Œ�`���� Destroy �Ă�
                    return true;     // erase �Ώۂɂ���
                }
                return false;
            });

        ownedSceneComponents_.erase(it, ownedSceneComponents_.end());
        nameToSceneComponent_.erase(name);
    }

    template<typename T>
    T* GetComponent()
    {
        // SceneComponent ����T��
        //for (const std::shared_ptr<SceneComponent>& compent : ownedSceneComponents_)
        for (const std::shared_ptr<Component>& compent : ownedSceneComponents_)
        {
            T* casted = dynamic_cast<T*>(compent.get());
            if (casted != nullptr)
            {
                return casted;
            }
        }

        // LogicComponent ����T��
        //for (const std::shared_ptr<Component>& compent : ownedLogicComponents_)
        //{
        //    T* casted = dynamic_cast<T*>(compent.get());
        //    if (casted != nullptr)
        //    {
        //        return casted;
        //    }
        //}

        _ASSERT(L"Actor �� GetComponent �� nullptr ��Ԃ��Ă��܂��B");
        return nullptr;
    }

    template<class T>       //������vector������ƁA�R���|�[�l���g��
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
            // isValid �`�F�b�N�{�b�N�X��ǉ�
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

    // �p���������T�u�N���X�̐�pGUI
    virtual void DrawImGuiDetails() {};

    // �ʒu���擾����֐�
    DirectX::XMFLOAT3 GetPosition() const
    {
        if (rootComponent_)
        {
            return rootComponent_->GetRelativeLocation();
        }
        return { 0.0f,0.0f,3.0f };
    }

    // �ʒu��ݒ肷��֐�
    void SetPosition(const DirectX::XMFLOAT3& position)
    {
        this->rootComponent_->SetRelativeLocationDirect(position);
        PostInitialize();
    }

    // �N�H�[�^�j�I�����擾����֐�
    const DirectX::XMFLOAT4& GetQuaternionRotation() const { return rootComponent_->GetRelativeRotation(); }

    // �N�H�[�^�j�I����ݒ肷��֐�
    void SetQuaternionRotation(const DirectX::XMFLOAT4& rotation)
    {
        // �����I�Ɍ�����ǉ�
        _ASSERT_EXPR(MathHelper::IsValidQuaternion(rotation), L"SetQuaternionRotation: Invalid quaternion");
        this->rootComponent_->SetRelativeRotationDirect(rotation);
    }

    // �p�x���擾����֐�
    DirectX::XMFLOAT3 GetEulerRotation() const
    {
        return rootComponent_->GetRelativeEulerRotation();
    }

    // �p�x��ݒ肷��֐�
    void SetEulerRotation(const DirectX::XMFLOAT3& eulerRotation)
    {
        this->rootComponent_->SetRelativeEulerRotationDirect(eulerRotation);
    }

    // �X�P�[�����擾����֐�
    DirectX::XMFLOAT3 GetScale() const { return rootComponent_->GetRelativeScale(); }

    // �X�P�[����ݒ肷��֐�
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

    // ������ĂԂƎ��̃t���[���ł��� actor �͍폜�����
    virtual void SetValid(bool isValid)
    {
        this->isValid = isValid;

        char buf[256];
        sprintf_s(buf, "SetValid called: this=%p, isValid=%d\n", this, isValid);
        OutputDebugStringA(buf);
    }

    // ���̃t���[���ł��� actor �� Destroy �֐����Ă΂��
    void SetPendingDestroy()
    {
        isPendingDestroy = true;
    }

    bool GetIsValid()
    {
        return this->isValid;
    }

    //Character�Ŏg�����߂�AnimationIndex��ǉ�
    virtual size_t GetAnimationIndex() const { return 0; }  //�f�t�H���g��0��Ԃ�

    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    virtual void NotifyHit(/*std::shared_ptr<Component> selfComp, std::shared_ptr<Component> otherComp,*//* std::shared_ptr<Actor> otherActor*/Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) {}
    //�@collisionComponent�@�� Dynamic �̕��Ɠ����������ɒʂ�
    virtual void NotifyHit(CollisionComponent* selfComp, CollisionComponent* otherComp, Actor* otherActor, const DirectX::XMFLOAT3& hitPos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& impulse) {}

#if 0
    //���f���̃W���C���g��position���擾����֐�
    const DirectX::XMFLOAT3& GetJointPosition(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedJointPosition = modelComponent.GetModel()->JointPosition(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedJointPosition;
    }

    //���f���̃W���C���g��X�������̃x�N�g���֐�
    const DirectX::XMFLOAT3& GetJointRightVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT3 right{ 0,0,0 };
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //right = modelComponent.GetModel()->GetJointRightVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return right;
    }

    //���f���̃W���C���g��Y�������̃x�N�g���֐�
    const DirectX::XMFLOAT3& GetJointUpVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT3 up{ 0,0,0 };
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //up = modelComponent.GetModel()->GetJointUpVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return up;
    }

    //���f���̃W���C���g��Z�������̃x�N�g���֐�
    const DirectX::XMFLOAT3& GetJointForwardVector(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedForward = modelComponent.GetModel()->GetJointForwardVector(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedForward;
    }

    //���f���̃W���C���g�̃��[���h�ϊ��s���Ԃ��֐�
    const DirectX::XMFLOAT4X4& GetJointTransform(size_t nodeIndex)
    {
        //DirectX::XMFLOAT4X4 world = CreateWorldMatrix(position, scale, rotation);
        //catchedJointTransform = modelComponent.GetModel()->GetJointTransform(nodeIndex, modelComponent.GetCurrentNodes(), world);
        //return catchedJointTransform;
    }

#endif
    using HitCallBack = std::function<void(std::pair<CollisionComponent*, CollisionComponent*>)>;

    // �����������ɒʂ�֐�       // hitShapes.first�����g hitShapes.second������
    void BroadcastHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)
    {
        for (auto& callback : hitCallbacks_)
        {
            callback(hitShapes);
        }
    }

    // �����������ɒʂ�֐� �L�l�}�e�B�b�N
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
        //// ���łɖ����Ȃ��d����������
        //if (!isValid)
        //{
        //    return;
        //}

        //// ���݃t���O�𖳌��ɂ���
        //isValid = false;

        // �q�b�g�R�[���o�b�N���N���A
        RemoveAllHitCallBacks();

        // �S�R���|�[�l���g�̔j��
        for (auto& comp : ownedSceneComponents_)
        {
            if (comp)
            {
                comp->Destroy();          // PhysX����̏����ȂǓ����I�ȃN���[���A�b�v
                comp->OnUnregister();     // Scene�Ȃǂ���̓o�^����
            }
        }

        // �S�R���|�[�l���g�̔j��
        //for (auto& comp : ownedLogicComponents_)
        //{
        //    if (comp)
        //    {
        //        comp->Destroy();          // PhysX����̏����ȂǓ����I�ȃN���[���A�b�v
        //        comp->OnUnregister();     // Scene�Ȃǂ���̓o�^����
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

    // ��ɍ폜����R���|�[�l���g���X�g�ɒǉ�
    void ScheduleDestroyComponentByName(const std::string& name)
    {
        pendingDestroyComponentNames.push_back(name);
    }

    // physx �̌�ł�肽������
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

    //actor�̖��O
    std::string actorName;

    // ���[���h�ϊ��s��
    DirectX::XMFLOAT4X4 worldTransform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

protected:
    Scene* ownerScene_ = nullptr;   // �����������Ă���Scene

public:
    void SetOwnerScene(Scene* scene) { ownerScene_ = scene; }
    Scene* GetOwnerScene() const { return ownerScene_; }

public:
    //�A�N�^�[���L�����ǂ���
    bool isActive = true;

    // �A�N�^�[�����݂��Ă��邩�ǂ���
    bool isValid = true;

    // �A�N�^�[�̍폜�\��
    bool isPendingDestroy = false;

public:
    // rootComponent (Transform) �n
    std::shared_ptr<SceneComponent> rootComponent_;

    // Component�iTransform�s�v�n�j
    //std::vector<std::shared_ptr<Component>> ownedLogicComponents_;

    // SceneComponent (Transform) �n
    std::vector<std::shared_ptr<Component>> ownedSceneComponents_;
    //std::vector<std::shared_ptr<SceneComponent>> ownedSceneComponents_;

    // ���O�ƃR���|�[�l���g���L���b�V�����Ă���
    //std::unordered_map<std::string, std::shared_ptr<SceneComponent>> nameToSceneComponent_;
    std::unordered_map<std::string, std::shared_ptr<Component>> nameToSceneComponent_;

    // ���O�ƃR���|�[�l���g���L���b�V�����Ă���
    //std::unordered_map<std::string, std::shared_ptr<Component>> nameToLogicComponent_;

    // �폜�\��p���X�g�@
    // physx �� simulate �r���� pxShape ���t���� shapeComponent ���폜����̂�h������
    std::vector<std::string> pendingDestroyComponentNames;

protected:
    // Physx�ɐ������ʒu�𑗂邽�߂̏������O�� position �̕ۊǏꏊ
    DirectX::XMFLOAT3 tempPosition = { 0.0f,0.0f,0.0f };

    // �I�C���[�p���g�����ǂ���
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