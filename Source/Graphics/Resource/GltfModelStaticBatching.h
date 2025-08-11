#pragma once
#define NOMINMAX

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <vector>
#include <unordered_map>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tiny_gltf.h"
#include "GltfModelBase.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>



class GltfModelStaticBatching :public GltfModelBase
{
public:
    // コンストラクタ: GLTFモデルを読み込む
    GltfModelStaticBatching(ID3D11Device* device, const std::string& filename);
    virtual ~GltfModelStaticBatching() = default;

    // プリミティブ（モデルの各パーツ）を表す構造体
    struct Primitive
    {
        int material;// 使用するマテリアルのインデックス
        // 頂点バッファのビューを格納するマップ（キーは頂点属性名）
        std::map<std::string, BufferView> vertexBufferViews;
        BufferView indexBufferView; // インデックスバッファのビュー
    };
    // バッチ処理されたバッファを表す構造体
    struct CombinedBuffer
    {
        size_t index_count;// インデックス数
        size_t vertex_count;// 頂点数

        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        std::vector<unsigned int> indices;
        struct structure_of_arrays
        {
            std::vector<DirectX::XMFLOAT3> positions;
            std::vector<DirectX::XMFLOAT3> normals;
            std::vector<DirectX::XMFLOAT4> tangents;
            std::vector<DirectX::XMFLOAT2> texcoords;
            //template <class T>
            //void serialize(T& archive)
            //{
            //    archive(positions/*, normals, tangents, texcoords*/);
            //}
        };
        structure_of_arrays vertices;
        //template <class T>
        //void serialize(T& archive)
        //{
        //    archive(vertices);
        //}
    };
    //std::unordered_map<int/*material*/, CombinedBuffer> combined_buffers;
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        template <class T>
        void serialize(T& archive)
        {
            archive(position/*, normals, tangents, texcoords*/);
        }
    };
    Vertex vertices;


    std::vector<Primitive> primitives;

    void Render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, RenderPass pass)override;

    void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes, RenderPass pass)override
    {//アニメーションしないがrendererで使用するため
        Render(immediateContext, world, pass);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes)override;

private:
    void FetchNodes(const tinygltf::Model& gltf_model);
    BufferView MakeBufferView(const tinygltf::Accessor& accessor);
    void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltf_model);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    struct primitive_constants
    {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int has_tangent{ 0 };
        int pad[2];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer;

    void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltf_model);
    void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltf_model);
};