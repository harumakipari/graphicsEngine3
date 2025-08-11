#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "Engine/Utility/Win32Utils.h"
#include "Engine/Utility/Timer.h"


#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#include "../External/imgui//imgui_internal.h"
#include "../External/imgui//imgui_impl_dx11.h"
#include "../External/imgui//imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif


#include <d3d11_1.h>
//COMオブジェクトをComPtrスマートポインターテンプレートを使った変数宣言に変更する
#include <wrl.h>
#include <dxgi1_6.h>

#define ENABLE_DIRECT2D
#ifdef ENABLE_DIRECT2D
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#endif


//スマートポインタを使う
#include <memory>

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"

#include "Graphics/Sprite/Sprite.h"
#include "Graphics/Sprite/SpriteBatch.h"
#include "Graphics/Resource/GeometricPrimitive.h"

CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
CONST LPCWSTR APPLICATION_NAME{ L"X3DGP" };

class Framework
{
public:
    //タイムスケール（ヒットストップやポーズでいじる）
    static inline const float defaultTimeScale = 1.0f;
    static inline float timeScale = 1.0f;


    CONST HWND hwnd;


    Framework(HWND hwnd,BOOL fullscreen);
    ~Framework();

    Framework(const Framework&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework(Framework&&) noexcept = delete;
    Framework& operator=(Framework&&) noexcept = delete;

    int run()
    {
        MSG msg{};

        if (!Initialize())
        {
            return 0;
        }

#ifdef USE_IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(Graphics::GetDevice(), Graphics::GetDeviceContext());
        ImGui::StyleColorsDark();
#endif

        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                tictoc.tick();
                calculate_frame_stats();

                // SCENE_TRANSITION
#ifdef USE_IMGUI
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
#endif

                bool skipRendering = Update(tictoc.time_interval());
                
                Render(tictoc.time_interval(), skipRendering);
                //Render(tictoc.time_interval());

#ifdef USE_IMGUI
                ImGui::Render();
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
                //UINT sync_interval{ vsync ? 1U : 0U };
                //UINT flags = (tearing_supported && !fullscreenMode && !vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
                //Graphics::GetSwapChain()->Present(sync_interval, flags);
                UINT sync_interval{ 0 };
                Graphics::GetSwapChain()->Present(sync_interval, 0);
            }
        }

#ifdef USE_IMGUI
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif

#if 0
        BOOL fullscreen = 0;
        Graphics::GetSwapChain()->GetFullscreenState(&fullscreen, 0);
        if (fullscreen)
        {
            Graphics::GetSwapChain()->SetFullscreenState(FALSE, 0);
        }
#endif

        return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
    }

    LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
#ifdef USE_IMGUI
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
        switch (msg)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps{};
            BeginPaint(hwnd, &ps);

            EndPaint(hwnd, &ps);
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CREATE:
            break;
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE)
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        case WM_ENTERSIZEMOVE:
            tictoc.stop();
            break;
        case WM_EXITSIZEMOVE:
            tictoc.start();
            break;
        case WM_SIZE:
        {
#if 1
            RECT client_rect{};
            GetClientRect(hwnd, &client_rect);
            Graphics::OnSizeChanged(hwnd,static_cast<UINT64>(client_rect.right - client_rect.left), client_rect.bottom - client_rect.top);
#endif
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
        return 0;
    }

private:
    bool Initialize();
    bool Update(float elapsed_time/*Elapsed seconds from last frame*/);
    void Render(float elapsed_time/*Elapsed seconds from last frame*/, bool renderable);
    bool Uninitialize();
private:
    HighResTimer tictoc;
    uint32_t frames{ 0 };
    float elapsed_time{ 0.0f };
    void calculate_frame_stats()
    {
        if (++frames, (tictoc.time_stamp() - elapsed_time) >= 1.0f)
        {
            float fps = static_cast<float>(frames);
            std::wostringstream outs;
            outs.precision(6);
            outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
            SetWindowTextW(hwnd, outs.str().c_str());

            frames = 0;
            elapsed_time += 1.0f;
        }
    }


    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];

private:
    DirectX::XMFLOAT2 pos = {};
    DirectX::XMFLOAT2 size = { 1280,720 };
    DirectX::XMFLOAT4 colors = {};
    float  angle = {};
    int samplerState = 0;

    // keyFrameの変数
    //回転軸と回転角度の定義
    DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(1, 0, 0, 0);
    float rotationAngle = 1.5f;
    //位置のX座標を設定
    DirectX::XMFLOAT3 translation = { 300.0f,0.0f,0.0f };

    //ブレンドの比率   アニメーションの
    float blendRate = 0.5f;


    //輝度
    float threshold = 0.8f;
    float gaussiamSigma = 1.0f;
    float bloomIntenssity = 1.0f;
    float exposure = 1.2f;

    BOOL fullscreenMode{ FALSE };
    BOOL vsync{ FALSE };
    BOOL tearing_supported{ FALSE };

    RECT windowed_rect;
    DWORD windowed_style;

    //std::unique_ptr<GameManager> gameManager;
};
