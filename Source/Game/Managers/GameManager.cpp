#include "GameManager.h"

#include "ItemManager.h"
#include "BuildingManager.h"


void GameManager::Initialize()
{
    {
        itemManager_ = std::make_shared<ItemManager>();
        itemManager_->Initialize();
    }
    {
#if 1
        buildingManager_ = std::make_shared<BuildingManager>();
        buildingManager_->Initialize();
#endif
    }

    // リザルト用の変数をリセットする
    playerDamageCount = 0;
    buildBrokeCount = 0;
    bossDamageCount = 0;
    gameTimerCountStart = false;
    char buf[256];
    sprintf_s(buf, "Game Start :PlayerDamageCount:%d, BuildBrokeCount:%d, BossDamageCount:%d\n", playerDamageCount, buildBrokeCount, bossDamageCount);
    OutputDebugStringA(buf);
}

void GameManager::Update(float deltaTime)
{
    itemManager_->Update(deltaTime);
    buildingManager_->Update(deltaTime);
}

void GameManager::DrawGUI()
{
    itemManager_->DrawImGuiAreaVisualization();
    buildingManager_->DrawImGuiAreaVisualization();

}

void GameManager::DebugRender(ID3D11DeviceContext* immediateContext)
{
    itemManager_->DebugRender(immediateContext);
    buildingManager_->DebugRender(immediateContext);
}

void GameManager::Finalize()
{
    char buf[256];
    sprintf_s(buf, "Game Finish :PlayerDamageCount:%d, BuildBrokeCount:%d\n", playerDamageCount, buildBrokeCount);
    OutputDebugStringA(buf);

    itemManager_->Finalize();
    buildingManager_->Finalize();
}
