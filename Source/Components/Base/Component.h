#ifndef COMPONENT_H
#define COMPONENT_H
// 1. �������g�̃w�b�_����ԏ�ɗ���
 
// 2. C �W�����C�u�����i����΁j

// C++ �W�����C�u����
#include <algorithm>
#include <memory>
#include <optional>
#include <string>

// �����C�u����
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>


#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// �v���W�F�N�g�̑��̃w�b�_
#include "Graphics/Core/Graphics.h"
#include "Math/MathHelper.h"
#include "Components/Transform/Transform.h"
#include "Engine/Camera/CameraManager.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsHelper.h"

class Actor;


// �����{�f�B���ړ��i�e���|�[�g�j��������@���w��
enum class TeleportType :int
{
    None, //�e���|�[�g�Ȃ��B�ʏ�̈ړ��i���x�ƏՓˌv�Z����j
    TeleportPhysics, // �u�Ԉړ����邪���x�͈ێ��B�Փ˔���Ȃ�
    ResetPhysics, // �e���|�[�g��ɕ�����Ԃ����S���Z�b�g
};

// Transform ���X�V����ۂ̃t���O
enum class UpdateTransformFlags :int
{
    None = 0x0,  // �ʏ�ʂ�X�V
    SkipPhysicsUpdate = 0x1,    // �����G���W���ɂ͒ʒm���Ȃ�
    PropagateFromParent = 0x2,  // �e����`�d���Ă����X�V���Ǝ���
    OnlyUpdateIfUsingSocket = 0x4,  // Socket�i�\�P�b�g�j���g���Ă���q�R���|�[�l���g�������X�V�Ώۂɂ���
};
// UpdateTransformFlags���r�b�g���Z�ł���悤�ɂ���I�[�o�[���[�h�̒�`
inline UpdateTransformFlags operator|(UpdateTransformFlags lhs, UpdateTransformFlags rhs)
{
    // underlying_type_t -> enum class �𐮐��Ƃ��Ĉ������̃L���X�g
    using T = std::underlying_type_t<UpdateTransformFlags>;
    return static_cast<UpdateTransformFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
inline UpdateTransformFlags operator&(UpdateTransformFlags lhs, UpdateTransformFlags rhs)
{
    // underlying_type_t -> enum class �𐮐��Ƃ��Ĉ������̃L���X�g
    using T = std::underlying_type_t<UpdateTransformFlags>;
    return static_cast<UpdateTransformFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
inline UpdateTransformFlags operator^(UpdateTransformFlags lhs, UpdateTransformFlags rhs)
{
    // underlying_type_t -> enum class �𐮐��Ƃ��Ĉ������̃L���X�g
    using T = std::underlying_type_t<UpdateTransformFlags>;
    return static_cast<UpdateTransformFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}
inline UpdateTransformFlags operator~(UpdateTransformFlags flag)
{
    // underlying_type_t -> enum class �𐮐��Ƃ��Ĉ������̃L���X�g
    using T = std::underlying_type_t<UpdateTransformFlags>;
    return static_cast<UpdateTransformFlags>(~static_cast<T>(flag));
}


//	�R���|�[�l���g�̃x�[�X�N���X
class Component
{
public:
    Component(const std::string& name,std::shared_ptr<Actor> owner) :name_(name), owner_(owner), active_(true) {};
    virtual ~Component() = default;

    Component(const Component& rhs) = delete;
    Component& operator=(const Component& rhs) = delete;
    Component(Component&&) noexcept = delete;
    Component& operator=(Component&&) noexcept = delete;

    virtual void Initialize() = 0;

    virtual void Tick(float deltaTime) = 0;

    virtual void OnRegister() {} // �h���N���X�� override ���ēo�^����������
    virtual void OnUnregister() {} // �h���N���X�� override ���ĉ�������������

    virtual void DrawImGuiInspector(){}

    const std::string& name() const
    {
        return name_;
    }

    Actor* GetOwner() { return owner_.lock().get(); }

    std::shared_ptr<Actor> GetActor() { return owner_.lock(); }

    void SetActive(bool active) { this->active_ = active; }

    bool GetActive() { return active_; }

    virtual void UpdateComponentToWorld(UpdateTransformFlags update_transform_flags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None) = 0;

    virtual void Destroy()
    {
        OnUnregister();
        SetActive(false);
    }
protected:
    //Actor* owner_ = nullptr;
    std::weak_ptr<Actor> owner_;
    std::string name_;
    bool active_;
    bool initialized_ = false; // ������̏���������̂�h���̂Ɏg���B�B
};




#endif //COMPONENT_H