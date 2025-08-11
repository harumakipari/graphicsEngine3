#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <memory>
#include <string>
#include "Graphics/Renderer/SceneRenderer.h"
#include "Core/Actor.h"
class CollisionSystem;


class World
{
public:
    World() = default;
    virtual ~World() {};

    // �X�V����
    void Tick(float deltaTime);

    // �`�揈��
    void Render(ID3D11DeviceContext* immediateContext);

    // �e�̕`�揈��
    void CastShadowRender(ID3D11DeviceContext* immediateContext);

    // ImGui��`�悷��
    void DrawGUI();
    
    void Clear();

    // �A�N�^�[�𖼑O�t���ō쐬�E�o�^����i�����A�N�^�[�����݂���ꍇ�͌x������j
    template<class T>
    std::shared_ptr<T> SpawnActor(const std::string& actorName)
    {
        auto findByName = [&actorName](const std::shared_ptr<Actor>& actor)
            {
                return actor->GetName() == actorName;
            };
        // ���O����v����A�N�^�[��T��
        std::vector<std::shared_ptr<Actor>>::iterator it = std::find_if(allActors_.begin(), allActors_.end(), findByName);

        // �����̃A�N�^�[�����łɑ��݂��Ă�����x��
        _ASSERT_EXPR(it == allActors_.end(), L"���� actor �̖��O�͊��Ɏg�p����Ă��܂��B");
        std::shared_ptr<T> newActor = std::make_shared<T>(actorName);
        allActors_.push_back(newActor);

        newActor->Initialize();
        return newActor;
    }


    // ���O����A�N�^�[���擾�i�L���b�V���t�������j
    std::shared_ptr<Actor> FindActorByName(const std::string& actorName)
    {
        // �L���b�V���ɂ���΂����Ԃ�
        auto cached = actorCacheByName_.find(actorName);
        if (cached != actorCacheByName_.end())
        {
            return cached->second;
        }

        // �Ȃ���ΑS�A�N�^�[����T��
        auto found = std::find_if(allActors_.begin(), allActors_.end(),
            [&actorName](const std::shared_ptr<Actor>& actor) {
                return actor->GetName() == actorName;
            });

        // ���������ꍇ�̓L���b�V�����ĕԂ�
        if (found != allActors_.end()) {
            actorCacheByName_[actorName] = *found;
            return *found;
        }

        // ������Ȃ�����
        return nullptr;
    }

private:
    // �A�N�^�[������A�N�^�[�ւ̃|�C���^�������Ɏ擾���邽�߂̃L���b�V���B
    // ���O��������Ȃ��ꍇ�̓A�N�^�[���X�g���������A���ʂ����̃L���b�V���ɕۑ�����B
    std::unordered_map<std::string, std::shared_ptr<Actor>> actorCacheByName_;
    // ���ݑ��݂��Ă��邷�ׂẴA�N�^�[
    std::vector<std::shared_ptr<Actor>> allActors_;
    // �����_���[
    SceneRenderer renderer;
};