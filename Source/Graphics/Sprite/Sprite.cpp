#include "Sprite.h"
#include <sstream>
//画像ファイルのロード
#include <WICTextureLoader.h>

//モジュール化する
#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/Shader.h"

Sprite::Sprite(ID3D11Device* device, const wchar_t* filename)
{
    HRESULT hr{ S_OK };

    //頂点情報
    Vertex vertices[]
    {
        { { -0.5, +0.5, 0 }, { 1, 1, 1, 1 },{0,0} },
        { { +0.5, +0.5, 0 }, { 1, 0, 0, 1 },{1,0} },
        { { -0.5, -0.5, 0 }, { 0, 1, 0, 1 },{0,1} },
        { { +0.5, -0.5, 0 }, { 0, 0, 1, 1 },{1,1} },
    };

    //頂点バッファオブジェクトを生成
    {
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(vertices);
        //buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        //buffer_desc.CPUAccessFlags = 0;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA subresource_data{};
        subresource_data.pSysMem = vertices;
        subresource_data.SysMemPitch = 0;
        subresource_data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    //入力レイアウトオブジェクトを生成
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    //頂点シェーダーオブジェクトを生成
    hr = CreateVsFromCSO(device, "./Shader/sprite_vs.cso", vertex_shader.ReleaseAndGetAddressOf(), input_layout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //ピクセルシェーダーオブジェクトを生成
    hr = CreatePsFromCSO(device, "./Shader/sprite_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //テクスチャのロード
    hr = LoadTextureFromFile(device, filename, shaderResourceView.ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


#if 0//以前の
    //頂点シェーダーオブジェクトを生成
    {
        const char* cso_name{ "sprite_vs.cso" };

        FILE* fp{};
        fopen_s(&fp, cso_name, "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long cso_sz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
        fread(cso_data.get(), cso_sz, 1, fp);
        fclose(fp);
        hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, &vertex_shader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //HRESULT hr{ S_OK };
        hr = device->CreateInputLayout(input_element_desc, _countof(input_element_desc),
            cso_data.get(), cso_sz, &input_layout);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    //ピクセルシェーダーオブジェクトの生成
    {
        const char* cso_name{ "sprite_ps.cso" };

        FILE* fp{};
        fopen_s(&fp, cso_name, "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long cso_sz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
        fread(cso_data.get(), cso_sz, 1, fp);
        fclose(fp);
        hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, &pixel_shader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    //画像ファイルのロードとシェーダーリソースビューオブジェクト生成
    ID3D11Resource* resource{};
    hr = DirectX::CreateWICTextureFromFile(device, filename, &resource, &shaderResourceView);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    resource->Release();

    //テクスチャ情報(D3D11_TEXTURE2D_DESC)の取得
    ID3D11Texture2D* texture2d{};
    hr = resource->QueryInterface<ID3D11Texture2D>(&texture2d);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    texture2d->GetDesc(&texture2dDesc);
    texture2d->Release();
#endif
}

Sprite::~Sprite()
{
}

void Sprite::Render(ID3D11DeviceContext* immediate_context,
    float dx, float dy,//矩形の左上の座標（スクリーン座標系)
    float dw, float dh,//矩形のサイズ（スクリーン座標系）
    float r, float g, float b, float a,//色
    float angle/*degree*/)
{
    Render(immediate_context, dx, dy, dw, dh, r, g, b, a, angle, 0, 0, static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height));
}

void Sprite::Render(ID3D11DeviceContext* immediate_context,
    float dx, float dy,//矩形の左上の座標（スクリーン座標系)
    float dw, float dh)//矩形のサイズ（スクリーン座標系）
{
    Render(immediate_context, dx, dy, dw, dh, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0, 0, static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height));
}

void Sprite::Render(ID3D11DeviceContext* immediate_context,
    float dx, float dy,//矩形の左上の座標（スクリーン座標系)
    float dw, float dh,//矩形のサイズ（スクリーン座標系）
    float r, float g, float b, float a,//色
    float angle/*degree*/,
    float sx, float sy, float sw, float sh)//左上座標とサイズ
{
    //スクリーン（ビューポート）のサイズを取得する
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    immediate_context->RSGetViewports(&num_viewports, &viewport);

    //x,yがcx,cyを中心にangleで回転した時の座標を計算する関数
    auto rotate = [](float& x, float& y, float cx, float cy, float angle)
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
        };



    //renderメンバ関数の引数から矩形の各頂点の位置（スクリーン座標系）計算する
    //  (x0, y0) *----* (x1, y1)  
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3) 

    //leftTop
    float x0{ dx };
    float y0{ dy };
    //rightTop
    float x1{ dx + dw };
    float y1{ dy };
    //leftBottom
    float x2{ dx };
    float y2{ dy + dh };
    //rightBottom
    float x3{ dx + dw };
    float y3{ dy + dh };

    //回転の中心を矩形の中心点にした場合の座標を求める
    float cx = dx + dw * 0.5f;
    float cy = dy + dh * 0.5f;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);

    //スクリーン座標からNDCへの座標変換を行う
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    //テクセル座標からUV座標系に変換
    float textureWidth = static_cast<float>(texture2dDesc.Width);
    float textureHeight = static_cast<float>(texture2dDesc.Height);
    float u1 = sx / textureWidth;//テクスチャの左上
    float v1 = sy / textureHeight;
    float u2 = (sx + sw) / textureWidth;//テクスチャの右下
    float v2 = (sy + sh) / textureHeight;

    //計算結果で頂点バッファオブジェクトを更新する
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = immediate_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
        0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0,y0,0 };
        vertices[2].position = { x1,y1,0 };
        vertices[1].position = { x2,y2,0 };
        vertices[3].position = { x3,y3,0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { r,g,b,a };

        vertices[0].texCoord = { u1,v1 };
        vertices[1].texCoord = { u1,v2 };
        vertices[2].texCoord = { u2,v1 };
        vertices[3].texCoord = { u2,v2 };
    }

    immediate_context->Unmap(vertex_buffer.Get(), 0);


    //頂点バッファのバインド(接続）
    UINT stride{ sizeof(Vertex) };
    UINT offset{ 0 };
    immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

    //プリミティブタイプおよびデータの順序に関する情報のバインド
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //入力レイアウトオブジェクトのバインド
    immediate_context->IASetInputLayout(input_layout.Get());

    //シェーダーのバインド
    immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

    //シェーダーリソースのバインド
    immediate_context->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

    //プリミティブの描画
    immediate_context->Draw(4, 0);

}

//フォント画像ファイルを使用して任意の文字列を画面に出力する
void Sprite::Textout(ID3D11DeviceContext* immediateContext, std::string s,
    float x, float y, float w, float h, float r, float g, float b, float a)
{
    float sw = static_cast<float>(texture2dDesc.Width / 16);
    float sh = static_cast<float>(texture2dDesc.Height / 16);
    float carriage = 0;
    for (const char c : s)
    {
        Render(immediateContext, x + carriage, y, w, h, r, g, b, a, 0,
            sw * (c & 0x0F), sh * (c >> 4), sw, sh);
        carriage += w;
    }
}
