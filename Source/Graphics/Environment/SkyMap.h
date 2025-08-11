#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

// SkyMap クラス: スカイボックスや環境マップを描画するクラス
class SkyMap
{
public:
	// コンストラクタ: デバイスとテクスチャファイルを受け取る
	SkyMap(ID3D11Device* device, const wchar_t* filename, bool generate_mips = false);
	virtual ~SkyMap() = default;

	// コピー禁止（オブジェクトの重複を防ぐ）
	SkyMap(const SkyMap&) = delete;
	SkyMap& operator =(const SkyMap&) = delete;
	SkyMap(SkyMap&&) noexcept = delete;
	SkyMap& operator =(SkyMap&&) noexcept = delete;

	// スカイマップを描画する関数
	void Blit(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& view_projection);

	// スカイマップのテクスチャ（シェーダーリソースビュー）
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

private:
	// 頂点シェーダー（SkyMap 用）
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyMapVs;
	// ピクセルシェーダー（通常の SkyMap 用）
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyMapPs;
	// ピクセルシェーダー（スカイボックス用）
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyBoxPs;

	// 定数バッファ（シェーダーに渡すデータ）
	struct Constants
	{
		DirectX::XMFLOAT4X4 inverseViewProjection;// ビュープロジェクション行列の逆行列
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	//テクスチャがキューブマップかどうかを判定するフラグ
	bool isTexturecube = false;
};