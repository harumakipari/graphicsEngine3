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

    // プレイヤーが被弾する時に呼ぶ関数
    static void CallPlayerDamaged() 
    { 
        playerDamageCount++; 
        //char buf[256];
        //sprintf_s(buf, " Called PlayerDamageCount:%d\n", playerDamageCount);
        //OutputDebugStringA(buf);
    }

    // ボスが被弾する時に呼ぶ関数
    static void CallBossDamaged() 
    { 
        bossDamageCount++;
        //char buf[256];
        //sprintf_s(buf, " Called BossDamageCount:%d\n", bossDamageCount);
        //OutputDebugStringA(buf);
    }

    // ビルが破壊されたときに呼ぶ関数
    static void CallBuildBroken() 
    {
        buildBrokeCount++; 
        //char buf[256];
        //sprintf_s(buf, " Called BuildBrokeCount:%d\n", buildBrokeCount);
        //OutputDebugStringA(buf);
    }

    // プレイヤーの被弾数を取得する関数
    static int GetPlayerDamageCount()  { return playerDamageCount; }

    // ビルの破壊数を取得する関数
    static int GetBuildBrokeCount() { return buildBrokeCount; }

    // ボスの被弾数を取得する関数
    static int GetBossDamageCount() { return bossDamageCount; }

    // ゲームのカウントがスタートされたら呼ぶ関数
    static void GameCountStart()
    {
        gameTimerCountStart = true;

        //UI表示
        ObjectManager::Find("TimerCanvas")->SetActive(true);
        ObjectManager::Find("BossIndicatorCanvas")->SetActive(true);
        ObjectManager::Find("BossCanvas")->SetActive(true);
        ObjectManager::Find("PlayerCanvas")->SetActive(true);
        ObjectManager::Find("MenuCanvas")->SetActive(true);
    }

    // ゲームが開始されたかどうか
    static bool GetGameTimerStart()
    {
        return gameTimerCountStart;
    }

    //ランク保存
    static void SetRank(int rank)
    {
        _rank = rank;
    }
    //ランク取得
    static int GetRank() 
    {
        return _rank;
    }

private:
    static inline std::shared_ptr<ItemManager> itemManager_;
    static inline std::shared_ptr<BuildingManager> buildingManager_;

    //------ リザルト用のカウント変数
    // プレイヤーの被弾回数
    static inline int playerDamageCount = 0;
    // ビル破壊数
    static inline int buildBrokeCount = 0;
    // ボスの被弾回数
    static inline int bossDamageCount = 0;

    // ゲームのタイマーカウントスタート
    static inline bool gameTimerCountStart = false;

    //ランクの結果保存用
    static inline int _rank = 0;
};

#endif // GAME_MANAGER_H