#ifndef SCENE_COMPONENT_H
#define SCENE_COMPONENT_H

// C++ �W�����C�u����
#include <memory>
#include <string>
#include <vector>

// �����C�u����
#include <DirectXMath.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// �v���W�F�N�g�̑��̃w�b�_
#include "Component.h"
#include "Components/Transform/Transform.h"


class Actor;


class SceneComponent :public Component, public std::enable_shared_from_this<SceneComponent>
{
public:
    SceneComponent(const std::string& name,std::shared_ptr<Actor> owner) :Component(name, owner)
    {
    }

    virtual ~SceneComponent() {}

    // �����o���ꂽ Transform ��ۑ�
    void SetPhysicalTransform(const Transform& t) 
    {
        physicalTransform_ = t; 
        hasPhysicalCorrection_ = true;
    }

    // Tick �ōŏITransform���擾����Ƃ��͂�����g��
    Transform GetFinalWorldTransform() const
    {
        if (hasPhysicalCorrection_)
            return physicalTransform_;
        return componentToWorld_;
    }

    void ClearPhysicalCorrection()
    {
        hasPhysicalCorrection_ = false;
    }

    // ��������
    virtual void Destroy() override;

    // ���������ɍ��邩�瑦�� Transform �X�V����
    void UpdateTransformImmediate();
private:
    // SceneComponent �� Transform �����N���X�ɒǉ�
    DirectX::XMFLOAT3 inspectorEuler_ = { 0,0,0 };
    bool inspectorEulerInitialized_ = false;

    // �Փˉ����o���œ��� Transform ��ۑ�����p�̕ϐ�
    Transform physicalTransform_;

    // ���̃R���|�[�l���g�̃��[���h��ԏ�ł�Transform
    // final Transform_�@�e�q�֌W��S�čl�������ŏI�I��Transform
    Transform componentToWorld_;     // �L���b�V��

    // �Փˉ��o�� Transform �������Ă��邩
    bool hasPhysicalCorrection_ = false;
protected:
    // ���ݐڑ����Ă���e�B�@valid�@�Ȃ�@relativeLocation_ �Ȃǂ͂��̃I�u�W�F�N�g�ɑ΂��鑊�Βl�ɂȂ�
    std::weak_ptr<SceneComponent> attachParent_; // ��Q��

    // �e�̃\�P�b�g�m�[�h (����̐ڑ��|�C���g) �ɐڑ�����ꍇ�Ɏg�p�����I�v�V�����̃C���f�b�N�X
    int attachSocketNode_ = -1;

    // ���̃R���|�[�l���g�ɐڑ�����Ă���q���̃V�[���R���|�[�l���g�̃��X�g
    std::vector<std::shared_ptr<SceneComponent>> attachChildren_; // �q��ێ��i���L�j

public:
    std::shared_ptr<SceneComponent> GetAttachParent() const
    {
        return attachParent_.lock();
    }

    int GetAttachSocketNode() const
    {
        return attachSocketNode_;
    }

    const std::vector<std::shared_ptr<SceneComponent>>& GetAttachChildren() const
    {
        return attachChildren_;
    }

    void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        inspectorEuler_ = MathHelper::QuaternionToEuler(relativeRotation_);

#if 1
        if (ImGui::TreeNode((name_ + "  Transform").c_str()))
        {
            ImGui::DragFloat3("Relative Location", &relativeLocation_.x, 0.1f);
#if 0
            //if (!inspectorEulerInitialized_)
//{
//    inspectorEulerInitialized_ = true;
//}
//inspectorEuler_ = GetComponentEulerRotation();
            if (ImGui::DragFloat3("Relative Rotation", &inspectorEuler_.x, 1.0f))
            {
                DirectX::XMFLOAT3 eulerRad =
                {
                    DirectX::XMConvertToRadians(inspectorEuler_.x),
                    DirectX::XMConvertToRadians(inspectorEuler_.y),
                    DirectX::XMConvertToRadians(inspectorEuler_.z)
                };
                DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(eulerRad.x, eulerRad.y, eulerRad.z);
                DirectX::XMFLOAT4 q;
                DirectX::XMStoreFloat4(&q, quat);
                //SetWorldRotationDirect(q);
                SetRelativeRotationDirect(q);
            }

#endif // 0
            testAngle = GetRelativeEulerRotation();
            testAngle.x = DirectX::XMConvertToDegrees(testAngle.x);
            testAngle.y = DirectX::XMConvertToDegrees(testAngle.y);
            testAngle.z = DirectX::XMConvertToDegrees(testAngle.z);
            ImGui::DragFloat3("RelativeAngle", &testAngle.x, 1.0f);
            //testAngle.x = MathHelper::ClampAngle(testAngle.x);
            //testAngle.y = MathHelper::ClampAngle(testAngle.y);
            //testAngle.z = MathHelper::ClampAngle(testAngle.z);
            DirectX::XMFLOAT3 eulerRadNew =
            {
                DirectX::XMConvertToRadians(testAngle.x),
                DirectX::XMConvertToRadians(testAngle.y),
                DirectX::XMConvertToRadians(testAngle.z)
            };

            DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw(
                eulerRadNew.x, eulerRadNew.y, eulerRadNew.z
            );

            DirectX::XMFLOAT4 qNew;
            XMStoreFloat4(&qNew, quatNew);
            SetRelativeRotationDirect(qNew);
            //SetQuaternionRotation(qNew);

            //// UI�ŉ�]�p�x�ύX���������ꍇ�͔��f
            //if (ImGui::DragFloat3("Relative Rotation", &inspectorEuler_.x, 1.0f))
            //{
            //    DirectX::XMFLOAT3 eulerRadNew = {
            //        DirectX::XMConvertToRadians(inspectorEuler_.x),
            //        DirectX::XMConvertToRadians(inspectorEuler_.y),
            //        DirectX::XMConvertToRadians(inspectorEuler_.z)
            //    };

            //    DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw(
            //        eulerRadNew.x, eulerRadNew.y, eulerRadNew.z
            //    );

            //    DirectX::XMFLOAT4 qNew;
            //    XMStoreFloat4(&qNew, quatNew);
            //    SetRelativeRotationDirect(qNew);
            //}
            ImGui::DragFloat3("Relative Scale", &relativeScale_.x, 0.01f, 0.01f, 100.0f);
            ImGui::TreePop();
        }


        if (ImGui::TreeNode((name_ + "   Debug Info").c_str()))
        {
            ImGui::Text("Children: %zu", attachChildren_.size());
            ImGui::Text("Parent: %s", attachParent_.lock() ? attachParent_.lock()->name().c_str() : "None");
            ImGui::TreePop();
        }
#endif
#endif
    }
private:

    // �e����̑��ΓI�Ȉʒu
    DirectX::XMFLOAT3 relativeLocation_ = { 0.0f,0.0f,0.0f };

    // �e����̑��ΓI�ȃN�H�[�^�j�I��
    DirectX::XMFLOAT4 relativeRotation_ = { 0.0f,0.0f,0.0f,1.0f };

    // �e����̑��ΓI�ȃX�P�[��
    DirectX::XMFLOAT3 relativeScale_ = { 1.0f,1.0f,1.0f };

private:
    // 0��false 1��true �����Ȃ�8�r�b�g����

    // ���ΓI�Ȉʒu�A��]�A�X�P�[���Ɋ�Â��� worldTransform�@���X�V�������Ƃ������ true
    // �J�n���Ɂ@worldTrnasform ������������Ă��邩�ǂ������m�F���邽�߂Ɏg��
    uint8_t componentToWorldTransformUpdate_ : 1 = 0; // <-�P�r�b�g�����g�� 

    // true�̎��ɁA���̃R���|�[�l���g�₻�̎q�ɑ΂��āAupdateOverlaps���Ăяo���K�v���Ȃ�
    // ����́A�c���[��H���Ă������蔻��̍X�V���s�v�ȏꍇ�ɁA�p�t�H�[�}���X�œK���Ƃ��Ďg����
    // �ʏ킱�̃t���O�́@UpdateOverlaps ���s��� true�@�ɃZ�b�g�����
    // ��ԁi�A�^�b�`�ύX�A�����蔻��ݒ�Ȃǁj���ς�����Ƃ��́AclearSkipUpdateOverlaps() ���Ă�Ńt���O�𖳌�������B
    uint8_t skipUpdateOverlaps_ : 1 = 0; //�� �����蔻��̍X�V�������X�L�b�v���č��������邽�߂̃t���O
    // �q�R���|�[�l���g�ɉe�����Ȃ��ꍇ�A���ʂȏ����������

//�@�ʏ�A�V�[���R���|�[�l���g�̈ʒu�Ȃǂ͐e����̑��΍��W�����ǁA
// �����t���O��true���ƃ��[���h�ɑ΂��Ē��ڎw�肳���
// relativeLocation_ ��e�ł͂Ȃ����[���h���W�n�ɑ΂���ʒu�Ƃ݂Ȃ��ꍇ�� true
    uint8_t absoluteLocation_ : 1 = 0;
    // relativeRotation_ ��e�ł͂Ȃ����[���h���W�n�ɑ΂���ʒu�Ƃ݂Ȃ��ꍇ�� true
    uint8_t absoluteRotation_ : 1 = 0;
    // relativeScale_ ��e�ł͂Ȃ����[���h���W�n�ɑ΂���ʒu�Ƃ݂Ȃ��ꍇ�� true
    uint8_t absoluteScale_ : 1 = 0;

    // true �̏ꍇ�͂��̃R���|�[�l���g�͕`�悳�ꂩ�A�e�����Ƃ�
    // false �̏ꍇ�͕`������ꂸ�A�e�����Ƃ��Ȃ�
    uint8_t visible_ : 1 = 1;

public:
    bool IsUsingAbsoluteLocation() const
    {
        return absoluteLocation_;
    }
    void SetUsingAbsoluteLocation(bool absoluteLocation)
    {
        absoluteLocation_ = absoluteLocation ? 1 : 0;
    }
    bool IsUsingAbsoluteRotation() const
    {
        return absoluteRotation_;
    }
    void SetUsingAbsoluteRotation(bool absoluteRotation)
    {
        absoluteRotation_ = absoluteRotation ? 1 : 0;
    }
    bool IsUsingAbsoluteScale() const
    {
        return absoluteScale_;
    }
    void SetUsingAbsoluteScale(bool absoluteScale)
    {
        absoluteScale_ = absoluteScale ? 1 : 0;
    }


public:
    // ���ΓI�ȍ��W���擾
    DirectX::XMFLOAT3 GetRelativeLocation() const
    {
        return relativeLocation_;
    }
    // ���ځ@���ΓI�ȍ��W��ݒ�
    void SetRelativeLocationDirect(const DirectX::XMFLOAT3& newRelativeLocation)
    {
        relativeLocation_ = newRelativeLocation;
    }
    // ���ΓI�ȃX�P�[�����擾
    DirectX::XMFLOAT3 GetRelativeScale()const
    {
        return relativeScale_;
    }
    // ���ځ@���ΓI�ȃX�P�[����ݒ�
    void SetRelativeScaleDirect(const DirectX::XMFLOAT3& newRelativeScale)
    {
        relativeScale_ = newRelativeScale;
    }
    // ���ΓI�Ȋp�x���擾
    DirectX::XMFLOAT3 GetRelativeEulerRotation()const
    {
        DirectX::XMFLOAT3 angle= MathHelper::QuaternionToEuler(relativeRotation_);
        return angle;
    }
    DirectX::XMFLOAT4 QuaternionFromEulerYXZ(const DirectX::XMFLOAT3& eulerRadians)
    {
        using namespace DirectX;
        XMVECTOR qx = XMQuaternionRotationAxis({ 1,0,0 }, eulerRadians.x);
        XMVECTOR qy = XMQuaternionRotationAxis({ 0,1,0 }, eulerRadians.y);
        XMVECTOR qz = XMQuaternionRotationAxis({ 0,0,1 }, eulerRadians.z);

        // ���� Y �� X �� Z�iYXZ intrinsic�j
        XMVECTOR q = XMQuaternionMultiply(qz, XMQuaternionMultiply(qx, qy));
        XMFLOAT4 result;
        XMStoreFloat4(&result, q);
        return result;
    }
    // ���ځ@���ΓI�Ȋp�x��ݒ�
    void SetRelativeEulerRotationDirect(const DirectX::XMFLOAT3& newEulerRotaion)
    {
        DirectX::XMStoreFloat4(&relativeRotation_, DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(newEulerRotaion.x), DirectX::XMConvertToRadians(newEulerRotaion.y), DirectX::XMConvertToRadians(newEulerRotaion.z)));
    }
    // ���ΓI�ȃN�H�[�^�j�I�����擾
    const DirectX::XMFLOAT4& GetRelativeRotation() const
    {
        return relativeRotation_;
    }
    // ���ځ@���ΓI�ȃN�H�[�^�j�I����ݒ�
    void SetRelativeRotationDirect(const DirectX::XMFLOAT4& newRelativeRotation)
    {
        relativeRotation_ = newRelativeRotation;
    }
    //  ���̃R���|�[�l���g�̃��[���h��ԏ�ł�Transform���擾
    const Transform& GetComponentWorldTransform() const
    {
        return componentToWorld_;
    }
    //  ���̃R���|�[�l���g�̃��[���h��ԏ�ł�Transform���擾(���������p)
    Transform& GetComponentWorldTransform()
    {
        return componentToWorld_;
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̈ʒu���擾
    DirectX::XMFLOAT3 GetComponentLocation() const
    {
        return GetComponentWorldTransform().GetLocation();
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̃N�H�[�^�j�I�����擾
    DirectX::XMFLOAT4 GetComponentRotation() const
    {
        return GetComponentWorldTransform().GetRotation();
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̃X�P�[�����擾
    DirectX::XMFLOAT3 GetComponentScale() const
    {
        return GetComponentWorldTransform().GetScale();
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̊p�x���擾
    DirectX::XMFLOAT3 GetComponentEulerRotation() const
    {
        return GetComponentWorldTransform().GetEulerRotation();
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̍��W��ݒ�
    // �i�e������ꍇ�́A�e�̃��[���h�s����g���đ��Έʒu�ɕϊ����ăZ�b�g����j
    virtual void SetWorldLocationDirect(const DirectX::XMFLOAT3& newWorldLocation)
    {
#if 0
        GetComponentWorldTransform().SetTranslation(newWorldLocation);
#else
        if (auto parent = attachParent_.lock())
        {
            // �e�̃��[���h�g�����X�t�H�[���̋t�s����g���āA���Έʒu�����߂�
#if 1
            DirectX::XMMATRIX parentWorld = parent->GetComponentWorldTransform().ToMatrix();
            DirectX::XMMATRIX parentInv = DirectX::XMMatrixInverse(nullptr, parentWorld);
            DirectX::XMVECTOR worldPos = DirectX::XMLoadFloat3(&newWorldLocation);
            DirectX::XMVECTOR localPos = DirectX::XMVector3TransformCoord(worldPos, parentInv);
            DirectX::XMFLOAT3 relative;
            DirectX::XMStoreFloat3(&relative, localPos);
            SetRelativeLocationDirect(relative);
#else
            parent->SetRelativeLocationDirect(newWorldLocation);
#endif
        }
        else
        {
            SetRelativeLocationDirect(newWorldLocation);
        }
#endif
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̃N�H�[�^�j�I����ݒ�
    void SetWorldRotationDirect(const DirectX::XMFLOAT4& newWorldRotation)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeRotationDirect(newWorldRotation);
        }
        else
        {
            SetRelativeRotationDirect(newWorldRotation);
        }
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̊p�x��ݒ�
    void SetWorldEulerRotationDirect(const DirectX::XMFLOAT3& newWorldEuler)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeEulerRotationDirect(newWorldEuler);
        }
        else
        {
            SetRelativeEulerRotationDirect(newWorldEuler);
        }
    }
    // ���[���h��Ԃł̂��̃R���|�[�l���g�̃X�P�[����ݒ�
    void SetWorldScaleDirect(const DirectX::XMFLOAT3& newWorldScale)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeScaleDirect(newWorldScale);
        }
        else
        {
            SetRelativeScaleDirect(newWorldScale);
        }
    }



    // ���̃R���|�[�l���g�����������ɃR�[���o�b�N(�Ăяo)�����֐�
    virtual void OnUpdateTransform(UpdateTransformFlags updateTransformFlags, TeleportType teleport = TeleportType::None)
    {
    }

    // �R���|�[�l���g�� worldTransform �́@update ���@false ��������@worldTrasnformUpdate ���Ă�
    void ConditionalUpdateComponentWorldTransform()
    {
        if (!componentToWorldTransformUpdate_)
        {
            UpdateComponentToWorld();
        }
    }

    // ���̃R���|�[�l���g�ɃA�^�b�`����Ă���S�Ă̎q�R���|�[�l���g������ Transform ���X�V����
    void UpdateChildTransforms(UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None);

protected:
    // ���̃R���|�[�l���g�̐e����̑��ΓI�� Transform ��Ԃ�
    Transform GetRelativeTransform() const
    {
        return Transform(relativeLocation_, relativeRotation_, relativeScale_);
    }

    // �w�肳�ꂽ�\�P�b�g�m�[�h�̃��[���h��Ԃ�Transform��Ԃ�
    // �\�P�b�g��������Ȃ������ꍇ�́A���g�� WorldTransform ��Ԃ�
    virtual Transform GetSocketTransform(int socketNode) const
    {
        // TODO: // ���g�̃��[���h��Ԃ̃g�����X�t�H�[��

        return componentToWorld_;
    }

    // ���̃R���|�[�l���g�̐V�����@componentToWorld_Transform���v�Z����
    // parent �͏ȗ��\�ŁA�C�ӂ� sceneComponent ���g���Čv�Z�ł���
    // �w�肳��Ȃ��ꍇ�́A�R���|�[�l���g���g�́@attachParent ���g��
    Transform CalculateNewComponentToWorldTransform(const Transform& newRelativeTransform, const SceneComponent* parent = nullptr, int socketNode = -1)const
    {
        // socketNode �� parent ���w�肳��Ă��Ȃ���΁A�A�^�b�`���ꂽ��񂩂�擾
        socketNode = parent ? socketNode : attachSocketNode_;
        parent = parent ? parent : attachParent_.lock().get();
        if (parent)
        {
            // �u��Έʒu�E��Ή�]�E��΃X�P�[���v�̂����ꂩ���g���Ă��邩
            const bool general = IsUsingAbsoluteLocation() || IsUsingAbsoluteRotation() || IsUsingAbsoluteScale();
            if (!general)
            {// ��΍��W�w�肪����Ă��Ȃ��Ȃ�i���ʏ�̐e����̑��ΓI�ȍ��W�Ȃ�j
                return newRelativeTransform * parent->GetSocketTransform(socketNode);
            }
            // ��Ύw�肪�܂܂�Ă��邩��A����ȍ����������s��
            return CalculateNewComponentToWorldGeneralCase(newRelativeTransform, parent, socketNode);
        }
        else
        {// �e�����݂��Ȃ����́A�����̑��� Transform �����̂܂� WorldTransform �ɂȂ�
            return newRelativeTransform;
        }
    }

    // �e�� Transform ���g�p���āA��Ύw��̗v�f���l�������@componentToWorldTransform ���v�Z����
    Transform CalculateNewComponentToWorldGeneralCase(const Transform& newRelativeTransform, const SceneComponent* parent, int socketNode)const
    {
        if (parent != nullptr)
        {
            const Transform parentToWorld = parent->GetSocketTransform(socketNode);
            Transform newComponentToWorldTransform = newRelativeTransform * parentToWorld;

            // ��Έʒu���L���Ȃ�A�ʒu�����͐e�̉e���𖳎�
            if (absoluteLocation_)
            {
                newComponentToWorldTransform.translation_ = newRelativeTransform.translation_;
            }
            // ��Ή�]���L���Ȃ�A��]�����͐e�̉e���𖳎�
            if (absoluteRotation_)
            {
                newComponentToWorldTransform.rotation_ = newRelativeTransform.rotation_;
            }
            // ��΃X�P�[�����L���Ȃ�A�X�P�[�������͐e�̉e���𖳎�
            if (absoluteScale_)
            {
                //newComponentToWorldTransform.scale_ = newRelativeTransform.scale_;
                // �����̑���ł͂Ȃ��A�����̕␳���������X�P�[������
                newComponentToWorldTransform.scale_ = DirectX::XMVectorMultiply(newRelativeTransform.scale_, MathHelper::VectorSign(newComponentToWorldTransform.scale_));
            }

            return newComponentToWorldTransform;
        }
        else
        {
            return newRelativeTransform;
        }
    }

    // ���̃R���|�[�l���g���A�w�肳�ꂽ�e�R���|�[�l���g�ɃA�^�b�`�i�ڑ��j����
    // parent �̓A�^�b�`��̐e�R���|�[�l���g�@
    // socketNode �ڑ���̃\�P�b�g�m�[�h�ԍ�( -1 �Ȃ�f�t�H���g)
    void AttachToComponent(const std::shared_ptr<SceneComponent>& parent, int socketNode);

    // ���̃R���|�[�l���g�����݂̐e�̃R���|�[�l���g����؂藣��
    void DetachFromParent();


private:
    //�uTransform���ς�������╨���I�Ɉړ��������ɁA�����Ƃ��̎q�����𐳂����X�V���邽�߂̒��S�I�ȏ����v
    // ���g�� Transform �̕ύX�𔽉f���A�K�v�ɉ����Ďq�R���|�[�l���g�ɂ������`�d������֐�
    void PropagateTransformUpdate(bool transformChanged, UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None);

    // �e�� Transform �����ƂɁA������ Transform�i= ComponentToWorld�j���X�V���A�K�v�ɉ����Ă��̕ύX���q�֓`�d����
    void UpdateComponentToWorldWithParent(SceneComponent* parent, int socketNode, UpdateTransformFlags updateTransformFlags, TeleportType teleport);


    //  �w�肳�ꂽ component ���A���̃R���|�[�l���g�̎q���i�q�A���Ȃǁj�ł��邩�𔻒肷��֐��B
    // component ���������g�܂��͎q���ł���� true ��Ԃ��āA����ȊO�� false ��Ԃ�
    bool IsAttachBelow(SceneComponent* component)
    {
        // �������g�ƈ�v����ꍇ�� true�i��component �͎����A����Ďq���֌W�ɂ���j
        if (component == this)
        {
            return true;
        }

        // �q�R���|�[�l���g���ꂼ��ɑ΂��āA�ċA�I�Ɏq���֌W���`�F�b�N
        for (const std::shared_ptr<SceneComponent>& child : attachChildren_)
        {
            if (child->IsAttachBelow(component))   // �ċA�I�`�F�b�N
            {
                return true;
            }
        }

        // �ǂ̎q���ɂ��Y�����Ȃ���� false
        return false;
    }

    // �w�肳�ꂽ component ���A���̃R���|�[�l���g�̑c��i�e�A�c����Ȃǁj�ł��邩�𔻒肷��֐��B
    // component ���c��ł���� true ��Ԃ��A����ȊO�� false ��Ԃ��B
    bool IsAttachAbove(SceneComponent* component)
    {
        // �e����H���Ă����āA�w�肳�ꂽ component �Ɉ�v���邩���`�F�b�N
        for (SceneComponent* parent = attachParent_.lock().get(); parent; parent = parent->attachParent_.lock().get())
        {
            if (parent == component)
            {
                return true;
            }
        }

        // �ǂ̐e�ɂ���v���Ȃ���� false
        return false;
    }

    friend class Actor;

public:
    virtual void UpdateComponentToWorld(UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None) override final;

    virtual void Tick(float deltaTime) {}


    virtual void Initialize()override {};

    virtual void OnRegister()override {} // �h���N���X�� override ���ēo�^����������

    virtual void OnUnregister()override {} // �h���N���X�� override ���ĉ�������������


#if 0
    void SetLerpQuaternion(DirectX::XMFLOAT3 angle)
    {
        DirectX::XMFLOAT3 euler = { DirectX::XMConvertToRadians(angle.x),DirectX::XMConvertToRadians(angle.y),DirectX::XMConvertToRadians(angle.z) };
        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
        DirectX::XMStoreFloat4(&afterRotation, q);
        if (std::abs(beforeRotation.y - afterRotation.y) <= FLT_EPSILON)
        {// �O��rotaion�ƕύX���rotation���ꏏ�̏ꍇ�X�L�b�v����
            return;
        }
        beforeRotation = rotationLocal;
        lerpTime = 0.0f;
    }

    void LerpQuaternion(float deltaTime)
    {
        DirectX::XMVECTOR qAfter = DirectX::XMLoadFloat4(&afterRotation);
        DirectX::XMVECTOR qBefore = DirectX::XMLoadFloat4(&beforeRotation);
        lerpTime += deltaTime * 0.8f;
        if (lerpTime > 1.0f)
        {
            lerpTime = 1.0f;
        }
        DirectX::XMVECTOR q = DirectX::XMQuaternionSlerp(qBefore, qAfter, lerpTime);
        DirectX::XMStoreFloat4(&rotationLocal, q);
    }

#endif

    // �e�q�֌W�Z�b�g
    void AttachTo(const std::shared_ptr<SceneComponent>& parent)
    {
        attachParent_ = parent;
        parent->attachChildren_.push_back(shared_from_this());
        // ������x�m�F
    }
    void AddWorldOffset(const DirectX::XMFLOAT3& offset);

    bool isDirty = true;


    // �e�X�g�̂��ɍ폜
    DirectX::XMFLOAT4 afterRotation = { 0.0f,0.0f,0.0f,1.0f }; // �N�H�[�^�j�I��
    DirectX::XMFLOAT4 beforeRotation = { 0.0f,0.0f,0.0f,1.0f }; // �N�H�[�^�j�I��
    float lerpTime = 0.0f;


    // �e�X�g
    DirectX::XMFLOAT3 testAngle = { 0.0f,0.0f,0.0f };
};


#endif  //SCENE_COMPONENT_H