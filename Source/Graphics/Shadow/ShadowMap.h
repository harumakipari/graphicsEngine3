#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <cstdint>
#include <functional>

#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/PostProcess/frameBuffer.h"

class ShadowMap
{
	// �[�x�X�e���V���r���[�i�[�x�o�b�t�@�Ƃ��Ďg�p�j
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

public:
	// �R���X�g���N�^�F�f�o�C�X�Ǝw�肳�ꂽ���E�����ŃV���h�E�}�b�v���쐬
	ShadowMap(ID3D11Device* device, uint32_t width, uint32_t height);
	virtual ~ShadowMap() = default;

	// �R�s�[������֎~
	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator =(const ShadowMap&) = delete;
	ShadowMap(ShadowMap&&) noexcept = delete;
	ShadowMap& operator =(ShadowMap&&) noexcept = delete;

	// �V�F�[�_�[���\�[�X�r���[�i�e�̃e�N�X�`���Ƃ��ăV�F�[�_�[�ɓn�����߁j
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	// �r���[�|�[�g���i�V���h�E�}�b�v�̕`��͈͂�ݒ�j
	D3D11_VIEWPORT viewport;

	// �V���h�E�}�b�v�̃N���A�i�[�x�o�b�t�@�����Z�b�g�j
	void Clear(ID3D11DeviceContext* immediateContext, float depth = 1);

	// �V���h�E�}�b�v�̕`����J�n
	void Activate(ID3D11DeviceContext* immediateContext);

	// �V���h�E�}�b�v�̕`����I�����A���̐ݒ�ɖ߂�
	void Deactivate(ID3D11DeviceContext* immediateContext);

private:
	// �r���[�|�[�g�̕ۑ��p
	UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	
	// ���̃����_�[�^�[�Q�b�g�Ɛ[�x�X�e���V���r���[�̕ۑ��p
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
};
