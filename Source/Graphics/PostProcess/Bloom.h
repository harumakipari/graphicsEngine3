// BLOOM
#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>

#include "FrameBuffer.h"
#include "FullScreenQuad.h"

class Bloom
{
public:
	Bloom(ID3D11Device* device, uint32_t width, uint32_t height);
	~Bloom() = default;
	Bloom(const Bloom&) = delete;
	Bloom& operator =(const Bloom&) = delete;
	Bloom(Bloom&&) noexcept = delete;
	Bloom& operator =(Bloom&&) noexcept = delete;

	void make(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* color_map);
	ID3D11ShaderResourceView* shader_resource_view() const
	{
		return glow_extraction->shaderResourceViews[0].Get();
	}

public:
	//float bloom_extraction_threshold = 0.85f;
	//float bloom_intensity = 0.016f;
	float bloom_extraction_threshold = 0.777f;
	float bloom_intensity = 0.163f;

private:
	std::unique_ptr<FullScreenQuad> bit_block_transfer;
	std::unique_ptr<FrameBuffer> glow_extraction;

	static const size_t downsampled_count = 6;
	std::unique_ptr<FrameBuffer> gaussian_blur[downsampled_count][2];

	Microsoft::WRL::ComPtr<ID3D11PixelShader> glow_extraction_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_downsampling_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_horizontal_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_vertical_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_upsampling_ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state;

	struct bloom_constants
	{
		float bloom_extraction_threshold;
		float bloom_intensity;
		float something[2];
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

};
