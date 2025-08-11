#pragma once
#define NOMINMAX

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <memory>

#include <vector>
#include <unordered_map>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tiny_gltf.h"
#include "Graphics/Core/RenderState.h"

//SceneRendererで設定している
enum class RenderPass
{
    Opaque,// 不透明オブジェクト
    Mask, // マスク処理
    Blend,// 半透明オブジェクト
};


// glTFモデルの基本クラス
class GltfModelBase
{
protected:
    std::string filename;// モデルファイルの名前

    //マテリアルごとにblendStateを設定するため
    //std::unique_ptr<RenderState> renderState;

public:
    // シーンデータの構造体
    struct Scene
    {
        std::string name;// シーン名
        std::vector<int> nodes;// ルートノードのインデックス配列（シーン内の最上位ノード）
    };
    std::vector<Scene> scenes;//シーンのリスト

    int defaultScene = 0;

    // ノード（オブジェクト）の構造体
    struct Node
    {
        std::string name;//ノード名
        int skin{ -1 };  // このノードが参照するスキンのインデックス
        int mesh{ -1 }; // このノードが参照するメッシュのインデックス

        std::vector<int> children; // 子ノードのインデックス配列

        // 親から見た子の場所ローカルトランスフォーム（ノード自身の回転、スケール、移動）
        DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
        DirectX::XMFLOAT3 translation{ 0,0,0 };

        // グローバルトランスフォーム（初期値は単位行列）
        DirectX::XMFLOAT4X4 globalTransform
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    };
    std::vector<Node> nodes; // ノードのリスト

    // バッファビュー（頂点/インデックスバッファ）
    struct BufferView
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN; // バッファのフォーマット
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;// Direct3Dバッファ
        std::vector<BYTE> row_data; // バッファの生データ
        size_t strideInBytes{ 0 };// 1頂点あたりのバイト数
        size_t sizeInBytes{ 0 };// バッファ全体のバイト数
        size_t count() const
        {
            return sizeInBytes / strideInBytes; // 要素数の計算
        }
    };
    // メッシュデータの構造体
    struct Mesh
    {
        std::string name;// メッシュ名
        struct Primitive
        {
            int material;// マテリアルのインデックス
            std::map<std::string, BufferView> vertexBufferViews;// 頂点バッファビュー
            BufferView indexBufferView;// インデックスバッファビュー
        };
        std::vector<Primitive> primitives;// プリミティブのリスト
    };
    std::vector<Mesh> meshes; // メッシュのリスト

    // テクスチャ情報の構造体
    struct TextureInfo
    {
        int index = -1;
        int texcoord = 0;
    };

    struct NormalTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float scale = 1;
    };

    struct OcclusionTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float strength = 1;
    };
    // PBR用のマテリアルデータ
    struct PbrMetallicRoughness
    {
        float basecolorFactor[4] = { 1,1,1,1 };
        TextureInfo basecolorTexture;
        float metallicFactor = 1;
        float roughnessFactor = 1;
        TextureInfo metallicRoughnessTexture;
    };

    struct Texture
    {
        std::string name;
        int source{ -1 };
    };
    std::vector<Texture> textures;
    // 画像データの構造体
    struct Image
    {
        std::string name;
        int width{ -1 };
        int height{ -1 };
        int component{ -1 };
        int bits{ -1 };
        int pixelType{ -1 };
        int bufferView;
        std::string mimeType;
        std::string uri;
        bool asIs{ false };
    };
    std::vector<Image> images;// 画像データのリスト
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews;// シェーダーリソースビュー

    // マテリアルデータの構造体
    struct Material
    {
        std::string name;// マテリアル名
        Microsoft::WRL::ComPtr<ID3D11PixelShader> replacedPixelShader{ nullptr };// カスタムシェーダー
        struct Cbuffer
        {
            float emissiveFactor[3] = { 0,0,0 };
            int alphaMode = 0; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2 
            float alphaCutoff = 0.5f;
            bool doubleSided = 0;

            PbrMetallicRoughness pbrMetallicRoughness;

            NormalTextureInfo normalTexture;
            OcclusionTextureInfo occlusionTexture;
            TextureInfo emissiveTexture;
        };
        Cbuffer data;
    };
    std::vector<Material> materials;// マテリアルのリスト
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;

    //GLTFMODELのみ使用
    struct Animation
    {
        std::string name;
        float duration{ 0.0f };
        struct Channel
        {
            int sampler{ -1 };
            int targetNode{ -1 };
            std::string targetPath;
        };
        std::vector<Channel> channels;

        struct Sampler
        {
            int input{ -1 };
            int output{ -1 };
            std::string interpolation;
        };
        std::vector<Sampler>  samplers;

        std::unordered_map<int/*sampler.input*/, std::vector<float>> timelines;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> scales;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT4>> rotations;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> translations;

    };
    std::vector<Animation> animations = {};

protected:
    // ノードの階層構造を考慮しながら各ノードのグローバルトランスフォームを累積計算
    virtual void CumulateTransforms(std::vector<Node>& nodes);

    // CascadedShadowMaps
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShaderCSM;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShaderCSM;

public:
    //デフォルトコンストラクタ
    GltfModelBase() = default;

    GltfModelBase(ID3D11Device* device, const std::string& filename) : filename(filename)
    {
        //renderState = std::make_unique<decltype(renderState)::element_type>(device);
    }

    virtual ~GltfModelBase() = default;

    // モデルの描画関数
    virtual void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, RenderPass pass) = 0;


    //------------------GLTFMODELでのみ使用のためフックにしている-------------------//

    // シャドウ用の描画関数
    virtual void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes) {};

    //アニメーションを適応する関数　
    virtual void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes) {}

    //アニメーションを追加する関数
    virtual void AddAnimation(const std::string& filename) {}

    //アニメーションをブレンドする関数
    virtual void BlendAnimations(const std::vector<Node>& from_nodes, const std::vector<Node>& to_nodes, float factor, std::vector<Node>& out_nodes) {}

    //アニメーションのノード用の描画処理     
    virtual void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes, RenderPass pass) {}

    //モデルのジョイントのpositionを返す関数
    virtual DirectX::XMFLOAT3 JointPosition(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //モデルのジョイントのX軸方向のベクトル関数
    virtual DirectX::XMFLOAT3 GetJointRightVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //モデルのジョイントのY軸方向のベクトル関数
    virtual DirectX::XMFLOAT3 GetJointUpVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //モデルのジョイントのZ軸方向のベクトル関数
    virtual DirectX::XMFLOAT3 GetJointForwardVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //モデルのジョイントのワールド変換行列を返す関数
    virtual DirectX::XMFLOAT4X4 GetJointTransform(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); };
};