#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include <string>
#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Math/MathHelper.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Core/ActorManager.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Renderer/ShapeRenderer.h"
#include "Game/Utils/SpawnValidator.h"
#include "Components/CollisionShape/ShapeComponent.h"

#include "Engine/Scene/Scene.h"

class ItemManager
{
public:
    void Initialize()
    {
        InitializeGridAreas(gridX, gridY, stageWidth, stageDepth);
    }

    void Update(float deltaTime)
    {
        for (auto& area : areas)
        {
            area.spawnTimer += deltaTime;
            if (area.currentItemCount < area.maxItemCount && area.spawnTimer > spawnInterval)
            {
                SpawnItemArea(area);
                area.spawnTimer = spawnInterval;
            }
        }
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

    void DecreaseAreaItemCount(const DirectX::XMFLOAT3& pos)
    {
        for (auto& area : areas)
        {
            if (area.Contains(pos))
            {
                area.currentItemCount--;
                break; // 複数マッチしない想定なら break で OK
            }
        }
    }

    void DrawImGuiAreaVisualization()
    {
#ifdef USE_IMGUI

        ImGui::Begin("Item Spawn Areas");

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
            }
            ImGui::PopID();
        }

        ImGui::End();
#endif
    }
private:
    std::vector<Area> areas;

    float spawnInterval = 5.0f;

    void SpawnItemArea(Area& area)
    {
#if 1
        const int maxTries = 10;
        int tryCount = 0;

        while (tryCount < maxTries)
        {
            ++tryCount;

            DirectX::XMFLOAT3 spawnPos = area.GetRandomSpawnPosition();
            Scene* currentScene = Scene::GetCurrentScene();
            if (!currentScene)
            {
                // シーンがなければ処理しない or エラー処理
                return;
            }

            // ActorManager はポインタなので-> でアクセス
            auto item = currentScene->GetActorManager()->CreateAndRegisterActor<PickUpItem>("item", false);            item->SetTempPosition(spawnPos);
            item->Initialize();
            item->PostInitialize();
#if 0
            auto box = item->GetSceneComponentByName("boxComponent");
            if (auto box = std::dynamic_pointer_cast<BoxComponet>(shape))
            {
                if (SpawnValidator::IsAreaFree(box->GetAABB()))
                {
                    SpawnValidator::Register(shared_from_this());

                    //item->GetComponent<SkeltalMeshComponent>()->SetIsVisible(true);
                    item->skeltalMeshComponent->SetIsVisible(true);
                    area.currentItemCount++;
                    return; //成功したのでこの area のループをやめる
                }
            }
#else
            auto shape = item->GetSceneComponentByName("sphereComponent");
            if (auto sphere = std::dynamic_pointer_cast<SphereComponent>(shape))
            {
                if (SpawnValidator::IsAreaFree(sphere->GetAABB()))
                {
                    //SpawnValidator::RegisterAABB(sphere->GetAABB());
                    //item->GetComponent<SkeltalMeshComponent>()->SetIsVisible(true);
                    item->skeltalMeshComponent->SetIsVisible(true);
                    area.currentItemCount++;
                    return; //成功したのでこの area のループをやめる
                }
            }
#endif // 0
            //item->SetValid(false);
            item->SetPendingDestroy();
        }
#else
        SpawnHelper::TrySpawnWithValidation<PickUpItem>(10,
            [&area]() {return area.GetRandomSpawnPosition(); }, [&area](auto item) {area.currentItemCount++; });
#endif
        //DirectX::XMFLOAT3 spawnPos = area.GetRandomSpawnPosition();
        //auto item = ActorManager::CreateAndRegisterActor<PickUpItem>("item", false);
        //item->SetTempPosition(spawnPos);
        //item->Initialize();
        //item->PostInitialize();
        //area.currentItemCount++;
    }

    void InitializeGridAreas(int gridX, int gridY, float stageWidth, float stageDepth)
    {
        areas.clear();
        float cellWidth = stageWidth / gridX;
        float cellDepth = stageDepth / gridY;

        for (int y = 0; y < gridY; ++y)
        {
            for (int x = 0; x < gridX; ++x)
            {
                std::string name = "item_area" + std::to_string(y) + "_" + std::to_string(x);
                DirectX::XMFLOAT2 min = {
                 x * cellWidth - stageWidth * 0.5f,
                 y * cellDepth - stageDepth * 0.5f
                };
                DirectX::XMFLOAT2 max = {
                    (x + 1) * cellWidth - stageWidth * 0.5f,
                    (y + 1) * cellDepth - stageDepth * 0.5f
                };
                Area area;
                area.name = name;
                area.min = min;
                area.max = max;
                area.maxItemCount = 5;
                area.currentItemCount = 0;
                area.spawnTimer = spawnInterval;

                area.color = DirectX::XMFLOAT4(
                    0.3f + 0.5f * (x / static_cast<float>(gridX)),  // R
                    0.3f + 0.5f * (y / static_cast<float>(gridY)),  // G
                    1.0f,                                            // B
                    1.0f
                );

                areas.push_back(area);
            }
        }
    }

    int gridX = 2;
    int gridY = 2;
    float stageWidth = 40.0f;
    float stageDepth = 30.0f;
    //float stageWidth = 70.0f;
    //float stageDepth = 70.0f;
};

#endif