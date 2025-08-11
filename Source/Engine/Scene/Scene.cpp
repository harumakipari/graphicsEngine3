#include "Scene.h"

#include <chrono>

#include "Graphics/Resource/Texture.h"

bool Scene::_update(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    // ���݂�ActorManager�̍X�V����
    if (_current_scene->actorManager_)
    {
        _current_scene->actorManager_->Update(deltaTime);
    }
    // ���݂̃V�[���̍X�V����
    _current_scene->Update(immediateContext, deltaTime);
    // �V�[����؂�ւ���ꍇ�Ƀ����_�����O���X�L�b�v���邽�߂̃t���O
    bool skipRendering = false;
    // ���̃V�[�����ݒ肳��Ă���ꍇ�A�V�[����؂�ւ���
    if (_next_scene)
    {
        // Direct3D11 �̃f�o�C�X�I�u�W�F�N�g���擾
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        immediateContext->GetDevice(device.GetAddressOf());

        // ���݂̃V�[�����u�I���������v�ɕύX
        _current_scene->State(SCENE_STATE::uninitializing);
        // ���݂̃V�[���̌㏈��
        _current_scene->Uninitialize(device.Get());
        // ���݂̃V�[�����u�I���ς݁v�ɕύX
        _current_scene->State(SCENE_STATE::uninitialized);

        // �v�����[�h�ς݂̃V�[��������ꍇ�A���̃V�[���ɃZ�b�g
        if (_preload_scene)
        {
            _next_scene = std::move(_preload_scene);
        }

        // ���̃V�[�����܂�����������Ă��Ȃ��ꍇ�A���������s��
        if (_next_scene->State() < SCENE_STATE::initializing)
        {
            // ���݂̃����_�����O�̃r���[�|�[�g�T�C�Y���擾
            D3D11_VIEWPORT viewport;
            UINT numViewports{ 1 };
            immediateContext->RSGetViewports(&numViewports, &viewport);

            // �V�[���̏�Ԃ��u���������v�ɕύX
            _next_scene->State(SCENE_STATE::initializing);
            // ActorManager �̐���
            _next_scene->actorManager_ = std::make_unique<ActorManager>();
            _next_scene->actorManager_->SetOwnerScene(_next_scene.get());
            // �V�[���̏����������i�f�o�C�X�A��ʃT�C�Y�A�v���p�e�B����n���j
            _next_scene->Initialize(device.Get(), static_cast<UINT64>(viewport.Width), static_cast<UINT64>(viewport.Height), _payload);
            // �V�[���̏�Ԃ��u�������ς݁v�ɕύX
            _next_scene->State(SCENE_STATE::initialized);
        }
        // �V�[�����u�A�N�e�B�u�v�ɐݒ�
        _next_scene->State(SCENE_STATE::active);
        // ���݂̃V�[�������̃V�[���ɐ؂�ւ�
        _current_scene = std::move(_next_scene);
        // �V�[���ύX���Ƀy�C���[�h���N���A�i���̃V�[���ɂ͉e�����Ȃ��j
        _payload.clear();
        //�V�[���̃X�^�[�g
        _current_scene->Start();
        // �V�[�����؂�ւ�������߁A�����_�����O���X�L�b�v
        skipRendering = true;
    }
    return skipRendering;
}

bool Scene::_async_preload_scene(ID3D11Device* device, UINT64 width, UINT height, const std::string& name)
{
    // �������K�؂��`�F�b�N�i��̖��O�△���ȃT�C�Y�̓G���[�j
    _ASSERT_EXPR(name.size() > 0 && width > 0 && height > 0, L"Invalid Argument");

    // ���łɃv�����[�h���̃V�[�������邩�A�񓯊����������s���Ȃ�v�����[�h���Ȃ�
    if (_preload_scene || _future.valid())
    {
        return false;
    }

    // �V�[�����o�^�ς݂ł��邱�Ƃ��m�F�i�Ȃ���΃G���[�j
    _ASSERT_EXPR(_reflections().find(name) != _reflections().end(), L"Not found scene name in _alchemists");

    // �V�����v�����[�h�p�̃V�[�����쐬
    _preload_scene = _reflections().at(name)();

    // �܂��񓯊������ifuture�j���J�n����Ă��Ȃ��ꍇ
    if (!_future.valid())
    {
        // �v�����[�h�p�V�[���̏�Ԃ��u�ҋ@���iawaiting�j�v�Ȃ�񓯊��Ń��[�h�J�n
        if (_preload_scene->State() == SCENE_STATE::awaiting)
        {
            _future = std::async(std::launch::async, [device, name, width, height]() {
                // ActorManager �̐���
                _preload_scene->actorManager_ = std::make_unique<ActorManager>();
                _preload_scene->actorManager_->SetOwnerScene(_preload_scene.get());
                _preload_scene->State(SCENE_STATE::initializing);// ��Ԃ��u���������v�ɐݒ�
                bool success = _preload_scene->Initialize(device, width, height, {});
                _preload_scene->State(SCENE_STATE::initialized);// ��Ԃ��u�����������v�ɐݒ�
                return success;
                });
        }
    }
    // �V�[���̏�Ԃ��u�������J�n�v���i��ł���� true
    return _preload_scene->State() > SCENE_STATE::initializing;
}


