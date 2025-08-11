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
    PickUpItem = 1 << 5, // �n�ʂɗN���A�C�e��
    HeldItem = 1 << 6, // player �̉��ɂ������A�C�e��
    PlayerSide = 1 << 7, // player �̉��ɂ������
    Building = 1 << 8, // Convex�@�O�̂���
    EnemyHand = 1 << 9,   // Enemy �� ��
    WorldProps = 1 << 10, // stage �� object
    ShockWave = 1 << 11, // �Ռ��g
    Camera = 1 << 12,   // �J����
    EraseInArea = 1 << 13, // �����G���A�@�G�̔��e�Ȃ�
    Bomb = 1 << 14,// ���e


};



namespace CollisionHelper
{
    // �P�ꃌ�C���[���r�b�g�ɕϊ�
    inline uint32_t ToBit(CollisionLayer layer)
    {
        uint32_t bit = (1u << static_cast<uint32_t>(layer));
        return bit;
    }

    // �������C���[���܂Ƃ߂ă}�X�N�ɕϊ�
    inline uint32_t MakeMask(std::initializer_list<CollisionLayer> layers)
    {
        uint32_t mask = 0;
        for (auto layer : layers)
        {
            mask |= ToBit(layer);
        }
        return mask;
    }

    // �}�X�N�Ƀ��C���[��ǉ�
    inline void AddToMask(uint32_t& mask, CollisionLayer layer)
    {
        mask |= ToBit(layer);
    }

    // �}�X�N���烌�C���[������
    inline void RemoveFromMask(uint32_t& mask, CollisionLayer layer)
    {
        mask &= ~ToBit(layer);
    }

    // �}�X�N�Ɋ܂܂�Ă��邩�m�F
    inline bool HasLayer(uint32_t mask, CollisionLayer layer)
    {
        return (mask & ToBit(layer)) != 0;
    }
}

#endif //COLLISION_HELPER_H