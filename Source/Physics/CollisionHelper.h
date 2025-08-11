#ifndef COLLISION_HELPER_H
#define COLLISION_HELPER_H

#include <cmath>
#include <list>

enum class CollisionLayer :uint32_t
{
    None = 0,
    WorldStatic = 1 << 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3,
    Convex = 1 << 4,
    PickUpItem = 1 << 5, // 地面に湧くアイテム
    HeldItem = 1 << 6, // player の横にくっつくアイテム
    PlayerSide = 1 << 7, // player の横にあるもの
    Building = 1 << 8, // Convex　前のもの
    EnemyHand = 1 << 9,   // Enemy の 手
    WorldProps = 1 << 10, // stage の object
    ShockWave = 1 << 11, // 衝撃波
    Camera = 1 << 12,   // カメラ
    EraseInArea = 1 << 13, // 消すエリア　敵の爆弾など
    Bomb = 1 << 14,// 爆弾


};



namespace CollisionHelper
{
    // 単一レイヤーをビットに変換
    inline uint32_t ToBit(CollisionLayer layer)
    {
        uint32_t bit = (1u << static_cast<uint32_t>(layer));
        return bit;
    }

    // 複数レイヤーをまとめてマスクに変換
    inline uint32_t MakeMask(std::initializer_list<CollisionLayer> layers)
    {
        uint32_t mask = 0;
        for (auto layer : layers)
        {
            mask |= ToBit(layer);
        }
        return mask;
    }

    // マスクにレイヤーを追加
    inline void AddToMask(uint32_t& mask, CollisionLayer layer)
    {
        mask |= ToBit(layer);
    }

    // マスクからレイヤーを除去
    inline void RemoveFromMask(uint32_t& mask, CollisionLayer layer)
    {
        mask &= ~ToBit(layer);
    }

    // マスクに含まれているか確認
    inline bool HasLayer(uint32_t mask, CollisionLayer layer)
    {
        return (mask & ToBit(layer)) != 0;
    }
}

#endif //COLLISION_HELPER_H