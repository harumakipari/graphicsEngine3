#include "World.h"
#include "Core/Actor.h"

void World::Tick(float deltaTime)
{
    // �S�A�N�^�[��Update�������Ăяo���iRootComponent��OwnedComponent�j
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

// �`�揈��
void World::Render(ID3D11DeviceContext* immediateContext)
{
    //renderer.RenderOpaque(immediateContext, allActors_);
    //renderer.RenderMask(immediateContext, allActors_);
    //renderer.RenderBlend(immediateContext, allActors_);
}

// �e�̕`�揈��
void World::CastShadowRender(ID3D11DeviceContext* immediateContext)
{
    renderer.CastShadowRender(immediateContext, allActors_);
}



// ImGui��`�悷��
void World::DrawGUI()
{
#ifdef USE_IMGUI
    // �S�Ă� actor �� ImGui ��`�悷��
    {
        // ��ʃT�C�Y���擾
        ImGuiIO& io = ImGui::GetIO();
        float windowWidth = io.DisplaySize.x * 0.25f;
        float windowHeight = io.DisplaySize.y;

        // ���̃E�B���h�E�̈ʒu�ƃT�C�Y���w��
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - windowWidth, 0));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

        // �t���O�����ČŒ�\���Ɂi�T�C�Y�ύX�Ȃǂ��֎~�������ꍇ�j
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
    // �o�^�ς݃A�N�^�[�ƃL���b�V�������ׂăN���A����
    {
        allActors_.clear();
        actorCacheByName_.clear();
    }
}