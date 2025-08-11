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
	// 深度ステンシルビュー（深度バッファとして使用）
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

public:
	// コンストラクタ：デバイスと指定された幅・高さでシャドウマップを作成
	ShadowMap(ID3D11Device* device, uint32_t width, uint32_t height);
	virtual ~ShadowMap() = default;

	// コピー操作を禁止
	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator =(const ShadowMap&) = delete;
	ShadowMap(ShadowMap&&) noexcept = delete;
	ShadowMap& operator =(ShadowMap&&) noexcept = delete;

	// シェーダーリソースビュー（影のテクスチャとしてシェーダーに渡すため）
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	// ビューポート情報（シャドウマップの描画範囲を設定）
	D3D11_VIEWPORT viewport;

	// シャドウマップのクリア（深度バッファをリセット）
	void Clear(ID3D11DeviceContext* immediateContext, float depth = 1);

	// シャドウマップの描画を開始
	void Activate(ID3D11DeviceContext* immediateContext);

	// シャドウマップの描画を終了し、元の設定に戻す
	void Deactivate(ID3D11DeviceContext* immediateContext);

private:
	// ビューポートの保存用
	UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	
	// 元のレンダーターゲットと深度ステンシルビューの保存用
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
};
