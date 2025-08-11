#include "EraseInAreaComponent.h"

#include "Core/ActorManager.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Player/Player.h"

// 衝突イベント
void EraseInAreaComponent::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitShapes)
{
    //OutputDebugStringA(hitShapes.second->GetActor()->GetName().c_str());
    //OutputDebugStringA(hitShapes.first->GetActor()->GetName().c_str());
    if (auto player = std::dynamic_pointer_cast<Player>(hitShapes.second->GetActor()))
    {// player の時
        if (player->IsInvincible())
        {// 無敵中はスキップ
            return;
        }

        if (hitShapes.second->name() == "playerDamageLeft")
        {// 左に当たった時
            player->hitLeftThisFrame = true;
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else if (hitShapes.second->name() == "playerDamageRight")
        {// 右に当たった時
            player->hitRightThisFrame = true;
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else if (hitShapes.second->name() == "capsuleComponent")
        {// 本体にのみ当たった時
            if (!player->hitLeftThisFrame && !player->hitRightThisFrame)
            {
                player->ApplyDirectHpDamage(damage);
            }
            std::string a = hitShapes.second->name() + "is Hit\n";
            OutputDebugStringA(a.c_str());
        }
        else
        {// そのほかに当たった時
            return;
        }


        // 初回だけダメージを登録する
        if (!player->hasDamageThisFrame)
        {// ダメージ設定をしていなかったら
            player->hasDamageThisFrame = true;
            player->currentFrameDamage = damage;
            player->applyInvincibilityNextFrame = true;
        }

        //std::string a = player->GetName() + "is Hit";
        //OutputDebugStringA(a.c_str());

    }
    else if (auto item = std::dynamic_pointer_cast<PickUpItem>(hitShapes.second->GetActor()))
    {
        // ステージ上に湧かせるアイテムの max の値を決める
        if (auto itemManager = GameManager::GetItemManager())
        {
            if (item->GetType() == PickUpItem::Type::RandomSpawn)
            {// ランダムスポーンのアイテムを拾ったら　エリアに通知する
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
