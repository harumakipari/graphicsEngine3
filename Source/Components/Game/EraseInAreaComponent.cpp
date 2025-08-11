#include "EraseInAreaComponent.h"

#include "Core/ActorManager.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Player/Player.h"

// �Փ˃C�x���g
void EraseInAreaComponent::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)
{
    //OutputDebugStringA(hitShapes.second->GetActor()->GetName().c_str());
    //OutputDebugStringA(hitShapes.first->GetActor()->GetName().c_str());
    if (auto player = std::dynamic_pointer_cast<Player>(hitShapes.second->GetActor()))
    {// player �̎�
        if (player->IsInvincible())
        {// ���G���̓X�L�b�v
            return;
        }

        if (hitShapes.second->name() == "playerDamageLeft")
        {// ���ɓ���������
            player->hitLeftThisFrame = true;
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else if (hitShapes.second->name() == "playerDamageRight")
        {// �E�ɓ���������
            player->hitRightThisFrame = true;
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else if (hitShapes.second->name() == "capsuleComponent")
        {// �{�̂ɂ̂ݓ���������
            if (!player->hitLeftThisFrame && !player->hitRightThisFrame)
            {
                player->ApplyDirectHpDamage(damage);
            }
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else
        {// ���̂ق��ɓ���������
            return;
        }


        // ���񂾂��_���[�W��o�^����
        if (!player->hasDamageThisFrame)
        {// �_���[�W�ݒ�����Ă��Ȃ�������
            player->hasDamageThisFrame = true;
            player->currentFrameDamage = damage;
            player->applyInvincibilityNextFrame = true;
        }

        //std::string a = player->GetName() + "is Hit";
        //OutputDebugStringA(a.c_str());

    }
    else if (auto item = std::dynamic_pointer_cast<PickUpItem>(hitShapes.second->GetActor()))
    {
        // �X�e�[�W��ɗN������A�C�e���� max �̒l�����߂�
        if (auto itemManager = GameManager::GetItemManager())
        {
            if (item->GetType() == PickUpItem::Type::RandomSpawn)
            {// �����_���X�|�[���̃A�C�e�����E������@�G���A�ɒʒm����
                const auto& pos = item->GetPosition();
                itemManager->DecreaseAreaItemCount(pos);
            }
        }
        item->SetPendingDestroy();
        //item->SetValid(false);
        //std::string a = item->GetName() + "is Hit";
        //OutputDebugStringA(a.c_str());
    }
}
