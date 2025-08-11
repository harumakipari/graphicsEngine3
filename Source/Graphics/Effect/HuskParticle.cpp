#include "HuskParticle.h"

#include <random>
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Win32Utils.h"

using namespace DirectX;

HuskParticles::HuskParticles(ID3D11Device* device, size_t maxParticleCount) : maxParticleCount(maxParticleCount)
{
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC bufferDesc{};

	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Particle) * maxParticleCount);
	bufferDesc.StructureByteStride = sizeof(Particle);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&bufferDesc, NULL, particleBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t));
	bufferDesc.StructureByteStride = sizeof(uint32_t);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.MiscFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, NULL, particleCountBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Buffer.ElementOffset = 0;
	shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(maxParticleCount);
	hr = device->CreateShaderResourceView(particleBuffer.Get(), &shaderResourceViewDesc, particleBufferSrv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
	unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unorderedAccessViewDesc.Buffer.FirstElement = 0;
	unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(maxParticleCount);
	unorderedAccessViewDesc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particleBuffer.Get(), &unorderedAccessViewDesc, particleBufferUav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unorderedAccessViewDesc.Buffer.FirstElement = 0;
	unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(maxParticleCount);
	unorderedAccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	hr = device->CreateUnorderedAccessView(particleBuffer.Get(), &unorderedAccessViewDesc, particleAppendBufferUav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	bufferDesc.ByteWidth = sizeof(ParticleConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//TODO: あとで適切なシェーダをセットする(かも)
	CreateVsFromCSO(device, "./Shader/HuskParticleVS.cso", vertexShader.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
	CreatePsFromCSO(device, "./Shader/HuskParticlePS.cso", pixelShader.ReleaseAndGetAddressOf());
	CreateGsFromCSO(device, "./Shader/HuskParticleGS.cso", geometryShader.ReleaseAndGetAddressOf());
	CreateCsFromCSO(device, "./Shader/HuskParticleCS.cso", computeShader.ReleaseAndGetAddressOf());

	CreatePsFromCSO(device, ".Data/Shaders/AccumulateHuskParticlePS.cso", accumlateHuskParticlesPs.ReleaseAndGetAddressOf());
}

UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}

void HuskParticles::Integrate(ID3D11DeviceContext* immediateContext, float deltaTime)
{
	_ASSERT_EXPR(particleData.particleCount <= maxParticleCount, L"");
	if (particleData.particleCount == 0)
	{
		return;
	}

	immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), nullptr);

	particleData.deltaTime = deltaTime;
	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleData, 0, 0);
	immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

	immediateContext->CSSetShader(computeShader.Get(), NULL, 0);

	UINT numThreads = align(particleData.particleCount, 256);
	immediateContext->Dispatch(numThreads / 256, 1, 1);

	ID3D11UnorderedAccessView* nullUnordredAccessView{};
	immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnordredAccessView, nullptr);
}

void HuskParticles::Render(ID3D11DeviceContext* immediateContext)
{
	_ASSERT_EXPR(particleData.particleCount <= maxParticleCount, L"");
	if (particleData.particleCount == 0)
	{
		return;
	}
	
	immediateContext->VSSetShader(vertexShader.Get(), NULL, 0);
	immediateContext->PSSetShader(pixelShader.Get(), NULL, 0);
	immediateContext->GSSetShader(geometryShader.Get(), NULL, 0);
	immediateContext->GSSetShaderResources(9, 1, particleBufferSrv.GetAddressOf());

	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleData, 0, 0);
	immediateContext->VSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
	immediateContext->PSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
	immediateContext->GSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

	immediateContext->IASetInputLayout(NULL);
	immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	immediateContext->Draw(static_cast<UINT>(particleData.particleCount), 0);

	ID3D11ShaderResourceView* nullShaderResourceView{};
	immediateContext->GSSetShaderResources(9, 1, &nullShaderResourceView);
	immediateContext->VSSetShader(NULL, NULL, 0);
	immediateContext->PSSetShader(NULL, NULL, 0);
	immediateContext->GSSetShader(NULL, NULL, 0);
}

void HuskParticles::AccumulateHaskParticles(ID3D11DeviceContext* immediateContext, std::function<void(ID3D11PixelShader*)> drawcallback)
{
	HRESULT hr{ S_OK };

	UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	immediateContext->RSGetViewports(&viewportCount, cachedViewports);

	const float resolutionRatio = 1.0f;
	D3D11_VIEWPORT viewport;
	viewport.Width = cachedViewports[0].Width * resolutionRatio;
	viewport.Height = cachedViewports[0].Height * resolutionRatio;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	immediateContext->RSSetViewports(1, &viewport);

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> cachedUnordredAccessView;
	immediateContext->OMGetRenderTargetsAndUnorderedAccessViews(
		1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.GetAddressOf(),
		1, 1, cachedUnordredAccessView.GetAddressOf());

	ID3D11RenderTargetView* nullRenderTargetView{};
	UINT initialCount{ 0 };
	immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
		1, &nullRenderTargetView, NULL,
		1, 1, particleAppendBufferUav.GetAddressOf(), &initialCount
	);

	drawcallback(accumlateHuskParticlesPs.Get());

	immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
		1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get(),
		1, 1, cachedUnordredAccessView.GetAddressOf(), NULL
	);

	immediateContext->CopyStructureCount(particleCountBuffer.Get(), 0, particleAppendBufferUav.Get());
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	hr = immediateContext->Map(particleCountBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	UINT count{ *reinterpret_cast<UINT*>(mappedSubresource.pData) };
	immediateContext->Unmap(particleCountBuffer.Get(), 0);

	particleData.particleCount = count;

#if 1
	immediateContext->RSSetViewports(viewportCount, cachedViewports);
#endif
}