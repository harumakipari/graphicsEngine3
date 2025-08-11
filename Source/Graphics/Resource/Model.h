#ifndef MODEL_H
#define MODEL_H

#define NOMINMAX // max 識別子が反応するから

#include <map>
#include <limits>
#include <string>

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <unordered_map>
#include <memory>

#include <vector>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Graphics/Core/ConstantBuffer.h"

UINT _SizeofComponent(DXGI_FORMAT format);

class Model
{
public:
    static inline std::unordered_map<std::string/*file name*/, std::shared_ptr<Model>> models_;

    static std::shared_ptr<Model> emplace(ID3D11Device* device, const std::string& filename, bool staticBatching, float toMetersScale = 1.0f, size_t coordinateSystem = 0)
    {
        auto cachedModel = models_.find(filename);
        if (cachedModel != models_.end())
        {
            return cachedModel->second;
        }
        return models_.emplace(filename, std::make_shared<Model>(device, filename, staticBatching, toMetersScale, coordinateSystem)).first->second;
    }

    static std::shared_ptr<Model> at(const std::string& filename)
    {
        _ASSERT_EXPR(models_.find(filename) != models_.end(), L"このファイル名でモデルは登録されていません。");
        return models_.at(filename);
    }

public:
    Model(ID3D11Device* device, const std::string& filename, bool staticBatching, float toMetersScale = 1.0f, size_t coordinateSystem = 0);
    Model(const Model& rhs) = delete;
    Model& operator=(const Model& rhs) = delete;
    Model(Model&&) noexcept = default;
    Model& operator=(Model&&) noexcept = delete;
    ~Model() = default;

    const bool staticBatching;
    const  DirectX::XMMATRIX axisConversionMatrix;

public:
    std::string filename;

    struct Asset
    {
        std::string version = "2.0";
        std::string generator;
        std::string minversion;
        std::string copyright;
    };
    Asset asset;

    struct PunctualLight
    {
        std::string name;
        float color[3] = { 1.0f,1.0f,1.0f };
    };

    // マテリアルの variants
    std::vector<std::string> variants;

    struct Scene
    {
        std::string name;
        std::vector<int> nodes;// ルートノードの配列
    };
    std::vector<Scene> scenes;
    int defaultScene = 0;

    struct Node
    {
        std::string name;
        int camera = -1;
        int skin = -1;
        int mesh = -1;

        std::vector<int> children;

        // ローカルのトランスフォーム
        DirectX::XMFLOAT4 rotation = { 0,0,0,1 };
        DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
        DirectX::XMFLOAT3 translation = { 0,0,0 };
        std::vector<double> weights; // モーフターゲットの重み

        DirectX::XMFLOAT4X4 globalTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1,
        };

        // グローバル空間での AABB（軸平行境界ボックス）最小・最大座標
        DirectX::XMFLOAT3 min_value =
        {
           +std::numeric_limits<float>::max(),
           +std::numeric_limits<float>::max(),
           +std::numeric_limits<float>::max()
        };

        DirectX::XMFLOAT3 max_value =
        {
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max()
        };

        bool operator==(const std::string& name)
        {
            return this->name == name;
        }

        //このノードが参照するライトのインデックス。-1 ならライトなし。
        int light = -1;
    };

    std::vector<Node> nodes;

    // ライトインデックスに対して、それを参照しているノードの一覧を保持するマップ。
    std::unordered_map<int/*light*/, std::vector<int>/*nodes*/> lightNodesBridge;

    struct Vertex
    {
        DirectX::XMFLOAT3 position = { 0, 0, 0 };   // 頂点の位置座標（ローカル空間）
        DirectX::XMFLOAT3 normal = { 0, 0, 1 };     // 法線ベクトル。ライティング計算に使用される。
        DirectX::XMFLOAT4 tangent = { 1, 0, 0, 1 }; // 接線ベクトル（法線マップで使われる）。最後の w はbitangentの向き。
        DirectX::XMFLOAT2 texcoord = { 0, 0 };      // テクスチャ座標（UV）。画像の貼り付け位置。
    };

    struct Rig
    {
        DirectX::XMUINT4 joints = { 0, 0, 0, 0 };
        DirectX::XMFLOAT4 weights = { 1, 0, 0, 0 };
    };

    struct IndexBufferView
    {
        int buffer = -1;
        UINT sizeInBytes = 0;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    };

    struct VertexBufferView
    {
        int buffer = -1;
        UINT sizeInBytes = 0;
        UINT strideInBytes = 0;
    };

    struct Primitive
    {
        int material = -1;  // 使用するマテリアルのインデックス（-1は未指定）
        // 三角形、線、パッチなど、描画プリミティブの種類
        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        // ローカル座標系でのAABB（バウンディングボックス）
        DirectX::XMFLOAT3 minValue =
        {
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        };

        DirectX::XMFLOAT3 maxValue =
        {
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max()
        };

        // 頂点属性名（"POSITION", "NORMAL"など）とそのフォーマット（DXGI_FORMAT）の対応
        std::unordered_map<std::string, DXGI_FORMAT> attributes;

        // 指定した属性を持っているか確認するユーティリティ関数
        bool Has(const char* attribute)const
        {
            return attributes.find(attribute) != attributes.end();
        }

        // インデックス数の取得
        UINT indexCount() const
        {
            return indexBufferView.sizeInBytes / _SizeofComponent(indexBufferView.format);
        }

        // 頂点数の取得
        UINT vertexCount() const
        {
            return vertexBufferView.sizeInBytes / vertexBufferView.strideInBytes;
        }

        VertexBufferView vertexBufferView;
        VertexBufferView rigBufferView;
        IndexBufferView indexBufferView;

        std::vector<unsigned char> cachedIndices;
        std::vector<Vertex> cachedVertices;
        std::vector<Rig> cachedRigs;
    };

    struct Mesh
    {
        std::string name;
        std::vector<Primitive> primitives;
    };
    std::vector<Mesh> meshes;

    struct BatchMesh
    {
        int material;

        std::vector<UINT> cachedIndices;    // CPU
        IndexBufferView indexBufferView;    // GPU

        std::vector<Vertex> cachedVertices; // CPU 頂点データ
        VertexBufferView vertexBufferView;  // GPU 頂点バッファ

        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        std::unordered_map<std::string, DXGI_FORMAT> attributes;    // 頂点属性情報

        // 指定した属性を持っているか確認するユーティリティ関数
        bool Has(const char* attribute)const
        {
            return attributes.find(attribute) != attributes.end();
        }

        // インデックス数の取得
        UINT indexCount() const
        {
            return indexBufferView.sizeInBytes / _SizeofComponent(indexBufferView.format);
        }

        // 頂点数の取得
        UINT vertexCount() const
        {
            return vertexBufferView.sizeInBytes / vertexBufferView.strideInBytes;
        }
    };
    // バッチ最適化されたメッシュ群
    std::vector<BatchMesh> batchMeshes;
    // 作成されたGPUバッファの一覧（頂点・インデックス・スキンなど）
    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> buffers;

    // モデル全体のAABB（グローバル座標系）
    DirectX::XMFLOAT3 minValue =
    {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };
    DirectX::XMFLOAT3 maxValue =
    {
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max()
    };

    // スケルタルアニメーションに必要な情報
    struct Skin
    {
        // inverseBindMatrices によって、メッシュの頂点をボーンのローカル空間に戻すための変換が行われる
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices;
        std::vector<int> joints;
    };
    std::vector<Skin> skins;
    // 複数のスキンを格納。glTFではメッシュがこれらのスキンを参照可能。

    struct Animation
    {
        std::string name;
        float duration = 0.0f;

        // アニメーションのチャンネル：どのノードのどのパラメータをアニメーションさせるか
        struct Channel
        {
            int sampler = -1;       // 対応するサンプラーのインデックス（必須）
            int targetNode = -1;    // 対象のノードのインデックス（必須）
            std::string targetPath; // アニメーション対象のパス（"translation", "rotation", "scale", "weights"）
        };
        std::vector<Channel> channels;  // 複数のチャンネルを持つ（例：同一ノードに回転と移動）
        
        // キーフレーム間の補間処理と、入力／出力データのインデックス
        struct Sampler
        {
            int input = -1;      // 入力（時間）のアクセッサインデックス
            int output = -1;     // 出力（値）のアクセッサインデックス
            std::string interpolation;  // 補間方法（"LINEAR", "STEP", "CUBICSPLINE"など）
        };
        std::vector<Sampler> samplers;  // 複数の補間方法に対応可能

        // キーフレームごとの時間情報（node_index → 時間配列）
        std::unordered_map<int, std::vector<float>> keyframeTimestamps;
        // 各ノードごとのスケールのキーフレーム
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> keyframeScales;
        // 各ノードごとの回転のキーフレーム（クォータニオン）
        std::unordered_map<int, std::vector<DirectX::XMFLOAT4>> keyframeRotations;
        // 各ノードごとの平行移動（位置）のキーフレーム
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> keyframeTranslations;
    };
    std::vector<Animation> animations;

    // KHR_texture_transformは glTF の公式拡張機能で、
    // テクスチャの UV 座標のオフセット・回転・スケーリングを可能にする
    struct KhrTextureTransform
    {
        float offset[2] = { 0,0 };
        float rotation = 0;
        float scale[2] = { 1,1 };
        int texcoord = 0;

        void Init(const tinygltf::ExtensionMap& extensions)
        {
            // 拡張マップから "KHR_texture_transform" を探す
            auto extension = extensions.find("KHR_texture_transform");
            if (extension != extensions.end())
            {
                // offset が存在する場合
                if (extension->second.Has("offset"))
                {
                    auto& value = extension->second.Get("offset");
                    offset[0]=static_cast<float>(value.Get<tinygltf::Value::Array>().at(0).GetNumberAsDouble());
                    offset[1]=static_cast<float>(value.Get<tinygltf::Value::Array>().at(1).GetNumberAsDouble());
                }
            }
            // rotation が存在する場合
            if (extension->second.Has("rotation"))
            {
                auto& value = extension->second.Get("rotation");
                rotation = static_cast<float>(value.GetNumberAsDouble());
            }
            // scale が存在する場合
            if (extension->second.Has("scale"))
            {
                auto& value = extension->second.Get("scale");
                scale[0] = static_cast<float>(value.Get<tinygltf::Value::Array>().at(0).GetNumberAsDouble());
                scale[1] = static_cast<float>(value.Get<tinygltf::Value::Array>().at(1).GetNumberAsDouble());
            }
            // texCoord が存在する場合（指定があれば使用）
            if (extension->second.Has("texCoord"))
            {
                auto& value = extension->second.Get("texCoord");
                texcoord = value.GetNumberAsInt();
            }
        }
    };

    // メッシュとそれを使用しているノードの対応付け
    std::unordered_map<int/*mesh*/, std::vector<int>/*nodes*/> meshNodesBridge;


};
#endif // !MODEL_H