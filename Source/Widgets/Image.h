#pragma once
#include "Graphic.h"

#include "Engine/Utility/Win32Utils.h"

#include "../Graphics/Resource/Texture.h"
#include "../Graphics/Core/Shader.h"
#include "Color.h"
#if 1
#include "../Graphics/Core/Graphics.h"
#include "Utils/Dialog.h"
#include "Utils/stdUtiles.h"
#endif
class Image : public Graphic
{
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};
public:

	Image(ID3D11Device* device, const wchar_t* filePath = nullptr) {
		HRESULT hr{ S_OK };
		//頂点情報のセット
		Vertex vertices[]
		{
			{ { -1,  1, 0 }, { 1, 1, 1, 1 } },
			{ {  1,  1, 0 }, { 1, 0, 0, 1 } },
			{ { -1, -1, 0 }, { 0, 1, 0, 1 } },
			{ {  1, -1, 0 }, { 0, 0, 1, 1 } },
		};
		//頂点バッファオブジェクトの生成
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA subresourceData{};
		subresourceData.pSysMem = vertices;
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//画像ファイルのロードとシェーダーリソースビューオブジェクトの生成
		hr = SetSource(device, filePath, false);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//入力レイアウトオブジェクトの生成
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		//頂点シェーダーオブジェクトの生成
		const char* vsName{ "./Shader/sprite_vs.cso" };
		hr = CreateVsFromCSO(device, vsName, vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//ピクセルシェーダーオブジェクトの生成
		const char* psName{ "./Shader/sprite_ps.cso" };
		hr = CreatePsFromCSO(device, psName, pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	~Image() override = default;

	HRESULT SetSource(ID3D11Device* device, const wchar_t* source, bool reload = true) {
		//画像ファイルのロードとシェーダーリソースビューオブジェクトの生成
		HRESULT hr = source ?
			LoadTextureFromFile(device, source, shaderResourceView.ReleaseAndGetAddressOf(), &texture2dDesc) :
			MakeDummyTexture(device, shaderResourceView.ReleaseAndGetAddressOf(), 0xFFFFFFFF, 16);
		if (reload) {
			Initialize();
		}
		return hr;
	}

	void Initialize() override {
		sx = 0, sy = 0, sw = static_cast<float>(texture2dDesc.Width > 0 ? texture2dDesc.Width : 100), sh = static_cast<float>(texture2dDesc.Height > 0 ? texture2dDesc.Height : 100);
		rect->size.x = sw, rect->size.y = sh;
	}

	void Draw(ID3D11DeviceContext* immediateContext) override {
		D3D11_VIEWPORT viewport{};
		UINT numViewports{ 1 };
		immediateContext->RSGetViewports(&numViewports, &viewport);

		//引数から矩形の各頂点の位置（スクリーン座標系）を計算する
		// left-top
		float x0{ rect->TopLeft().x };
		float y0{ rect->TopLeft().y };
		// right-top
		float x1{ rect->TopRight().x };
		float y1{ rect->TopRight().y };
		// left-bottom
		float x2{ rect->BottomLeft().x };
		float y2{ rect->BottomLeft().y };
		// right-bottom
		float x3{ rect->BottomRight().x };
		float y3{ rect->BottomRight().y };

		//切り取り位置
		//left-top
		float tx0{ sx };
		float ty0{ sy };
		//right-top
		float tx1{ sx + sw };
		float ty1{ sy };
		//left-bottom
		float tx2{ sx };
		float ty2{ sy + sh };
		//right-bottom
		float tx3{ sx + sw };
		float ty3{ sy + sh };

		//スクリーン座標系からNDCへの座標変換を行う
		x0 = 2.0f * x0 / viewport.Width - 1.0f;
		y0 = 1.0f - 2.0f * y0 / viewport.Height;
		x1 = 2.0f * x1 / viewport.Width - 1.0f;
		y1 = 1.0f - 2.0f * y1 / viewport.Height;
		x2 = 2.0f * x2 / viewport.Width - 1.0f;
		y2 = 1.0f - 2.0f * y2 / viewport.Height;
		x3 = 2.0f * x3 / viewport.Width - 1.0f;
		y3 = 1.0f - 2.0f * y3 / viewport.Height;

		//計算結果で頂点バッファオブジェクトを更新する
		HRESULT hr{ S_OK };
		D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
		hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
		if (vertices != nullptr)
		{
			vertices[0].position = { x0, y0, 0 };
			vertices[1].position = { x1, y1, 0 };
			vertices[2].position = { x2, y2, 0 };
			vertices[3].position = { x3, y3, 0 };
			vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

			vertices[0].texcoord = { tx0 / texture2dDesc.Width,ty0 / texture2dDesc.Height };
			vertices[1].texcoord = { tx1 / texture2dDesc.Width,ty1 / texture2dDesc.Height };
			vertices[2].texcoord = { tx2 / texture2dDesc.Width,ty2 / texture2dDesc.Height };
			vertices[3].texcoord = { tx3 / texture2dDesc.Width,ty3 / texture2dDesc.Height };
		}
		immediateContext->Unmap(vertexBuffer.Get(), 0);

		//頂点バッファのバインド
		UINT stride{ sizeof(Vertex) };
		UINT offset{ 0 };
		immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

		//プリミティブタイプおよびデータの順序に関する情報のバインド
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//入力レイアウトオブジェクトのバインド
		immediateContext->IASetInputLayout(inputLayout.Get());

		//シェーダーリソースのバインド
		immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

		//シェーダーのバインド
		immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		//プリミティブの描画
		immediateContext->Draw(4, 0);
	}

	void DrawProperty() override {
#ifdef USE_IMGUI
		if (ImGui::Button("Source")) {
			static const char* filter = "Image Files(*.png;*.dds\0*.png;*.dds;\0All Files(*.*)\0*.*;\0\0)";

			char filePath[256] = { 0 };
			HWND hwnd = Graphics::GetWindowHandle();
			DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
			if (result == DialogResult::OK) {
				std::wstring wpath = StringToWstring(std::string(filePath));
				HRESULT hr = SetSource(Graphics::GetDevice(), wpath.c_str());
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
		}
		ImGui::Image(shaderResourceView.Get(), ImVec2(100, 100));
		//ImGui::DragFloat("Angle", &angle);
		ImGui::ColorEdit4("Color", &color.r);
		Graphic::DrawProperty();
#endif // USE_IMGUI
	}
public:
	Color color{ 1,1,1,1 };
public:
	float sx, sy, sw, sh;
private:
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
};