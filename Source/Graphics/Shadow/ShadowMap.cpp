#include "ShadowMap.h"
#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/Shader.h"

using namespace DirectX;

// �V���h�E�}�b�v�̃R���X�g���N�^
ShadowMap::ShadowMap(ID3D11Device* device, uint32_t width, uint32_t height)
{
	HRESULT hr{ S_OK };

	// �[�x�o�b�t�@�p�̃e�N�X�`�����쐬
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;// �[�x�o�b�t�@�p�t�H�[�}�b�g
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	
	// �[�x�X�e���V���r���[���쐬�i�`�掞�ɐ[�x�o�b�t�@�Ƃ��Ďg�p�j
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �V�F�[�_�[���\�[�X�r���[���쐬�i�V���h�E�}�b�v�Ƃ��ăT���v���\�ɂ���j
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �r���[�|�[�g��ݒ�
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
}

// �V���h�E�}�b�v���N���A�i�[�x�l�����Z�b�g�j
void ShadowMap::Clear(ID3D11DeviceContext* immediateContext, float depth)
{
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

// �V���h�E�}�b�v�̕`����J�n
void ShadowMap::Activate(ID3D11DeviceContext* immediateContext)
{
	// ���݂̃r���[�|�[�g�ƃ����_�[�^�[�Q�b�g��ۑ�
	viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	//RSGetViewports() ���Ăяo���ƁAcachedViewports �� �ő� 16 �̃r���[�|�[�g�̏���
	// �i�[�ł���z��Ɍ��݂̃r���[�|�[�g�̏�񂪊i�[�����
	immediateContext->RSGetViewports(&viewportCount, cachedViewports);
	//���݂̃����_�[�^�[�Q�b�g (cachedRenderTargetView) ��
	// �[�x�X�e���V���r���[ (cachedDepthStencilView) ���擾���ĕۑ�
	immediateContext->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(), cachedDepthStencilView.ReleaseAndGetAddressOf());

	// �V���h�E�}�b�v�p�̃r���[�|�[�g��ݒ�
	immediateContext->RSSetViewports(1, &viewport);

	// �[�x�o�b�t�@�̂ݎg�p���A�J���[�o�b�t�@�𖳌��ɂ���
	ID3D11RenderTargetView* nullRenderTargetView{ NULL };
	immediateContext->OMSetRenderTargets(1, &nullRenderTargetView, depthStencilView.Get());
}

// �V���h�E�}�b�v�̕`����I�����A���̐ݒ�ɖ߂�
void ShadowMap::Deactivate(ID3D11DeviceContext* immediateContext)
{
	// �ۑ������r���[�|�[�g�ƃ����_�[�^�[�Q�b�g�𕜌�
	immediateContext->RSSetViewports(viewportCount, cachedViewports);
	immediateContext->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get());
	cachedRenderTargetView.Reset();
	cachedDepthStencilView.Reset();
}

