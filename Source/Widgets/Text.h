#pragma once
#include "Graphic.h"
#include <map>
#include <string>
#include <fstream>

#include "Graphics/Resource/Texture.h"
#include "Image.h"
#include "Utils/stdUtiles.h"

class Text : public Graphic
{
	struct Character 
	{
		int x, y, width, height;
		int xoffset, yoffset;
		int xadvance;
		int page, chnl;
	};
	std::map<wchar_t, Character> characters;

	const wchar_t* face;
	float size;
	int bold;
	int italic;
	//char* charset;
	//int unicode;
	//int stretchH;
	//int smooth;
	//int aa;
	//int padding[4];
	//int spacing[2];
	//int outline;
	//friend class InputField;
	int lineHeight;
public:
	/*enum class Alignment {
		TopLeft,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BottomLeft,
		BottomCenter,
		BottomRight
	};
	const char* alignmentStr[9] = {
		"TopLeft",
		"TopCenter",
		"TopRight",
		"MiddleLeft",
		"MiddleCenter",
		"MiddleRight",
		"BottomLeft",
		"BottomCenter",
		"BottomRight"
	};
	inline XMFLOAT2 GetAlignedPos(Alignment a, XMFLOAT2 pos) {
		float x = 0.0f, y = 0.0f;

		switch (a) {
		case Alignment::TopLeft:       x = 0.0f; y = 0.0f; break;
		case Alignment::TopCenter:     x = 0.5f; y = 0.0f; break;
		case Alignment::TopRight:      x = 1.0f; y = 0.0f; break;

		case Alignment::MiddleLeft:    x = 0.0f; y = 0.5f; break;
		case Alignment::MiddleCenter:  x = 0.5f; y = 0.5f; break;
		case Alignment::MiddleRight:   x = 1.0f; y = 0.5f; break;

		case Alignment::BottomLeft:    x = 0.0f; y = 1.0f; break;
		case Alignment::BottomCenter:  x = 0.5f; y = 1.0f; break;
		case Alignment::BottomRight:   x = 1.0f; y = 1.0f; break;
		}

		return XMFLOAT2(x, y);
	}

	enum class HorizontalOverflow {
		Wrap,
		Overflow
	};
	HorizontalOverflow horizontalOverflow = HorizontalOverflow::Wrap;*/

	//右寄せするか
	bool alignRight = false;

public:

	Text(ID3D11Device* device, const std::string& fontFilePath, const char* customPsName = nullptr, const char* customVsName = nullptr, size_t maxElements = 256) 
		: maxVertices(maxElements * 6) {
		//.fntファイルを読み込む
		std::ifstream file(fontFilePath);
		std::string line;
		std::string str;
		while (std::getline(file, line)) {
			if (line.find("info") != std::string::npos) {
				char name[256] = { 0 };
				sscanf_s(line.c_str(), "info face=\"%255[^\"]\" size=%f bold=%d italic=%d",// ... charset=%c unicode=%d stretchH=%d smooth=%d aa=%d padding=\"%3[^\"]\" spacing=\"%1[^\"]\" outline=%d
					name, (unsigned)_countof(name), &size, &bold, &italic/*, charset, &unicode, &stretchH, &smooth, &aa, &padding, _countof(padding), &spacing, _countof(spacing), &outline*/);
				std::string buffer(name);
				std::wstring wstr = std::wstring(buffer.begin(), buffer.end());
				face = wstr.c_str();
			}
			if (line.find("common") != std::string::npos) {
				sscanf_s(line.c_str(), "common lineHeight=%d", &lineHeight);
			}
			if (line.find("page id=") != std::string::npos) {
				char fileName[256] = { 0 };
				sscanf_s(line.c_str(), "page id=%*d file=\"%255[^\"]\"", fileName, (unsigned)_countof(fileName));
				str = "./Data/Font/" + std::string(fileName);
				filePath = std::wstring(str.begin(), str.end());
			}
			if (line.find("char") != std::string::npos) {
				Character c{};
				unsigned int id;
				//文字情報を解析して設定
				sscanf_s(line.c_str(), "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d",
					&id, &c.x, &c.y, &c.width, &c.height, &c.xoffset, &c.yoffset, &c.xadvance, &c.page, &c.chnl);

				characters[id] = c;
			}
		}

		HRESULT hr{ S_OK };

		std::unique_ptr<Vertex[]> vertices{ std::make_unique<Vertex[]>(maxVertices) };

		//頂点バッファ生成
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * maxVertices);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pSysMem = vertices.get();
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, vertexBuffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		const char* vsName{ (customVsName) ? customVsName : "./Shader/sprite_vs.cso" };
		hr = CreateVsFromCSO(device, vsName, vertexShader.GetAddressOf(), inputLayout.GetAddressOf(),
			inputElementDesc, _countof(inputElementDesc));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		const char* psName{ (customPsName) ? customPsName : "./Shader/sprite_ps.cso" };
		hr = CreatePsFromCSO(device, psName, pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = LoadTextureFromFile(device, filePath.c_str(), shaderResourceView.ReleaseAndGetAddressOf(), &texture2dDesc);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	~Text() override = default;

	void Initialize() override {
		rect->size = { 200,200 };
	}

	void Begin(ID3D11DeviceContext* immediateContext) override {
		vertices.clear();
	}

	void Draw(ID3D11DeviceContext* immediateContext) override {
		if (!alignRight)
		{
			XMFLOAT2 _pos = rect->UnrotatedTopLeft();
			float x = _pos.x, y = _pos.y;
			for (int i = 0; text.c_str()[i] != '\0'; ++i) {
				wchar_t character = text.c_str()[i];
				//TODO: 改行する条件に、画像の範囲を超えてたときを追加する
				if (character == '\n') {
					x = _pos.x;
					y += static_cast<float>(lineHeight * lineSpacing * (fontSize / size));
					continue;
				}
				if (character == L' ') {
					//次の文字の位置に移動(今回のみ)
					x += static_cast<float>(31.0f * (fontSize / size));
					continue;
				}
				const Character& c = characters[character];
				//文字の描画に必要な頂点情報を設定
				DrawCharacter(c, x, y, immediateContext);
				//次の文字の位置に移動
				x += static_cast<float>(c.xadvance * (fontSize / size));
			}
		}
		else
		{
			for(std::wstring line : SplitTextToLines())
			{
				XMFLOAT2 _pos = rect->UnrotatedTopRight();
				float x = _pos.x, y = _pos.y;
				for (int i = static_cast<int>(line.size() - 1); i >= 0; --i) {
					wchar_t character = line.c_str()[i];
					//TODO: 改行する条件に、画像の範囲を超えてたときを追加する
					if (character == '\n') {
						x = _pos.x;
						y += static_cast<float>(lineHeight * lineSpacing * (fontSize / size));
						continue;
					}
					if (character == L' ') {
						//次の文字の位置に移動(今回のみ)
						x -= static_cast<float>(31.0f * (fontSize / size));
						continue;
					}
					const Character& c = characters[character];
					//文字の描画に必要な頂点情報を設定
					DrawCharacter(c, x, y, immediateContext);
					//次の文字の位置に移動
					x -= static_cast<float>(c.xadvance * (fontSize / size));
				}
			}
		}
		HRESULT hr{ S_OK };
		D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
		hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		size_t vertexCount = vertices.size();
		_ASSERT_EXPR(maxVertices >= vertexCount, "Buffer overflow");
		Vertex* data{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
		if (data != nullptr) {
			const Vertex* p = vertices.data();
			memcpy_s(data, maxVertices * sizeof(Vertex), p, vertexCount * sizeof(Vertex));
		}
		immediateContext->Unmap(vertexBuffer.Get(), 0);

		UINT stride{ sizeof(Vertex) };
		UINT offset{ 0 };
		immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		immediateContext->IASetInputLayout(inputLayout.Get());

		immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
		immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

		immediateContext->Draw(static_cast<UINT>(vertexCount), 0);
	}

	void End(ID3D11DeviceContext* immediateContext) override {
	}

	void DrawProperty() override {
#ifdef USE_IMGUI
		//ImGui::InputTextMultiline("Text", text.data(), text.max_size(), ImVec2(300, 150));//ImGuiからの日本語入力非対応(追加のライブラリが必要)
		std::string str = WstringToString(text);
		static size_t bufferSize = 256;
		static char buffer[256] = "";
		if (!ImGui::IsItemEdited()) {
			strncpy_s(buffer, str.c_str(), bufferSize);
			buffer[bufferSize - 1] = '\0';
		}
		ImGui::InputTextMultiline("Text", buffer, sizeof(buffer));
		if (ImGui::IsItemEdited()) {
			text = StringToWstring(std::string(buffer));
		}
		ImGui::Text("TextSize:%d", text.size());
		ImGui::Text("FontName:%lc", face);
		ImGui::Text("lineHeight:%d", lineHeight);
		ImGui::ColorEdit4("Color", &color.r);
		ImGui::DragFloat("FontSize", &fontSize);
#endif // USE_IMGUI
	}

	//カーソル位置のスクリーン座標を取得する(ワールド座標)
	void GetCursorPos(size_t cursorPos, _Out_ float& x, _Out_ float& y) {
		XMFLOAT2 _pos = rect->UnrotatedTopLeft();
		x = _pos.x, y = _pos.y;
		for (size_t i = 0; i < cursorPos; ++i) {
			wchar_t character = text.c_str()[i];
			//TODO: 改行する条件に、画像の範囲を超えてたときを追加する
			if (character == '\n') {
				x = _pos.x;
				y += static_cast<float>(lineHeight * (fontSize / size));
				continue;
			}
			//次の文字の位置に移動
			x += static_cast<float>(characters[character].xadvance * (fontSize / size));
		}
	}

	//テキストを列ごとに分ける（L"\n"のみ改行）
	std::vector<std::wstring> SplitTextToLines() const {
		std::vector<std::wstring> lines;
		for (size_t i = 0; i < text.size(); i++) {
			size_t pos = text.find(L"\n", i);
			lines.push_back(text.substr(i, pos - i));
		}
		return lines;
	}
	//テキストを列ごとに分ける（改行：L"\n"とWrap対応）
	std::vector<std::wstring> SplitTextToLines(float maxWidth) {
		std::vector<std::wstring> lines;
		std::wstring currentLine;
		float currentWidth = 0.0f;
		for (size_t i = 0; i < text.size(); ++i) {
			wchar_t ch = text[i];

			if (ch == L'\n') {
				//改行文字
				lines.push_back(currentLine);
				currentLine.clear();
				currentWidth = 0.0f;
			}
			else {
				float charWidth = static_cast<float>(characters[ch].xadvance * (fontSize / size));
				if (currentWidth + charWidth > maxWidth && !currentLine.empty()) {
					//Wrapによる折り返し
					lines.push_back(currentLine);
					currentLine.clear();
					currentWidth = 0.0f;
				}

				currentLine += ch;
				currentWidth += charWidth;
			}
		}
		//残ってたらそれもいれる
		if (!currentLine.empty()) {
			lines.push_back(currentLine);
		}
		return lines;
	}

	void DrawCharacter(const Character& c, float x, float y, ID3D11DeviceContext* immediateConext) {
		float scale = fontSize / size;
		float dx = x + static_cast<float>(c.xoffset);
		float dy = y + static_cast<float>(c.yoffset);
		float dw = static_cast<float>(c.width * scale), dh = static_cast<float>(c.height * scale);
		float sx = static_cast<float>(c.x), sy = static_cast<float>(c.y);
		float sw = static_cast<float>(c.width), sh = static_cast<float>(c.height);
		UINT numViewports{ 1 };
		D3D11_VIEWPORT viewport{};
		immediateConext->RSGetViewports(&numViewports, &viewport);
		XMFLOAT2 pivot = rect->pivot;
		//引数から矩形の各頂点の位置（スクリーン座標系）を計算する
		// left-top
		float x0{ dx };
		float y0{ dy };
		// right-top
		float x1{ dx + dw };
		float y1{ dy };
		// left-bottom
		float x2{ dx };
		float y2{ dy + dh };
		// right-bottom
		float x3{ dx + dw };
		float y3{ dy + dh };

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

		//回転の中心を矩形の中心点にした場合
		float cx = rect->GetWorldPosition().x;
		float cy = rect->GetWorldPosition().y;

		//回転処理
		float angle = rect->angle;
		Rotate(x0, y0, cx, cy, angle);
		Rotate(x1, y1, cx, cy, angle);
		Rotate(x2, y2, cx, cy, angle);
		Rotate(x3, y3, cx, cy, angle);

		//スクリーン座標系からNDCへの座標変換を行う
		x0 = 2.0f * x0 / viewport.Width - 1.0f;
		y0 = 1.0f - 2.0f * y0 / viewport.Height;
		x1 = 2.0f * x1 / viewport.Width - 1.0f;
		y1 = 1.0f - 2.0f * y1 / viewport.Height;
		x2 = 2.0f * x2 / viewport.Width - 1.0f;
		y2 = 1.0f - 2.0f * y2 / viewport.Height;
		x3 = 2.0f * x3 / viewport.Width - 1.0f;
		y3 = 1.0f - 2.0f * y3 / viewport.Height;

		float u0{ sx / texture2dDesc.Width };
		float v0{ sy / texture2dDesc.Height };
		float u1{ (sx + sw) / texture2dDesc.Width };
		float v1{ (sy + sh) / texture2dDesc.Height };


		vertices.push_back({ { x0, y0,0 }, color, { u0, v0 } });
		vertices.push_back({ { x1, y1,0 }, color, { u1, v0 } });
		vertices.push_back({ { x2, y2,0 }, color, { u0, v1 } });
		vertices.push_back({ { x2, y2,0 }, color, { u0, v1 } });
		vertices.push_back({ { x1, y1,0 }, color, { u1, v0 } });
		vertices.push_back({ { x3, y3,0 }, color, { u1, v1 } });
	}

	void Rotate(float& x, float& y, float cx, float cy, float angle)
	{
		x -= cx;
		y -= cy;

		float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
		float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
		float tx{ x }, ty{ y };
		x = cos * tx + -sin * ty;
		y = sin * tx + cos * ty;

		x += cx;
		y += cy;
	}
	std::wstring text = L"Text";
	float fontSize = 64.0f;
	float lineSpacing = 1.0f;
	Color color{ 1,1,1,1 };
private:
	std::wstring filePath;
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};
	std::vector<Vertex> vertices;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	const size_t maxVertices;
};