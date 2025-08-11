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
    // �R���X�g���N�^: GLTF���f����ǂݍ���
    GltfModelStaticBatching(ID3D11Device* device, const std::string& filename);
    virtual ~GltfModelStaticBatching() = default;

    // �v���~�e�B�u�i���f���̊e�p�[�c�j��\���\����
    struct Primitive
    {
        int material;// �g�p����}�e���A���̃C���f�b�N�X
        // ���_�o�b�t�@�̃r���[���i�[����}�b�v�i�L�[�͒��_�������j
        std::map<std::string, BufferView> vertexBufferViews;
        BufferView indexBufferView; // �C���f�b�N�X�o�b�t�@�̃r���[
    };
    // �o�b�`�������ꂽ�o�b�t�@��\���\����
    struct CombinedBuffer
    {
        size_t index_count;// �C���f�b�N�X��
        size_t vertex_count;// ���_��

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
    {//�A�j���[�V�������Ȃ���renderer�Ŏg�p���邽��
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