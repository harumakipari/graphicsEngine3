#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tiny_gltf.h"
#include "GltfModelBase.h"

class GltfModel :public GltfModelBase
{
public:
    struct Skin
    {
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices;
        std::vector<int> joints;
    };
    std::vector<Skin> skins;

    //ボーン行列の構造体と定数バッファ
    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJointCbuffer;

public:
    GltfModel(ID3D11Device* device, const std::string& filename);
    virtual ~GltfModel() = default;

    // アニメーションのノードを取得する関数        //戻り値      使ってない
    //std::vector<Node> GetAnimationNodes(size_t animationIndex, float time, bool loop)
    //{
    //    std::vector<Node> animatedNodes = nodes;
    //    Animate(animationIndex, time, animatedNodes, loop);
    //    return animatedNodes;
    //}

    void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes, RenderPass pass);

    //GLTFMODELBASEと統一したRender
    void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, RenderPass pass)override
    {
        Render(immediateContext, world, nodes, pass);
    }

    void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes)override;

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes)override;

    //アニメーションを追加する関数
    void AddAnimation(const std::string& filename)override;

    //アニメーションをブレンドする関数
    void BlendAnimations(const std::vector<Node>& from_nodes, const std::vector<Node>& to_nodes, float factor, std::vector<Node>& out_nodes);

    //モデルのジョイントのpositionを返す関数
    DirectX::XMFLOAT3 JointPosition(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)override;

    //モデルのジョイントのX軸方向のベクトル関数
    DirectX::XMFLOAT3 GetJointRightVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)override;

    //モデルのジョイントのY軸方向のベクトル関数
    DirectX::XMFLOAT3 GetJointUpVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)override;

    //モデルのジョイントのZ軸方向のベクトル関数
    DirectX::XMFLOAT3 GetJointForwardVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)override;

    //モデルのジョイントのワールド変換行列を返す関数
    DirectX::XMFLOAT4X4 GetJointTransform(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)override;
private:
    void FetchNodes(const tinygltf::Model& gltfModel);

    void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchAnimations(const tinygltf::Model& gltfModel);


    GltfModel::BufferView MakeBufferView(const tinygltf::Accessor& accessor);


public:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 color;
        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        int pad;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer;
};
