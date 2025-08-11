#include "ShadowMap.h"
#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/Shader.h"

using namespace DirectX;

// シャドウマップのコンストラクタ
ShadowMap::ShadowMap(ID3D11Device* device, uint32_t width, uint32_t height)
{
	HRESULT hr{ S_OK };

	// 深度バッファ用のテクスチャを作成
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;// 深度バッファ用フォーマット
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	
	// 深度ステンシルビューを作成（描画時に深度バッファとして使用）
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// シェーダーリソースビューを作成（シャドウマップとしてサンプル可能にする）
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ビューポートを設定
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
}

// シャドウマップをクリア（深度値をリセット）
void ShadowMap::Clear(ID3D11DeviceContext* immediateContext, float depth)
{
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

// シャドウマップの描画を開始
void ShadowMap::Activate(ID3D11DeviceContext* immediateContext)
{
	// 現在のビューポートとレンダーターゲットを保存
	viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	//RSGetViewports() を呼び出すと、cachedViewports は 最大 16 個のビューポートの情報を
	// 格納できる配列に現在のビューポートの情報が格納される
	immediateContext->RSGetViewports(&viewportCount, cachedViewports);
	//現在のレンダーターゲット (cachedRenderTargetView) と
	// 深度ステンシルビュー (cachedDepthStencilView) を取得して保存
	immediateContext->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(), cachedDepthStencilView.ReleaseAndGetAddressOf());

	// シャドウマップ用のビューポートを設定
	immediateContext->RSSetViewports(1, &viewport);

	// 深度バッファのみ使用し、カラーバッファを無効にする
	ID3D11RenderTargetView* nullRenderTargetView{ NULL };
	immediateContext->OMSetRenderTargets(1, &nullRenderTargetView, depthStencilView.Get());
}

// シャドウマップの描画を終了し、元の設定に戻す
void ShadowMap::Deactivate(ID3D11DeviceContext* immediateContext)
{
	// 保存したビューポートとレンダーターゲットを復元
	immediateContext->RSSetViewports(viewportCount, cachedViewports);
	immediateContext->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get());
	cachedRenderTargetView.Reset();
	cachedDepthStencilView.Reset();
}

