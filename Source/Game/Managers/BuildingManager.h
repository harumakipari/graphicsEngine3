#ifndef BUILDING_MANAGER_H
#define BUILDING_MANAGER_H

#include <string>
#include <vector>
#include <memory>

#include <DirectXMath.h>
#include <d3d11.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Math/MathHelper.h"
#include "Game/Actors/Stage/Building.h"
#include "Game/Actors/Stage/BossBuilding.h"
#include "Core/ActorManager.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Renderer/ShapeRenderer.h"
#include "Game/Utils/SpawnValidator.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/CollisionShape/CollisionComponent.h"
#include "Game/Utils/TiledMapLoader.h"
#include "Game/Utils/ShockWaveTargetRegistry.h"
#include "Components/Audio/AudioSourceComponent.h"

#include "Engine/Scene/Scene.h"

class BuildingManager
{
public:
    void Initialize()
    {
        InitializeGridAreas(gridX, gridY, stageWidth, stageDepth);

        map = TiledMapLoader::LoadFromFile("SpawnData.json", "./Data/SpawnData/");
        audio = std::make_unique<StandaloneAudioSource>(L"./Data/Sound/SE/bill_spawn.wav");
        audio->SetVolume(3.0f);
    }

    //ボスの着地点
    XMFLOAT3 GetMovePos(const std::string& layerName)
    {
        Area& area = areas[0];

        const TileLayer& layer = map.layers[layerName];
        int i = 0;
        for (int row = 0; row < buildingLayout.size(); ++row)
        {
            for (int col = 0; col < buildingLayout[row].size(); ++col)
            {
                int id = layer.tiles[i++];
                if (id != 1) continue;
                return area.GetGridSpawnPosition(col, row);
            }
        }
        return { 0,0,0 };
    }

    void Spawn(const std::string& layerName, bool billDestroy = true)
    {
        if (billDestroy)
        {
            for (auto& actorPtr : ShockWaveTargetRegistry::GetTargets())
            {
                if (auto building = std::dynamic_pointer_cast<Building>(actorPtr))
                {
                    // 移動
                    auto pos = building->GetPosition();
                    pos.y -= 10.0f;
                    building->SetPosition(pos);
                    building->SetPendingDestroy();
                }
            }
        }
        //ShockWaveTargetRegistry::GetTargets().clear();

        Area& area = areas[0];

        // 音を再生
        audio->Play();

        const TileLayer& layer = map.layers[layerName];
        int i = 0;
        for (int row = 0; row < buildingLayout.size(); ++row)
        {
            for (int col = 0; col < buildingLayout[row].size(); ++col)
            {
                int id = layer.tiles[i++];
                if (id == 0) continue;

                //普通ビル
                if (id == 2)
                {
                    SpawnItemArea(area, col, row);
                }

                //爆発ビル
                if (id == 3)
                {
                    SpawnBossBuildArea(area, col, row);
                }
            }
        }
    }
    // GameManager::GetBuildingManager()->SwitchToBossBuilding();
    // 通常ビルをボスのビルに差し替える処理
    void SwitchToBossBuilding()
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (auto& actorPtr : ShockWaveTargetRegistry::GetTargets())
        {
            if (auto building = std::dynamic_pointer_cast<Building>(actorPtr))
            {
                if (building->GetDestroy())
                {// ビルが壊れていたら生成しない
                    continue;
                }
                auto pos = building->GetPosition();
                building->SetPosition(pos);
                building->SetPendingDestroy();
                Transform test2Tr(pos, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
                Scene* currentScene = Scene::GetCurrentScene();
                if (!currentScene)
                {
                    return;
                }
                auto testCollision2 = currentScene->GetActorManager()->CreateAndRegisterActorWithTransform<BossBuilding>("bossBuilding", test2Tr);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        char buf[256];
        sprintf_s(buf, "SwitchToBossBuilding time: %lld ms\n", duration.count());
        OutputDebugStringA(buf);
    }

    void Update(float deltaTime)
    {
        if (!initialized)
        {
            if (!areas.empty())
            {
                //Spawn("NormalBill2");

                initialized = true;
            }
        }

#if 0
        for (auto& area : areas)
        {
            area.spawnTimer += deltaTime;
            if (area.currentItemCount < area.maxItemCount && area.spawnTimer > spawnInterval)
            {
                SpawnItemArea(area);
                area.spawnTimer = 0.0f;
            }
        }

#endif // 0
    }

    void DrawImGuiAreaVisualization()
    {
#ifdef USE_IMGUI

        ImGui::Begin("Building Spawn Areas");

        ImVec2 canvasSize = ImVec2(400, 400);
        ImGui::Text("Spawn Areas Visualization");
        ImGui::InvisibleButton("canvas", canvasSize);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetItemRectMin();


        auto WorldToCanvas = [&](float x, float z) -> ImVec2
            {
                float cx = ((x + stageWidth * 0.5f) / stageWidth) * canvasSize.x;
                float cz = ((z) / stageDepth) * canvasSize.y;
                return ImVec2(origin.x + cx, origin.y + canvasSize.y - cz);
            };

        for (const auto& area : areas)
        {
            ImVec2 topLeft = WorldToCanvas(area.min.x, area.max.y);
            ImVec2 bottomRight = WorldToCanvas(area.max.x, area.min.y);

            // DirectX::XMFLOAT4 -> ImU32 変換
            ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(ImVec4(area.color.x, area.color.y, area.color.z, area.color.w));

            // 枠線の色は固定でもよい
            ImU32 borderColor = IM_COL32(50, 50, 200, 200);

            drawList->AddRectFilled(topLeft, bottomRight, fillColor);
            drawList->AddRect(topLeft, bottomRight, borderColor, 0.0f, 0, 2.0f);

            // ラベル描画
            std::string label = area.name + "\nItems: " + std::to_string(area.currentItemCount);
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            ImVec2 textPos = ImVec2((topLeft.x + bottomRight.x) * 0.5f - textSize.x * 0.5f,
                (topLeft.y + bottomRight.y) * 0.5f - textSize.y * 0.5f);

            drawList->AddText(textPos, IM_COL32_WHITE, label.c_str());
        }

        if (ImGui::SliderInt("Grid X", &gridX, 1, 10) ||
            ImGui::SliderInt("Grid Y", &gridY, 1, 10) ||
            ImGui::SliderFloat("Stage Width", &stageWidth, 1.0f, 100.0f) ||
            ImGui::SliderFloat("Stage Depth", &stageDepth, 1.0f, 100.0f))
        {
            InitializeGridAreas(gridX, gridY, stageWidth, stageDepth);
        }

        for (size_t i = 0; i < areas.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::CollapsingHeader(areas[i].name.c_str()))
            {
                ImGui::InputInt("Max Item Count", &areas[i].maxItemCount);
                ImGui::Text("Current Item Count: %d", areas[i].currentItemCount);

                ImGui::Text("Min: (%.1f, %.1f)", areas[i].min.x, areas[i].min.y);
                ImGui::Text("Max: (%.1f, %.1f)", areas[i].max.x, areas[i].max.y);

                ImGui::Text("Grid: %d cols x %d rows", areas[i].numCols, areas[i].numRows);
            }
            ImGui::PopID();
        }

        ImGui::End();
#endif
    }

    void DebugRender(ID3D11DeviceContext* immediateContext)
    {
        for (auto& area : areas)
        {
            const float debugHeight = 0.01f;
            const float thickness = 1.0f;
            DirectX::XMFLOAT3 size = {
                (area.max.x - area.min.x),
                thickness,
                (area.max.y - area.min.y)
            };
            DirectX::XMFLOAT3 center = {
                (area.min.x + area.max.x) * 0.5f,
                debugHeight,
                (area.min.y + area.max.y) * 0.5f
            };

            ShapeRenderer::Instance().DrawBox(
                immediateContext,
                center,
                { 0, 0, 0 }, // 回転なし
                size,
                area.color
            );
        }
    }


    void Finalize()
    {
        areas.clear();
    }
private:
    void InitializeGridAreas(int gridX, int gridY, float stageWidth, float stageDepth)
    {
        areas.clear();

        float cellWidth = stageWidth / gridX;
        float cellDepth = stageDepth / gridY;
        float startX = -stageWidth * 0.5f;
        float startZ = -stageDepth * 0.5f;
        for (int y = 0; y < gridY; ++y)
        {
            for (int x = 0; x < gridX; ++x)
            {
                std::string name = "build_area" + std::to_string(y) + "_" + std::to_string(x);
                float minX = startX + x * cellWidth;
                float maxX = startX + (x + 1) * cellWidth;

                float minZ = startZ + y * cellDepth;
                float maxZ = startZ + (y + 1) * cellDepth;

                DirectX::XMFLOAT2 min = { minX, minZ };
                DirectX::XMFLOAT2 max = { maxX, maxZ };

                Area area;
                area.name = name;
                area.min = min;
                area.max = max;
                area.maxItemCount = 999;
                area.ComputeGridFromAreaSize();
                area.currentItemCount = 0;
                //area.spawnTimer = spawnInterval;
                area.spawnTimer = 0.0f;
                //area.numCols = 15;
                //area.numRows = 20;

                area.ResetGrid();

                area.color = DirectX::XMFLOAT4
                (
                    1.0f,                                           // R
                    0.3f + 0.5f * (x / static_cast<float>(gridX)),  // G
                    0.3f + 0.5f * (y / static_cast<float>(gridY)),  // B
                    1.0f
                );

                areas.push_back(area);
            }
        }
    }


    void SpawnItemArea(Area& area, int col, int row);

    void SpawnBossBuildArea(Area& area, int col, int row);
private:
    bool initialized = false;
#if 0
    std::vector<std::vector<int>> buildingLayout = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

#endif // 0

    //std::vector<std::vector<int>> buildingLayout = {
    //  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    //  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
    //};

    std::vector<std::vector<int>> buildingLayout =
    {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    TileMap map;

    std::vector<Area> areas;

    float spawnInterval = 5.0f;


    int gridX = 1;
    int gridY = 1;
    float stageWidth = 42.0f;
    float stageDepth = 32.0f;

    std::unique_ptr<StandaloneAudioSource> audio = nullptr;
};

#endif //BUILDING_MANAGER_H