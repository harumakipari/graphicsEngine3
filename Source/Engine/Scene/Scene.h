#pragma once

#include <d3d11_1.h>
#include <wrl.h>

#include <unordered_map>
#include <string>
#include <future>

#include <functional>

#include "Graphics/Resource/Texture.h"

//kuroda
#include "Widgets/ObjectManager.h"
#include "Widgets/Events/EventSystem.h"

#include "Core/ActorManager.h"

//�V�[���̏�Ԃ�\���񋓌^
enum class SCENE_STATE
{
    awaiting,			// �ҋ@��
    initializing,		// ��������
    initialized,		// ����������
    active,				// �A�N�e�B�u
    uninitializing,		// �I��������
    uninitialized		// ��������
};

// �V�[���N���X�̒�`
class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;
    Scene(const Scene&) = delete;
    Scene& operator =(const Scene&) = delete;
    Scene(Scene&&) noexcept = delete;
    Scene& operator =(Scene&&) noexcept = delete;

    // �V�[���̌��݂̏�Ԃ��擾
    SCENE_STATE State() const
    {
        return state_;
    }

    // Scene* scene = Scene::GetCurrrentScene()

    // ���݂̃V�[���ւ̃A�N�Z�X�֐�
    static Scene* GetCurrentScene()
    {
        return _current_scene.get();
    }

    ObjectManager objectManager;//kuroda

    // ActorManager �̎擾
    const ActorManager* GetActorManager() const { return actorManager_.get(); }
    ActorManager* GetActorManager() { return actorManager_.get(); }
    //std::unique_ptr<ActorManager>& GetActorManager() { return actorManager_; }
private:
    // �������z�֐��F�V�[���̏�����
    virtual bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) = 0;
    //�V�[�����n�܂����Ƃ�
    virtual void Start() {};
    // �������z�֐��F�V�[���̍X�V
    virtual void Update(ID3D11DeviceContext* immediate_context, float delta_time) = 0;
    // �������z�֐��F�V�[���̕`��
    virtual void Render(ID3D11DeviceContext* immediate_context, float delta_time) = 0;
    // ���z�֐��F�V�[���̏I������
    virtual bool Uninitialize(ID3D11Device* device)
    {
        ReleaseAllTextures();
        bool completelyRegenerate = true;
        return completelyRegenerate;
    }
    // ���z�֐��F�E�B���h�E�T�C�Y�ύX���̏���
    virtual bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
    {
        return true;
    }
    // �V�[���̏�Ԃ��Ǘ����邽�߂̕ϐ�
    std::atomic<SCENE_STATE> state_ = SCENE_STATE::awaiting;
    // �V�[���̏�Ԃ�ݒ�
    void State(SCENE_STATE state)
    {
        state_ = state;
    }

    virtual void DrawGui() {}

public:
    // �e���v���[�g�֐��F�V�[���̏�����
    //template<class _boot_scene>
    static bool _boot(ID3D11Device* device, std::string name, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
    {
        //�V�[���� _reflections() �ɓo�^����Ă��邩�`�F�b�N
        _ASSERT_EXPR(_reflections().find(name) != _reflections().end(), L"Not found scene name in reflections");
        //�o�^����Ă���V�[���� �t�@�N�g���֐��istd::function�j �����s���� std::unique_ptr<scene> ���擾�B
        _current_scene = _reflections().at(name)();
        _current_scene->State(SCENE_STATE::initializing);
        // ActorManager �̐���
        _current_scene->actorManager_ = std::make_unique<ActorManager>();
        _current_scene->actorManager_->SetOwnerScene(_current_scene.get());

        _current_scene->Initialize(device, width, height, props);
        if (!_current_scene->GetActorManager())
        {
            OutputDebugStringA("actorManager_ is nullptr immediately after creation!\n");
        }
        else
        {
            OutputDebugStringA("actorManager_ is properly created.\n");
        }
        _current_scene->State(SCENE_STATE::initialized);
        _current_scene->State(SCENE_STATE::active);
        _current_scene->Start();

        return true;
    }
    // �E�B���h�E�T�C�Y�ύX���̏���
    static bool _on_size_changed(ID3D11Device* device, UINT64 width, UINT height)
    {
        return _current_scene->OnSizeChanged(device, width, height);
    }

private:
    // �V�[���̍X�V
    static bool _update(ID3D11DeviceContext* immediateContext, float deltaTime);
    // �V�[���̕`��
    static void _render(ID3D11DeviceContext* immediateContext, float deltaTime)
    {
        _current_scene->Render(immediateContext, deltaTime);
    }
    // GUI�̕`��
    static void _drawGUI()
    {
        //if (_current_scene->actorManager_)
        //{
        //    _current_scene->actorManager_->DrawImGuiAllActors();
        //}
        _current_scene->DrawGui();
    }

    // �V�[���̏I������
    static bool _uninitialize(ID3D11Device* device)
    {
        // �񓯊������ifuture�j���L���Ȃ�A������҂�
        if (_future.valid())
        {
            _future.wait();
        }
        // actorManager �̔j������
        if (_current_scene->actorManager_)
        {
            _current_scene->actorManager_->ClearAll();
        }
        // ���݂̃V�[���̌㏈�������s
        _current_scene->Uninitialize(device);
        // ���݂̃V�[�������Z�b�g�inullptr�ɂ���j
        _current_scene.reset();

        return true;
    }
    // �e���v���[�g�֐��F�V�[���̓o�^
    template<class T>
    static std::string _enroll()
    {
        // �V�[���̃N���X�����擾�i�^��񂩂當����ɕϊ��j
        std::string className = typeid(T).name();
        // ���O��ԂȂǂ��܂܂�Ă���ꍇ�A�Ō�̕����݂̂��擾
        className = className.substr(className.find_last_of(" ") + 1, className.length());
        // ���łɓo�^�ς݂̃N���X�łȂ����Ƃ��m�F
        _ASSERT_EXPR(_reflections().find(className) == _reflections().end(), L"'reflections' already has a scene with 'className'");
        // �V�[���̐����֐���o�^�i�t�@�N�g���[�֐���ۑ��j
        _reflections().emplace(std::make_pair(className, []() {return std::make_unique<T>(); }));
        return className;//�o�^�����N���X����Ԃ�
    }
    // Framework �N���X�� friend �Ɏw��iFramework ���� private/protected �ɃA�N�Z�X�ł���j
    friend class Framework;

protected:
    // �V�[���̃v�����[�h���������Ă��邩�m�F
    static bool _has_finished_preloading()
    {
        // �v�����[�h���̃V�[��������A���񓯊������ifuture�j���L���Ȃ�`�F�b�N
        if (_preload_scene && _future.valid())
        {
            // future �̏�Ԃ�񓯊��Ŋm�F�i�����Ɍ��ʂ������邩�j
            if (_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                // �v�����[�h�V�[���̏�Ԃ��������������Ă���ΐ���
                if (_preload_scene->State() >= SCENE_STATE::initialized)
                {
                    bool success = _future.get();// future �̌��ʂ��擾�i�擾���Ȃ��� future �͖������ł��Ȃ��j
                    return true;
                }
            }
            return false;
        }
        // �v�����[�h�V�[�����Ȃ��A�܂��͂��łɏ��������Ȃ� true
        return true;
    }
public:
    // �V�[���̑J��
    static bool _transition(const std::string& name, const std::unordered_map<std::string, std::string>& props)
    {
        //�񓯊��������������Ă��邩�m�F�i�������Ȃ�V�[���J�ڂ��Ȃ��j
        if (!_async_wait())
        {
            return false;
        }
        // �V�[�������o�^����Ă��邩�m�F�i�o�^����Ă��Ȃ��ƃG���[�j
        _ASSERT_EXPR(_reflections().find(name) != _reflections().end(), L"Not found scene name in reflections");
        // ���ɑJ�ڂ���V�[����ݒ�i�t�@�N�g���[�֐��ŐV�����V�[�����쐬�j
        _next_scene = _reflections().at(name)();
        // �V�[���ɓn���f�[�^�i�v���p�e�B�j��ݒ�
        _payload = props;

        //UI�̏�񏉊����i�ǉ��j
        EventSystem::Reset();

        return true;// �V�[���J�ڂ�����
    }
    // �񓯊��ŃV�[�����v�����[�h
    static bool _async_preload_scene(ID3D11Device* device, UINT64 width, UINT height, const std::string& name);


private:
    // �񓯊������̊�����ҋ@
    static bool _async_wait()
    {
        // �����񓯊������ifuture�j���L���Ȃ珈����҂�
        if (_future.valid())
        {
            // `wait_for(0�b)` �Ŕ񓯊������������������m�F
            if (_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                bool success = _future.get();// �񓯊������̌��ʁitrue/false�j���擾�i�擾���Ȃ��� future �͖������ł��Ȃ��j
                // �����������s���Ă�����G���[���o��
                _ASSERT_EXPR(success, L"The scene initioalization process using asynchronous processing hasa failed.");
            }
            else
            {
                return false;// �񓯊��������܂��������Ă��Ȃ�
            }
        }
        return true;// �񓯊��������I����Ă���
    }


    //The compiler version must be set to C++20 or higher to use the reserved word 'static inline'.
    // �u�V�[���� �� �V�[�������֐��v �̃}�b�v�ɃA�N�Z�X�ł���֐�
    static inline std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>>& _reflections()
    {
        static std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>> reflections;
        return reflections;
    }


    static inline std::unique_ptr<Scene> _next_scene;//���̃V�[��
    static inline std::unique_ptr<Scene> _current_scene;//���݂̃V�[��
    static inline std::unique_ptr<Scene> _preload_scene;//�v�����[�h�ς݂̃V�[��
    // �񓯊��ŃV�[�������[�h���邽�߂� future
    static inline std::future<bool> _future;
    // �V�[���ɓn���ǉ����i�L�[�ƒl�̃}�b�v�j
    static inline std::unordered_map<std::string, std::string> _payload;

protected:
    // ����: �ÓI�����������̖���������邽�߂̎d�g��
    //CAUSION: Static Initialization Order Fiasco
    template<class T>
    struct Autoenrollment
    {//Scene::_reflections() �� �����I�ɓo�^
        Autoenrollment()
        {
            // �R���X�g���N�^�Ŏ����I�ɃV�[����o�^
            Scene::_enroll<T>();
        }
    };

private:
    std::unique_ptr<ActorManager> actorManager_;
};


