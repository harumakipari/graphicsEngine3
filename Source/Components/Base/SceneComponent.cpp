#include "SceneComponent.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Win32Utils.h"


void SceneComponent::UpdateComponentToWorld(UpdateTransformFlags updateTransformFlags, TeleportType teleport)
{
    UpdateComponentToWorldWithParent(GetAttachParent().get(), GetAttachSocketNode(), updateTransformFlags, teleport);
}

// �e�� Transform �����ƂɁA������ Transform�i= ComponentToWorld�j���X�V���A�K�v�ɉ����Ă��̕ύX���q�֓`�d����
void SceneComponent::UpdateComponentToWorldWithParent(SceneComponent* parent, int socketNode, UpdateTransformFlags updateTransformFlags, TeleportType teleport)
{
    // �e���܂��X�V����Ă��Ȃ��ꍇ�́A�e�̊K�w������čX�V����K�v������
    if (parent && !parent->componentToWorldTransformUpdate_)
    {
        parent->UpdateComponentToWorld();
        // �e�̍X�V�ɂ���Ď������g�̍X�V���������Ă��邩������Ȃ��̂ŁA�����Ȃ珈���𒆒f����
        if (componentToWorldTransformUpdate_)
        {
            return;
        }
    }
    componentToWorldTransformUpdate_ = true;

    Transform newTransform;
    // �V���� ComponentToWorld �ϊ����v�Z����
    const Transform relativeTransform(relativeLocation_, relativeRotation_, relativeScale_);

    newTransform = CalculateNewComponentToWorldTransform(relativeTransform, parent, socketNode);

    // Transform �ɕύX�������������肷��i���������_�̌덷�����e���Ĕ�r�j
    bool hasChanged = !GetComponentWorldTransform().Equals(newTransform, 1.0e-8f);

    // Transform �ɕύX������A�܂��� Teleport �w�肪����Ă���Ȃ�΁A
    // ���̃R���|�[�l���g�� Teleport �����m����K�v������\��������
    if (hasChanged || teleport != TeleportType::None)
    {
        // Transform ���X�V
        componentToWorld_ = newTransform;

        // �����␳�͒ʏ�X�V���������疳���ɂ���
        ClearPhysicalCorrection();

        // �ύX���q�ɓ`�d������
        PropagateTransformUpdate(true, updateTransformFlags, teleport);
    }
    else
    {
        // Transform �͕ς���Ă��Ȃ����A�q�R���|�[�l���g�Ȃǂ̂��߂ɓ`�d����K�v������
        PropagateTransformUpdate(false);
    }
}

// ���̃R���|�[�l���g�ɃA�^�b�`����Ă���S�Ă̎q�R���|�[�l���g�� Transform ���X�V����
void SceneComponent::UpdateChildTransforms(UpdateTransformFlags updateTransformFlags, TeleportType teleport)
{
    if (attachChildren_.size() > 0)
    {// �q�R���|�[�l���g�����݂��Ă�����A

        // OnlyUpdateIfUsingSocket �t���O���ݒ肳��Ă��邩�ǂ����𔻒�B
        // ���̃t���O���L���ȂƂ��́A�u�e�ƃ\�P�b�g�ڑ�����Ă��Ȃ��q�v�� Transform �X�V�ΏۊO�ɂȂ�B
        const bool onlyUpdateIfUsingSocket = !!static_cast<bool>(updateTransformFlags & UpdateTransformFlags::OnlyUpdateIfUsingSocket);

        // OnlyUpdateIfUsingSocket �t���O�����O�����X�V�t���O���쐬
        const UpdateTransformFlags updateTransformNoSocketSkip = ~UpdateTransformFlags::OnlyUpdateIfUsingSocket & updateTransformFlags;
        // PropagateFromParent�@�t���O�i�u�e����`�d���Ă����X�V�v�Ƃ�����j��ǉ��B
        const UpdateTransformFlags updateTransformFlagsFromParent = updateTransformNoSocketSkip | UpdateTransformFlags::PropagateFromParent;

        // ���ׂĂ̎q�R���|�[�l���g�ɑ΂��ď������s���B
        for (const std::shared_ptr<SceneComponent>& childComponent : GetAttachChildren())
        {
            if (childComponent != nullptr)
            {
                if (!childComponent->componentToWorldTransformUpdate_)
                {// �܂���x���X�V����Ă��Ȃ��q�Ȃ炻�̂܂܍X�V
                    childComponent->UpdateComponentToWorld(updateTransformFlagsFromParent, teleport);
                }
                else
                {// ���łɍX�V���ꂽ�q�R���|�[�l���g�Ȃ�����ɉ����ăX�L�b�v or �X�V�F

                    if (onlyUpdateIfUsingSocket && (childComponent->attachSocketNode_ == -1))
                    {// �u�\�P�b�g�ڑ�������q�����X�V�v�w�肠��A���\�P�b�g���g���Ă��Ȃ���΃X�L�b�v
                        continue;
                    }
                    if (childComponent->IsUsingAbsoluteLocation() && childComponent->IsUsingAbsoluteRotation() && childComponent->IsUsingAbsoluteScale())
                    {// �q�����ׂĐ�΍��W�i�ʒu�E��]�E�X�P�[���j���g���Ă���Ȃ�X�V�s�v
                        continue;
                    }

                    childComponent->UpdateComponentToWorld(updateTransformFlagsFromParent, teleport);
                }
            }
        }
    }
}

// ���g�� Transform �̕ύX�𔽉f���A�K�v�ɉ����Ďq�R���|�[�l���g�ɂ������`�d������֐�
void SceneComponent::PropagateTransformUpdate(bool transformChanged, UpdateTransformFlags updateTransformFlags, TeleportType teleport)
{
    // �A�^�b�`����Ă���q�R���|�[�l���g�̈ꗗ���擾
    const std::vector<std::shared_ptr<SceneComponent>>& attachedChildren = GetAttachChildren();

    if (transformChanged)
    {
        if (attachChildren_.size() > 0)
        {// �q�R���|�[�l���g������ꍇ�́A�������X�V
            // �q���ɂ� skipPhysicsUpdate �t���O��n���Ȃ��i�����G���W�������䂵�Ă���ꍇ�������j
            UpdateTransformFlags childrenFlagNoPhysics = ~UpdateTransformFlags::SkipPhysicsUpdate & updateTransformFlags;
            // �q�������� Transform ���X�V�i�ċA�I�j
            UpdateChildTransforms(childrenFlagNoPhysics, teleport);
        }
    }
    else
    {
        // �q�R���|�[�l���g������Ȃ�A�f�t�H���g�ݒ�ōX�V��`�d
        if (attachChildren_.size() > 0)
        {
            UpdateChildTransforms();
        }
    }
}

// ���̃R���|�[�l���g���A�w�肳�ꂽ�e�R���|�[�l���g�ɃA�^�b�`�i�ڑ��j����
// parent �̓A�^�b�`��̐e�R���|�[�l���g�@
// socketNode �ڑ���̃\�P�b�g�m�[�h�ԍ�( -1 �Ȃ�f�t�H���g)
void SceneComponent::AttachToComponent(const std::shared_ptr<SceneComponent>& parent, int socketNode)
{
    // �G���[: parent �� nullptr �̏ꍇ�̓A�T�[�g�i�����ȃ|�C���^�j
    _ASSERT_EXPR(parent != nullptr, L"�e�R���|�[�l���g�̃|�C���^�� null �ł�");
    // �G���[: �������g�ɃA�^�b�`���悤�Ƃ��Ă���ꍇ
    _ASSERT_EXPR(parent.get() != this, L"�������g�ɂ̓A�^�b�`�o���܂���");
    // �G���[: ���� Actor ���ŁA���[�g�R���|�[�l���g�����̃R���|�[�l���g�ɃA�^�b�`����悤�Ƃ��Ă���
    _ASSERT_EXPR(!(owner_.lock() == parent->owner_.lock() && owner_.lock() && owner_.lock()->GetRootComponent().get() == this),
        L"�����A�N�^�[���̃��[�g�R���|�[�l���g�͑��ɃA�^�b�`�ł��܂���");
    // �G���[: �T�C�N���i�z�j����낤�Ƃ��Ă���ꍇ�i�e�����ɂ��̃R���|�[�l���g�ɂԂ牺�����Ă���j
    _ASSERT_EXPR(!parent->IsAttachAbove(this), L"�z�Q�Ƃ��������܂�");

    // ���łɑ��ɃA�^�b�`����Ă���ꍇ�̓f�^�b�` (�e��ύX)
    if (attachParent_.lock())
    {
        DetachFromParent();
    }

    // �e�̎q�����X�g�Ɏ�����ǉ�
    parent->attachChildren_.emplace_back(std::dynamic_pointer_cast<SceneComponent>(shared_from_this()));

    // �e�ւ̎Q�Ƃƃ\�P�b�g�m�[�h���L�^
    attachParent_ = parent;
    attachSocketNode_ = socketNode;
}

// ���̃R���|�[�l���g�����݂̐e�̃R���|�[�l���g����؂藣��
void SceneComponent::DetachFromParent()
{
    std::shared_ptr<SceneComponent> parent = attachParent_.lock();
    if (parent)
    {
        // �f�^�b�`���ɂ��T�C�N���`�F�b�N�i�O�̂��߁j
        _ASSERT_EXPR(!parent->IsAttachAbove(this), L"�z�Q�Ƃ��������܂�");

        // �e�̎q�����X�g���玩�����폜
        for (decltype(parent->attachChildren_)::iterator child = parent->attachChildren_.begin(); child != parent->attachChildren_.end();)
        {
            if (child->get() == this)
            {
                parent->attachChildren_.erase(child);
                break;
            }
            child++;
        }

        // �e�̎Q�Ƃ��N���A
        attachParent_.reset();
    }
    // �\�P�b�g�������Z�b�g
    attachSocketNode_ = -1;
}

// ��������
void SceneComponent::Destroy()
{
    // �q�����ɍ폜
    for (auto& child : attachChildren_)
    {
        if (child)
        {
            child->Destroy();
        }
    }
    attachChildren_.clear();

    // �e�Ƃ̐ڑ�������
    DetachFromParent();

    OnUnregister();
    SetActive(false);
}

// ���������ɍ��邩�瑦�� Transform �X�V����
void SceneComponent::UpdateTransformImmediate()
{
    UpdateComponentToWorld();
    for (auto& child : attachChildren_)
    {
        child->UpdateTransformImmediate();
    }
}


void SceneComponent::AddWorldOffset(const DirectX::XMFLOAT3& offset)
{
    // ���݂̃��[���h���W���擾
    //DirectX::XMMATRIX worldMat = componentToWorld_.ToMatrix();
    //DirectX::XMVECTOR currentPos = DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), worldMat);

    // offset �����Z
    DirectX::XMFLOAT3 worldPos = GetComponentLocation();
    //DirectX::XMStoreFloat3(&worldPos, currentPos);

    worldPos.x += offset.x;
    worldPos.y += offset.y;
    worldPos.z += offset.z;

    GetOwner()->SetPosition(worldPos);

    //SetWorldLocationDirect(worldPos);
#if 0
    SetWorldPosition(worldPos);
    UpdateWorldMatrix();
#endif
}
