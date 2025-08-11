#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

// SkyMap �N���X: �X�J�C�{�b�N�X����}�b�v��`�悷��N���X
class SkyMap
{
public:
	// �R���X�g���N�^: �f�o�C�X�ƃe�N�X�`���t�@�C�����󂯎��
	SkyMap(ID3D11Device* device, const wchar_t* filename, bool generate_mips = false);
	virtual ~SkyMap() = default;

	// �R�s�[�֎~�i�I�u�W�F�N�g�̏d����h���j
	SkyMap(const SkyMap&) = delete;
	SkyMap& operator =(const SkyMap&) = delete;
	SkyMap(SkyMap&&) noexcept = delete;
	SkyMap& operator =(SkyMap&&) noexcept = delete;

	// �X�J�C�}�b�v��`�悷��֐�
	void Blit(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& view_projection);

	// �X�J�C�}�b�v�̃e�N�X�`���i�V�F�[�_�[���\�[�X�r���[�j
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

private:
	// ���_�V�F�[�_�[�iSkyMap �p�j
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyMapVs;
	// �s�N�Z���V�F�[�_�[�i�ʏ�� SkyMap �p�j
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyMapPs;
	// �s�N�Z���V�F�[�_�[�i�X�J�C�{�b�N�X�p�j
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyBoxPs;

	// �萔�o�b�t�@�i�V�F�[�_�[�ɓn���f�[�^�j
	struct Constants
	{
		DirectX::XMFLOAT4X4 inverseViewProjection;// �r���[�v���W�F�N�V�����s��̋t�s��
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	//�e�N�X�`�����L���[�u�}�b�v���ǂ����𔻒肷��t���O
	bool isTexturecube = false;
};