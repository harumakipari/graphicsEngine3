#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/Shader.h"

#include <functional>

#include "Graphics/Core/RenderState.h"

class ComputeParticleSystem
{
public:
	BLEND_STATE blendState = BLEND_STATE::ADD;


	//�p�[�e�B�N���X���b�h��
	static constexpr UINT NumParticleThread = 1024;

	//�p�[�e�B�N�������p�\����
	struct EmitParticleData
	{
		DirectX::XMFLOAT4 parameter{ 1,1,0,-1 };	//x : �p�[�e�B�N�������^�C�v, y : ��������, z : �������ԋL�^�p�i�Z�b�g�s�v�j, w : �����x�����ԁiCPU���Ŏg���p�j

		DirectX::XMFLOAT4 position{ 0,0,0,0 };		//�������W
		DirectX::XMFLOAT4 rotation{ 0,0,0,1 };		//��]
		DirectX::XMFLOAT4 scale{ 1,1,1,1 };			//�g�k ( xy : startSize, zw : endSize )

		DirectX::XMFLOAT4 velocity{ 0,0,0,0 };		//����
		DirectX::XMFLOAT4 acceleration{ 0,0,0,0 };	//�����x

		DirectX::XMFLOAT4 color{ 1,1,1,1 };			//�F���

		DirectX::XMFLOAT4 customData;				//�J�X�^���f�[�^
	};

	//�p�[�e�B�N���\����
	//�A�v���P�[�V�������ł͎g�p���Ȃ����A�`���Ƃ��ĕK�v�Ȃ̂ł����Ő錾���Ă���
	struct ParticleData
	{
		DirectX::XMFLOAT4 parameter{ 1,1,0,-1 };	//x : �p�[�e�B�N�������^�C�v, y : ��������, z : �������ԋL�^�p�i�Z�b�g�s�v�j, w : �����x�����ԁiCPU���Ŏg���p�j

		DirectX::XMFLOAT4 position{ 0,0,0,0 };		//�������W
		DirectX::XMFLOAT4 rotation{ 0,0,0,0 };		//��]
		DirectX::XMFLOAT4 scale{ 1,1,1,1 };			//�g�k ( xy : startSize, zw : endSize )

		DirectX::XMFLOAT4 velocity{ 0,0,0,0 };		//����
		DirectX::XMFLOAT4 acceleration{ 0,0,0,0 };	//�����x

		DirectX::XMFLOAT4 texcoord;					//UV���
		DirectX::XMFLOAT4 color{ 1,1,1,1 };			//�F���

		DirectX::XMFLOAT4 customData;				//�J�X�^���f�[�^
	};

	//�p�[�e�B�N���w�b�_�[�\����
	struct ParticleHeader
	{
		UINT alive;			//�����t���O
		UINT particleIndex;	//�p�[�e�B�N���ԍ�
		float depth;		//�[�x
		UINT dummy;
	};

	//�ėp����`
	struct CommonConstants
	{
		float deltaTime;					//�o�ߎ���
		DirectX::XMUINT2 textureSplitCount;	//�e�N�X�`���̕�����
		UINT systemNumParticles;			//�p�[�e�B�N������
		UINT totalEmitCount;				//���݂̃t���[���ł̃p�[�e�B�N����������
		UINT maxEmitParticles;				//���݂̃t���[���ł̃p�[�e�B�N���ő吶����
		UINT commonDummy[2];
	};

	//�o�C�g�j�b�N�\�[�g����`
	struct BitonicSortConstants
	{
		UINT increment;
		UINT direction;
		UINT dummy[2];
	};
	static constexpr UINT BitonicSortB2Thread = 256;
	static constexpr UINT BitonicSortC2Thread = 512;

public:
	ComputeParticleSystem(ID3D11Device* device, UINT particlesCount, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView,
		DirectX::XMUINT2 splitCount = DirectX::XMUINT2(1, 1));
	~ComputeParticleSystem();

	void Emit(const EmitParticleData& data);
	void PixelEmitBegin(ID3D11DeviceContext* immediateContext, float deltaTime);
	void PixelEmitEnd(ID3D11DeviceContext* immediateContext);

	void Update(ID3D11DeviceContext* immediateContext, float deltaTime);
	void Render(ID3D11DeviceContext* immediateContext);

	void DrawGUI();

private:
	UINT numParticles;
	UINT numEmitParticles;
	bool oneShotInitialize;
	DirectX::XMUINT2 textureSplitCount;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	std::vector<EmitParticleData> pendingParticles;//�G�~�b�g�҂��p�[�e�B�N��
	std::vector<EmitParticleData> emitParticles;//���݂̃t���[���ŃG�~�b�g����p�[�e�B�N��
	Microsoft::WRL::ComPtr<ID3D11Buffer> commonConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> bitonicSortConstantBuffer;

	//�p�[�e�B�N���o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleDataUnordredAccessView;

	//���g�p�p�[�e�B�N���ԍ����i�[����Append/Cosume�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleAppendConsumeBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleAppendConsumeUnordredAccessView;

	//�p�[�e�B�N�����������i�[�����o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleEmitBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleEmitShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleEmitUnordredAccessView;

	//DispatchIndirect�p�\����
	using DispatchIndirect = DirectX::XMUINT3; //UINT3�ŏ\��

	//	00�o�C�g�ځF���݂̃p�[�e�B�N������
	//	04�o�C�g�ځF1�t���[���O�̃p�[�e�B�N������
	//	08�o�C�g�ځF�p�[�e�B�N���j����
	//	12�o�C�g�ځF�p�[�e�B�N�������pDispatchIndirect���
	static constexpr UINT NumCurrentParticleOffset = 0;
	static constexpr UINT NumPreviousParticleOffset = NumCurrentParticleOffset + sizeof(UINT);
	static constexpr UINT NumDeadParticleOffset = NumPreviousParticleOffset + sizeof(UINT);
	static constexpr UINT EmitDispatchIndirectOffset = NumDeadParticleOffset + sizeof(UINT);
	//DrawInstanced�pDrawIndirect�p�\����
	struct DrawIndirect
	{
		UINT vertexCountPerInstance;
		UINT instanceCount;
		UINT startVertexLocation;
		UINT startInstanceLocation;
	};
	//	24�o�C�g�ځF�p�[�e�B�N���X�V�pDispatchIndirect���
	//	36�o�C�g�ځF�p�[�e�B�N���������Ɏg�p����C���f�b�N�X(Append/Consume�̑���)
	//	40�o�C�g�ځFDrawIndirect���
	//	40�o�C�g�ځF�s�N�Z���p�[�e�B�N���������J�E���^�[
	//	44�o�C�g�ځFDrawIndirect���
	static constexpr UINT UpdateDispatchIndirectOffset = EmitDispatchIndirectOffset + sizeof(DispatchIndirect);
	static constexpr UINT NumEmitParticleIndexOffset = UpdateDispatchIndirectOffset + sizeof(DispatchIndirect);
	static constexpr UINT NumEmitPixelParticleIndirectOffset = NumEmitParticleIndexOffset + sizeof(UINT);
	static constexpr UINT DrawIndirectOffset = NumEmitPixelParticleIndirectOffset + sizeof(UINT);

	static constexpr UINT DrawIndirectSize = DrawIndirectOffset + sizeof(DrawIndirect);

	//DrawIndirect��p���邽�߁ARWStructuredBuffer��p������̂ɕύX
	Microsoft::WRL::ComPtr<ID3D11Buffer> indirectDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> indirectDataUnordredAccessView;

	//�p�[�e�B�N���w�b�_�[�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleHeaderBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleHeaderShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleHeaderUnordredAccessView;

	//�e��V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> initShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> emitShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> updateShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> beginFrameShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> endFrameShader;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> sortB2Shader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> sortC2Shader;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
};