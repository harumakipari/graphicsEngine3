#include "Framework.h"

#include "Graphics/Core/Shader.h"
#include <dxgi1_3.h>
#include <memory>
#include "Graphics/Resource/Texture.h"

#include "Engine/Scene/SceneRegistry.h"
#include "Graphics/Core/RenderState.h"

#include "Engine/Input/InputSystem.h"
#include "Graphics/Renderer/ShapeRenderer.h"
#include "../../Components/Audio/AudioSourceComponent.h"


//�R���X�g���N�^�F�E�B���h�E�n���h�����󂯎���ď�����
Framework::Framework(HWND hwnd, BOOL fullscreen) : hwnd(hwnd), fullscreenMode(fullscreen), windowed_style(static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE)))
{
#ifndef _DEBUG
    fullscreenMode = true;
#endif
    Graphics::Initialize(hwnd, fullscreenMode);
    InputSystem::Initialize();
    RenderState::Initialize();
    Audio::Initialize();
}

bool Framework::Initialize()
{
    ////�f�o�C�X�E�f�o�C�X�R���e�N�X�g�E�X���b�v�`�F�[���̍쐬
    ID3D11Device* device = Graphics::GetDevice();
    if (!device) {
        assert("ModelComponent Error: device is null\n");
    }

    // ShapeRenderer��������
    ShapeRenderer::Instance().Initalize(device);

    // SCENE_TRANSITION
    //Scene::_boot(device, "MainScene", SCREEN_WIDTH, SCREEN_HEIGHT, {});
    Scene::_boot(device, "BootScene", SCREEN_WIDTH, SCREEN_HEIGHT, {});


    return true;
}

bool Framework::Update(float deltaTime/*Elapsed seconds from last frame*/)
{
    //�f�o�C�X�R���e�N�X�g���擾
    ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();

    //�I�[�f�B�I�X�V
    Audio::Update(deltaTime);

    // SCENE_TRANSITION
    bool skipRendering = Scene::_update(immediateContext, deltaTime * timeScale);

#ifdef USE_IMGUI
    //ImGui::Begin("ImGUI");

    Scene::_drawGUI();

    /*ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#if 0
    ImGui::Text("Video memory usage %d MB", video_memory_usage());
#endif
    ImGui::Text("ALT+ENTER to change window mode");

    ImGui::End();*/
#endif

    //gameManager->UpdateAll(elapsed_time);
    //if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
    //{
    //    Graphics& graphics = Graphics::Instance();
    //    graphics.StylizeWindow(hwnd, !graphics.fullscreenMode);
    //}

    InputSystem::Update(deltaTime);


    return skipRendering;


}

void Framework::Render(float elapsed_time/*Elapsed seconds from last frame*/, bool skipRendering)
{
    HRESULT hr{ S_OK };

    //�f�o�C�X�R���e�N�X�g���擾
    ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();
    // �T���v���[�X�e�[�g��ݒ�
    RenderState::SetSamplerState(immediateContext);

    ID3D11RenderTargetView* nullRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    immediateContext->OMSetRenderTargets(_countof(nullRenderTargetViews), nullRenderTargetViews, 0);
    ID3D11ShaderResourceView* nullShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    immediateContext->VSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);
    immediateContext->PSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);

    //// ��ʂ�����������i�F���w�肵�ă����_�[�^�[�Q�b�g���N���A�j
    // ��ʃN���A
    Graphics::Clear(0.2f, 0.2f, 0.2f, 0.0f);

    // �����_�[�^�[�Q�b�g�ݒ�
    Graphics::SetRenderTargets();


    // SCENE_TRANSITION
    if (!skipRendering)
    {
        Scene::_render(immediateContext, elapsed_time);
        //gameManager->GenerateOutputAll();
    }

#if 0
#ifdef USE_IMGUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

    //UINT sync_interval{ 0 };
    //swap_chain->Present(sync_interval, 0);
#endif


}

bool Framework::Uninitialize()
{
    ID3D11Device* device = Graphics::GetDevice();

    Audio::ClearAll();

    //gameManager->UninitAll();
    // SCENE_TRANSITION
    Scene::_uninitialize(device);
    return true;
}

Framework::~Framework()
{
    ReleaseAllTextures();
}
