#pragma once

#include <d3d11_1.h>
#include <wrl.h>
#include <memory>
#include <dxgi1_6.h>

//Direct2D�֘A��L���ɂ���}�N��
#define ENABLE_DIRECT2D
#ifdef ENABLE_DIRECT2D
#include <d2d1_1.h>		  // Direct2D���g�����߂̃w�b�_
#include <dwrite.h>		  // DirectWrite�i�e�L�X�g�����_�����O�j�̃w�b�_
#pragma comment(lib,"d2d1.lib")// Direct2D���C�u�����������N
#pragma comment(lib,"dwrite.lib")// DirectWrite���C�u�����������N
#endif


class Graphics
{
private:
    Graphics() = default;
    ~Graphics() = default;
public:
	// �C���X�^���X�擾
	//static Graphics& Instance()
	//{
	//	static Graphics instance;
	//	return instance;
	//}

	static size_t video_memory_usage()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO video_memory_info;
		adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &video_memory_info);
		return video_memory_info.CurrentUsage / 1024 / 1024;
	}

	// ������
	static void Initialize(HWND hWnd, BOOL fullscreen);

	// �N���A
	static void Clear(float r, float g, float b, float a);

	// �����_�[�^�[�Q�b�g�ݒ�
	static void SetRenderTargets();

	// ��ʕ\��
	static void Present(UINT syncInterval);

	// �E�C���h�E�n���h���擾
	static HWND GetWindowHandle() { return hWnd; }

	// �f�o�C�X�擾
	static ID3D11Device* GetDevice() { return device.Get(); }

	// �f�o�C�X�R���e�L�X�g�擾
	static ID3D11DeviceContext* GetDeviceContext() { return immediateContext.Get(); }

	//�X���b�v�`�F�[�����擾
	static IDXGISwapChain* GetSwapChain() { return swapChain.Get(); }

	// �X�N���[�����擾
	static float GetScreenWidth()  { return screenWidth; }

	// �X�N���[�������擾
	static float GetScreenHeight()  { return screenHeight; }

	// �X���b�v�`�F�[�����쐬����
	static void CreateSwapChain(HWND hwnd, IDXGIFactory6* dxgi_factory6);

	// �E�B���h�E�̊O�ς��t���X�N���[���ɐݒ肷��
	static void StylizeWindow( BOOL fullscreen);

	// �E�B���h�E�T�C�Y�ύX���̏������s��
	static void OnSizeChanged(HWND hwnd, UINT64 width, UINT height);



	static inline BOOL fullscreenMode{ FALSE };// �t���X�N���[�����[�h���ǂ���
private:
	static inline HWND hWnd = nullptr;// �E�B���h�E�̃n���h��
	static inline SIZE framebufferDimensions;// �t���[���o�b�t�@�̃T�C�Y�i��ʉ𑜓x�j
	static inline Microsoft::WRL::ComPtr<ID3D11Device>			device;// Direct3D�̃f�o�C�X�iGPU�j
	static inline Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext;// �f�o�C�X�R���e�L�X�g�i�`��̊Ǘ��j
	static inline Microsoft::WRL::ComPtr<IDXGISwapChain1>			swapChain;// �X���b�v�`�F�[���i��ʕ\���̃o�b�t�@�Ǘ��j
	static inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;// �����_�[�^�[�Q�b�g�r���[�i�`���j
	static inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;// �[�x�X�e���V���r���[�i3D�`��̐[�x�Ǘ��j
	static inline D3D11_VIEWPORT									viewport; // �r���[�|�[�g�i�`��͈́j
	static inline Microsoft::WRL::ComPtr<IDXGIFactory6> dxgi_factory6;

#ifdef ENABLE_DIRECT2D
	static inline Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1DeviceContext;
	static inline Microsoft::WRL::ComPtr<IDWriteTextFormat> dwriteTextFormats[8];// �e�L�X�g�t�H�[�}�b�g
	static inline Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dSolidColorBrushes[8]; // �\���b�h�J���[�̃u���V
	void CreateDirect2dObjects(); // Direct2D�̃I�u�W�F�N�g���쐬���郁�\�b�h
#endif

	static inline float	screenWidth = 0;// �X�N���[���̕�
	static inline float	screenHeight = 0;// �X�N���[���̍���

	static inline BOOL vsync{ FALSE };// ���������̐ݒ�
	static inline BOOL tearingSupported{ FALSE };// �e�A�����O�i��ʂ̂�����j���T�|�[�g���Ă��邩

	static inline RECT windowedRect;// �E�B���h�E���[�h�̍ۂ̃E�B���h�E�ʒu�E�T�C�Y
	static inline DWORD windowedStyle; // �E�B���h�E�X�^�C���i�ʏ펞�j

    static inline Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;// GPU�A�_�v�^�i���ۂ�GPU���j
};