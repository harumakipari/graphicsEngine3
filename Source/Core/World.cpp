#include "World.h"
#include "Core/Actor.h"

void World::Tick(float deltaTime)
{
    // 全アクターのUpdate処理を呼び出す（RootComponentとOwnedComponent）
    {
        for (std::shared_ptr<Actor>& actor : allActors_)
        {
            //for (std::shared_ptr<SceneComponent>& component : actor->ownedSceneComponents_)
            for (std::shared_ptr<Component>& component : actor->ownedSceneComponents_)
            {
                component->Tick(deltaTime);
            }
            if (actor->rootComponent_)
            {
                actor->rootComponent_->UpdateComponentToWorld();
            }
            actor->Update(deltaTime);
        }
    }
}

// 描画処理
void World::Render(ID3D11DeviceContext* immediateContext)
{
    //renderer.RenderOpaque(immediateContext, allActors_);
    //renderer.RenderMask(immediateContext, allActors_);
    //renderer.RenderBlend(immediateContext, allActors_);
}

// 影の描画処理
void World::CastShadowRender(ID3D11DeviceContext* immediateContext)
{
    renderer.CastShadowRender(immediateContext, allActors_);
}



// ImGuiを描画する
void World::DrawGUI()
{
#ifdef USE_IMGUI
    // 全ての actor の ImGui を描画する
    {
        // 画面サイズを取得
        ImGuiIO& io = ImGui::GetIO();
        float windowWidth = io.DisplaySize.x * 0.25f;
        float windowHeight = io.DisplaySize.y;

        // 次のウィンドウの位置とサイズを指定
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - windowWidth, 0));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

        // フラグをつけて固定表示に（サイズ変更などを禁止したい場合）
        ImGui::Begin("Actor Inspector", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse
        );

        for (const auto& actor : allActors_) 
        {
            actor->DrawImGuiInspector();
        }

        ImGui::End();
    }
#endif
}

void World::Clear()
{
    // 登録済みアクターとキャッシュをすべてクリアする
    {
        allActors_.clear();
        actorCacheByName_.clear();
    }
}