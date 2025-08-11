#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>
#include <functional>

struct HuskParticles
{
	size_t maxParticleCount;
	struct Particle
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 velocity;
		float age{};
		int state{};
	};

	struct ParticleConstants
	{
		int particleCount{};
		float particleSize{ 0.005f };
		float particleOption{};
		float deltaTime{};
	};
	ParticleConstants particleData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleAppendBufferUav;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particleCountBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	HuskParticles(ID3D11Device* device, size_t maxParticleCount = 100000);
	HuskParticles(const HuskParticles&) = delete;
	HuskParticles& operator=(const HuskParticles&) = delete;
	HuskParticles(HuskParticles&&) noexcept = delete;
	HuskParticles& operator=(HuskParticles&&) noexcept = delete;
	virtual ~HuskParticles() = default;

	void Integrate(ID3D11DeviceContext* immediateContext, float deltaTime);
	void Render(ID3D11DeviceContext* immediateContext);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> accumlateHuskParticlesPs;
	void AccumulateHaskParticles(ID3D11DeviceContext* immediateContext, std::function<void(ID3D11PixelShader*)> drawcallback);
};