#include "ComputeParticleSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


ComputeParticleSystem::ComputeParticleSystem(ID3D11Device* device, UINT particlesCount,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView, DirectX::XMUINT2 splitCount)
{
	HRESULT hr;
	//�o�C�g�j�b�N�\�[�g�̎d�l��A�Q�̗ݏ�ɂ��Ă����Ȃ��Ƃ����Ȃ����߁A�p�[�e�B�N������␳
	float f_exponent = log2f(static_cast<float>(particlesCount));
	int exponent = static_cast<int>(ceilf(f_exponent) + 0.5f);
	particlesCount = static_cast<UINT>(pow(2, exponent) + 0.5f);
	particlesCount = max(min(particlesCount, 1 << 27), 1 << 7);

	//�p�[�e�B�N�������X���b�h���ɍ��킹�Đ���
	numParticles = ((particlesCount + (NumParticleThread - 1)) / NumParticleThread) * NumParticleThread;
	numEmitParticles = numParticles;
	textureSplitCount = splitCount;
	oneShotInitialize = false;

	//�萔�o�b�t�@
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		{
			bufferDesc.ByteWidth = sizeof(CommonConstants);
			hr = device->CreateBuffer(&bufferDesc, nullptr, commonConstantBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			bufferDesc.ByteWidth = sizeof(BitonicSortConstants);
			hr = device->CreateBuffer(&bufferDesc, nullptr, bitonicSortConstantBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}

	//�p�[�e�B�N���o�b�t�@����
	{
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = sizeof(ParticleData) * numParticles;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(ParticleData);
		desc.Usage = D3D11_USAGE_DEFAULT;
		hr = device->CreateBuffer(&desc, nullptr, particleDataBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = device->CreateShaderResourceView(particleDataBuffer.Get(), nullptr, particleDataShaderResourceView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		hr = device->CreateUnorderedAccessView(particleDataBuffer.Get(), nullptr, particleDataUnordredAccessView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//�p�[�e�B�N���w�b�_�[�o�b�t�@
	{
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = sizeof(ParticleHeader) * numParticles;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(ParticleHeader);
		desc.Usage = D3D11_USAGE_DEFAULT;
		hr = device->CreateBuffer(&desc, nullptr, particleHeaderBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = device->CreateShaderResourceView(particleHeaderBuffer.Get(), nullptr, particleHeaderShaderResourceView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		hr = device->CreateUnorderedAccessView(particleHeaderBuffer.Get(), nullptr, particleHeaderUnordredAccessView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//�p�[�e�B�N���̐���/�j���ԍ��𗭂ߍ��ރo�b�t�@����
	{
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = sizeof(UINT) * numParticles;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(UINT);
		desc.Usage = D3D11_USAGE_DEFAULT;
		hr = device->CreateBuffer(&desc, nullptr, particleAppendConsumeBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//Append/Consume�𗘗p����ꍇ�̓r���[���Ƀt���O�𗧂Ă�
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = numParticles;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		hr = device->CreateUnorderedAccessView(particleAppendConsumeBuffer.Get(), &uavDesc, particleAppendConsumeUnordredAccessView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//�p�[�e�B�N���G�~�b�g�p�o�b�t�@����
	{
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = sizeof(EmitParticleData) * numEmitParticles;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(EmitParticleData);
		desc.Usage = D3D11_USAGE_DEFAULT;
		hr = device->CreateBuffer(&desc, nullptr, particleEmitBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = device->CreateShaderResourceView(particleEmitBuffer.Get(), nullptr, particleEmitShaderResourceView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = device->CreateUnorderedAccessView(particleEmitBuffer.Get(), nullptr, particleEmitUnordredAccessView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//�p�[�e�B�N���̍X�V�E�`�搔�̍팸�̂��߂̃o�b�t�@
	{
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = DrawIndirectSize;
		desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA initializeData{};
		std::vector<UINT> initializeBuffer(desc.ByteWidth / sizeof(UINT));
		{
			//�����l�ݒ�
			DrawIndirect* drawData = reinterpret_cast<DrawIndirect*>(initializeBuffer.data() + DrawIndirectOffset / sizeof(UINT));
			drawData->vertexCountPerInstance = numParticles;
			drawData->instanceCount = 1;
			drawData->startVertexLocation = 0;
			drawData->startInstanceLocation = 0;
		}

		//���������Ă���
		{
			UINT index = NumEmitPixelParticleIndirectOffset / sizeof(UINT);
			initializeBuffer[index] = 0;
		}

		initializeData.pSysMem = initializeBuffer.data();
		initializeData.SysMemPitch = desc.ByteWidth;
		hr = device->CreateBuffer(&desc, &initializeData, indirectDataBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//Append/Consume�𗘗p����ꍇ�̓r���[���Ƀt���O�𗧂Ă�
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = desc.ByteWidth / sizeof(UINT);
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		hr = device->CreateUnorderedAccessView(indirectDataBuffer.Get(), &uavDesc, indirectDataUnordredAccessView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//�R���s���[�g�V�F�[�_�[�ǂݍ���
	CreateCsFromCSO(device, "./Shader/ComputeParticleInitCS.cso", initShader.GetAddressOf());
	CreateCsFromCSO(device, "./Shader/ComputeParticleEmitCS.cso", emitShader.GetAddressOf());
	CreateCsFromCSO(device, "./Shader/ComputeParticleUpdateCS.cso", updateShader.GetAddressOf());
	CreateCsFromCSO(device, "./Shader/ComputeParticleBeginFrameCS.cso", beginFrameShader.GetAddressOf());
	CreateCsFromCSO(device, "./Shader/ComputeParticleEndFrameCS.cso", endFrameShader.GetAddressOf());
	
	CreateCsFromCSO(device, "./Shader/ComputeParticleBitonicSortB2CS.cso", sortB2Shader.GetAddressOf());
	CreateCsFromCSO(device, "./Shader/ComputeParticleBitonicSortC2CS.cso", sortC2Shader.GetAddressOf());

	//�`��p��񐶐�
	this->shaderResourceView = shaderResourceView;
	CreateVsFromCSO(device, "./Shader/ComputeParticleRenderVS.cso", vertexShader.GetAddressOf(), nullptr, nullptr, 0);
	CreateGsFromCSO(device, "./Shader/ComputeParticleRenderGS.cso", geometryShader.GetAddressOf());
	CreatePsFromCSO(device, "./Shader/ComputeParticleRenderPS.cso", pixelShader.GetAddressOf());
}

ComputeParticleSystem::~ComputeParticleSystem()
{
	initShader.Reset();
	updateShader.Reset();
	emitShader.Reset();
	vertexShader.Reset();
	geometryShader.Reset();
	pixelShader.Reset();
	shaderResourceView.Reset();
}

void ComputeParticleSystem::Emit(const EmitParticleData& data)
{
	if (emitParticles.size() + pendingParticles.size() >= numEmitParticles)
		return;

	if (data.parameter.w <= 0) {
		//�x���Ȃ��Ȃ璼�ڒǉ�
		emitParticles.emplace_back(data);
	}
	else {
		//�ۗ����X�g�ɒǉ�
		pendingParticles.emplace_back(data);
	}
}

void ComputeParticleSystem::PixelEmitBegin(ID3D11DeviceContext* immediateContext, float deltaTime)
{
	//�萔�o�b�t�@�ݒ�
	{
		immediateContext->PSSetConstantBuffers(10, 1, commonConstantBuffer.GetAddressOf());
		immediateContext->CSSetConstantBuffers(10, 1, commonConstantBuffer.GetAddressOf());
		immediateContext->CSSetConstantBuffers(11, 1, bitonicSortConstantBuffer.GetAddressOf());

		//�萔�o�b�t�@�X�V
		CommonConstants constant;
		constant.deltaTime = deltaTime;
		constant.textureSplitCount = textureSplitCount;
		constant.systemNumParticles = numParticles;
		constant.totalEmitCount = static_cast<UINT>(emitParticles.size());
		constant.maxEmitParticles = numEmitParticles;
		immediateContext->UpdateSubresource(commonConstantBuffer.Get(), 0, nullptr, &constant, 0, 0);
	}

	//����������
	if (!oneShotInitialize)
	{
		oneShotInitialize = true;
		immediateContext->CSSetShader(initShader.Get(), nullptr, 0);
		immediateContext->Dispatch(numParticles / NumParticleThread, 1, 1);
	}

	//�s�N�Z���G�~�b�^�[�̏���
	ID3D11UnorderedAccessView* uavs[] =
	{
		particleEmitUnordredAccessView.Get(),
		indirectDataUnordredAccessView.Get(),
	};
	immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr,
		0, ARRAYSIZE(uavs), uavs, nullptr);
}

void ComputeParticleSystem::PixelEmitEnd(ID3D11DeviceContext* immediateContext)
{
	immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr,
		0, 0, nullptr, nullptr);
}

void ComputeParticleSystem::Update(ID3D11DeviceContext* immediateContext, float deltaTime)
{
	//�ۗ����X�g�̃p�[�e�B�N���̏���
	{
		for (size_t i = 0; i < pendingParticles.size();)
		{
			EmitParticleData& data = pendingParticles[i];
			data.parameter.w -= deltaTime;
			if (data.parameter.w <= 0.0f)
			{
				emitParticles.emplace_back(data);

				//�Ō�̗v�f�ƌ������č폜
				data = pendingParticles.back();
				pendingParticles.pop_back();
				//�C���f�b�N�X�͂��̂܂܁i�Ōォ�玝���Ă����v�f���ĕ]���j
			}
			else {
				i++;
			}
		}
		
	}

	//�萔�o�b�t�@�ݒ�
	{
		//�萔�o�b�t�@�X�V
		CommonConstants constant;
		constant.deltaTime = deltaTime;
		constant.textureSplitCount = textureSplitCount;
		constant.systemNumParticles = numParticles;
		constant.totalEmitCount = static_cast<UINT>(emitParticles.size());
		constant.maxEmitParticles = numEmitParticles;
		immediateContext->UpdateSubresource(commonConstantBuffer.Get(), 0, nullptr, &constant, 0, 0);

		immediateContext->CSSetConstantBuffers(10, 1, commonConstantBuffer.GetAddressOf());
		immediateContext->CSSetConstantBuffers(11, 1, bitonicSortConstantBuffer.GetAddressOf());

	}

	//SRV/UAV�ݒ�
	{
		immediateContext->CSSetShaderResources(0, 1, particleEmitShaderResourceView.GetAddressOf());

		//Append/Consume�o�b�t�@����������
		if (!oneShotInitialize)
		{
			UINT clearParameter = 0;
			immediateContext->CSSetUnorderedAccessViews(0, 1, particleAppendConsumeUnordredAccessView.GetAddressOf(), &clearParameter);
		}

		ID3D11UnorderedAccessView* uavs[] =
		{
			particleDataUnordredAccessView.Get(),
			particleAppendConsumeUnordredAccessView.Get(),
			indirectDataUnordredAccessView.Get(),
			particleHeaderUnordredAccessView.Get(),
		};
		immediateContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
	}

	//����������
	if (!oneShotInitialize)
	{
		oneShotInitialize = true;
		immediateContext->CSSetShader(initShader.Get(), nullptr, 0);
		immediateContext->Dispatch(numParticles / NumParticleThread, 1, 1);
	}

	//�t���[���J�n���̏���
	{
		//���݃t���[���ł̃p�[�e�B�N���������Z�o
		//����ɍ��킹�Ċe��ݒ���s��
		immediateContext->CSSetShader(beginFrameShader.Get(), nullptr, 0);
		immediateContext->Dispatch(1, 1, 1);
	}

	//�G�~�b�g����
	{
		//�G�~�b�g�o�b�t�@�X�V
		if (!emitParticles.empty())
		{
			D3D11_BOX writeBox{};
			writeBox.left = 0;
			writeBox.right = static_cast<UINT>(emitParticles.size() * sizeof(EmitParticleData));
			writeBox.top = 0;
			writeBox.bottom = 1;
			writeBox.front = 0;
			writeBox.back = 1;
			immediateContext->UpdateSubresource(particleEmitBuffer.Get(),
				0,
				&writeBox,
				emitParticles.data(),
				static_cast<UINT>(emitParticles.size() * sizeof(EmitParticleData)),
				0);
			emitParticles.clear();
		}

		immediateContext->CSSetShader(emitShader.Get(), nullptr, 0);
		immediateContext->DispatchIndirect(indirectDataBuffer.Get(), EmitDispatchIndirectOffset);
	}

	//�X�V����
	{
		immediateContext->CSSetShader(updateShader.Get(), nullptr, 0);
		immediateContext->DispatchIndirect(indirectDataBuffer.Get(), UpdateDispatchIndirectOffset);
	}

	{
		//�\�[�g����
		//�o�C�g�j�b�N�\�[�g
		float f_exponent = log2f(static_cast<float>(numParticles));
		UINT exponent = static_cast<UINT>(ceilf(f_exponent) + 0.5f);
		for (UINT i = 0; i < exponent; ++i)
		{
			immediateContext->CSSetShader(sortB2Shader.Get(), nullptr, 0);
			UINT increment = 1 << i;
			for (UINT j = 0; j < i + 1; ++j)
			{
				BitonicSortConstants constant;
				constant.increment = increment;
				constant.direction = 2 << i;
				immediateContext->UpdateSubresource(bitonicSortConstantBuffer.Get(), 0, nullptr, &constant, 0, 0);

				if (increment <= BitonicSortC2Thread)
				{
					immediateContext->CSSetShader(sortC2Shader.Get(), nullptr, 0);
					immediateContext->Dispatch(numParticles / 2 / BitonicSortC2Thread, 1, 1);
					break;
				}

				immediateContext->Dispatch(numParticles / 2 / BitonicSortB2Thread, 1, 1);
				increment /= 2;
			}
		}
	}

	//�t���[���I�����̏���
	{
		//���p�[�e�B�N������ϓ�������
		immediateContext->CSSetShader(endFrameShader.Get(), nullptr, 0);
		immediateContext->Dispatch(1, 1, 1);
	}

	//NULL��UAV�ݒ�
	{
		ID3D11UnorderedAccessView* uavs[] = { nullptr, nullptr, nullptr, nullptr };
		immediateContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
	}
}

void ComputeParticleSystem::Render(ID3D11DeviceContext* immediateContext)
{
	//�_�`��ݒ�
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//�V�F�[�_�[�ݒ�
	immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	immediateContext->GSSetShader(geometryShader.Get(), nullptr, 0);
	immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

	//���̓��C�A�E�g�ݒ�
	immediateContext->IASetInputLayout(nullptr);

	//���\�[�X�ݒ�
	immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
	immediateContext->GSSetShaderResources(0, 1, particleDataShaderResourceView.GetAddressOf());
	immediateContext->GSSetShaderResources(1, 1, particleHeaderShaderResourceView.GetAddressOf());

	//�o�b�t�@�N���A
	ID3D11Buffer* clearBuffer[] = { nullptr };
	UINT strides[] = { 0 };
	UINT offsets[] = { 0 };
	immediateContext->IASetVertexBuffers(0, 1, clearBuffer, strides, offsets);
	immediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	//GPU���Ōv�Z�����p�[�e�B�N���̕`�搔��CPU���Ŏ擾���ĕ`��R�[�����Ăяo���̂͂��������Ȃ��̂ŁA
	// DrawIndirect���߂��g����GPU�������ŏ���������
	immediateContext->DrawInstancedIndirect(indirectDataBuffer.Get(), DrawIndirectOffset);

	//�V�F�[�_�[������
	immediateContext->VSSetShader(nullptr, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	immediateContext->PSSetShader(nullptr, nullptr, 0);

	//���\�[�X�N���A
	ID3D11ShaderResourceView* clearShaderResourceViews[] = { nullptr, nullptr };
	immediateContext->PSSetShaderResources(0, 2, clearShaderResourceViews);
	immediateContext->GSSetShaderResources(0, 2, clearShaderResourceViews);
	immediateContext->CSSetShaderResources(0, 2, clearShaderResourceViews);
}

void ComputeParticleSystem::DrawGUI()
{
#ifdef USE_IMGUI
	ImGui::Text("EmitParticles:%d", numEmitParticles);
	ImGui::Text("NumParticles:%d", numParticles);
#endif // USE_IMGUI
}