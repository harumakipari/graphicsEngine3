#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <memory>

#include "ItemManager.h"
#include "BuildingManager.h"
#include "Widgets/ObjectManager.h"
#include "Widgets/GameObject.h"

class GameManager
{
public:
    static void Initialize();

    static void Update(float deltaTime);

    static void DrawGUI();

    static void DebugRender(ID3D11DeviceContext* immediateContext);

    static void Finalize();

    static ItemManager* GetItemManager()
    {
        return itemManager_ ? itemManager_.get() : nullptr;
    }

    static BuildingManager* GetBuildingManager()
    {
        return buildingManager_ ? buildingManager_.get() : nullptr;
    }

    // �v���C���[����e���鎞�ɌĂԊ֐�
    static void CallPlayerDamaged() 
    { 
        playerDamageCount++; 
        //char buf[256];
        //sprintf_s(buf, " Called PlayerDamageCount:%d\n", playerDamageCount);
        //OutputDebugStringA(buf);
    }

    // �{�X����e���鎞�ɌĂԊ֐�
    static void CallBossDamaged() 
    { 
        bossDamageCount++;
        //char buf[256];
        //sprintf_s(buf, " Called BossDamageCount:%d\n", bossDamageCount);
        //OutputDebugStringA(buf);
    }

    // �r�����j�󂳂ꂽ�Ƃ��ɌĂԊ֐�
    static void CallBuildBroken() 
    {
        buildBrokeCount++; 
        //char buf[256];
        //sprintf_s(buf, " Called BuildBrokeCount:%d\n", buildBrokeCount);
        //OutputDebugStringA(buf);
    }

    // �v���C���[�̔�e�����擾����֐�
    static int GetPlayerDamageCount()  { return playerDamageCount; }

    // �r���̔j�󐔂��擾����֐�
    static int GetBuildBrokeCount() { return buildBrokeCount; }

    // �{�X�̔�e�����擾����֐�
    static int GetBossDamageCount() { return bossDamageCount; }

    // �Q�[���̃J�E���g���X�^�[�g���ꂽ��ĂԊ֐�
    static void GameCountStart()
    {
        gameTimerCountStart = true;

        //UI�\��
        ObjectManager::Find("TimerCanvas")->SetActive(true);
        ObjectManager::Find("BossIndicatorCanvas")->SetActive(true);
        ObjectManager::Find("BossCanvas")->SetActive(true);
        ObjectManager::Find("PlayerCanvas")->SetActive(true);
        ObjectManager::Find("MenuCanvas")->SetActive(true);
    }

    // �Q�[�����J�n���ꂽ���ǂ���
    static bool GetGameTimerStart()
    {
        return gameTimerCountStart;
    }

    //�����N�ۑ�
    static void SetRank(int rank)
    {
        _rank = rank;
    }
    //�����N�擾
    static int GetRank() 
    {
        return _rank;
    }

private:
    static inline std::shared_ptr<ItemManager> itemManager_;
    static inline std::shared_ptr<BuildingManager> buildingManager_;

    //------ ���U���g�p�̃J�E���g�ϐ�
    // �v���C���[�̔�e��
    static inline int playerDamageCount = 0;
    // �r���j��
    static inline int buildBrokeCount = 0;
    // �{�X�̔�e��
    static inline int bossDamageCount = 0;

    // �Q�[���̃^�C�}�[�J�E���g�X�^�[�g
    static inline bool gameTimerCountStart = false;

    //�����N�̌��ʕۑ��p
    static inline int _rank = 0;
};

#endif // GAME_MANAGER_H