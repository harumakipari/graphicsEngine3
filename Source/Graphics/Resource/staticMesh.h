#pragma once
#include <directXmath.h>
//COMオブジェクトをComPtrスマートポインターテンプレートを使った変数宣言に変更する
#include <wrl.h>

///ID3D11
#include <d3d11.h>

//wstring を　使うため
#include <string>

#include <vector>
class StaticMesh
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texCoord;
    };
    struct Constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 materialColor;
    };

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;


    struct Material
    {
        std::wstring name;
        DirectX::XMFLOAT4 Ka{ 0.2f,0.2f,0.2f,1.0f };
        DirectX::XMFLOAT4 Kd{ 0.8f,0.8f,0.8f,1.0f };
        DirectX::XMFLOAT4 Ks{ 1.0f,1.0f,1.0f,1.0f };
        std::wstring textureFileNames[2];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[2];
    };

    std::vector<Material> materials;

    //
    struct Subset
    {
        std::wstring usemtl;
        uint32_t indexStart{ 0 };   //start position index buffer
        uint32_t indexCount{ 0 };   //number of vertices(indices)
    };
    std::vector<Subset> subsets;


public:
    StaticMesh(ID3D11Device* device, const wchar_t* objFilename);
    virtual ~StaticMesh() = default;

    void Render(ID3D11DeviceContext* immediateContext,
        const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor);

    // 箱メッシュ作成
    void CreateBoxMesh(ID3D11Device* device);

    void CreateBoundingBox(ID3D11Device* device);

    //頂点情報と頂点の順番をGPUに伝える
    void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
        uint32_t* indices, size_t indexCount);

    struct BoundingBox
    {
        DirectX::XMFLOAT3 minPosition = { 0,0,0 };
        DirectX::XMFLOAT3 maxPosition = { 0,0,0 };
    };
    BoundingBox boundPos;

};

