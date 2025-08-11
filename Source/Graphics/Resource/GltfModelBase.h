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

//SceneRenderer�Őݒ肵�Ă���
enum class RenderPass
{
    Opaque,// �s�����I�u�W�F�N�g
    Mask, // �}�X�N����
    Blend,// �������I�u�W�F�N�g
};


// glTF���f���̊�{�N���X
class GltfModelBase
{
protected:
    std::string filename;// ���f���t�@�C���̖��O

    //�}�e���A�����Ƃ�blendState��ݒ肷�邽��
    //std::unique_ptr<RenderState> renderState;

public:
    // �V�[���f�[�^�̍\����
    struct Scene
    {
        std::string name;// �V�[����
        std::vector<int> nodes;// ���[�g�m�[�h�̃C���f�b�N�X�z��i�V�[�����̍ŏ�ʃm�[�h�j
    };
    std::vector<Scene> scenes;//�V�[���̃��X�g

    int defaultScene = 0;

    // �m�[�h�i�I�u�W�F�N�g�j�̍\����
    struct Node
    {
        std::string name;//�m�[�h��
        int skin{ -1 };  // ���̃m�[�h���Q�Ƃ���X�L���̃C���f�b�N�X
        int mesh{ -1 }; // ���̃m�[�h���Q�Ƃ��郁�b�V���̃C���f�b�N�X

        std::vector<int> children; // �q�m�[�h�̃C���f�b�N�X�z��

        // �e���猩���q�̏ꏊ���[�J���g�����X�t�H�[���i�m�[�h���g�̉�]�A�X�P�[���A�ړ��j
        DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
        DirectX::XMFLOAT3 translation{ 0,0,0 };

        // �O���[�o���g�����X�t�H�[���i�����l�͒P�ʍs��j
        DirectX::XMFLOAT4X4 globalTransform
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    };
    std::vector<Node> nodes; // �m�[�h�̃��X�g

    // �o�b�t�@�r���[�i���_/�C���f�b�N�X�o�b�t�@�j
    struct BufferView
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN; // �o�b�t�@�̃t�H�[�}�b�g
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;// Direct3D�o�b�t�@
        std::vector<BYTE> row_data; // �o�b�t�@�̐��f�[�^
        size_t strideInBytes{ 0 };// 1���_������̃o�C�g��
        size_t sizeInBytes{ 0 };// �o�b�t�@�S�̂̃o�C�g��
        size_t count() const
        {
            return sizeInBytes / strideInBytes; // �v�f���̌v�Z
        }
    };
    // ���b�V���f�[�^�̍\����
    struct Mesh
    {
        std::string name;// ���b�V����
        struct Primitive
        {
            int material;// �}�e���A���̃C���f�b�N�X
            std::map<std::string, BufferView> vertexBufferViews;// ���_�o�b�t�@�r���[
            BufferView indexBufferView;// �C���f�b�N�X�o�b�t�@�r���[
        };
        std::vector<Primitive> primitives;// �v���~�e�B�u�̃��X�g
    };
    std::vector<Mesh> meshes; // ���b�V���̃��X�g

    // �e�N�X�`�����̍\����
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
    // PBR�p�̃}�e���A���f�[�^
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
    // �摜�f�[�^�̍\����
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
    std::vector<Image> images;// �摜�f�[�^�̃��X�g
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews;// �V�F�[�_�[���\�[�X�r���[

    // �}�e���A���f�[�^�̍\����
    struct Material
    {
        std::string name;// �}�e���A����
        Microsoft::WRL::ComPtr<ID3D11PixelShader> replacedPixelShader{ nullptr };// �J�X�^���V�F�[�_�[
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
    std::vector<Material> materials;// �}�e���A���̃��X�g
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;

    //GLTFMODEL�̂ݎg�p
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
    // �m�[�h�̊K�w�\�����l�����Ȃ���e�m�[�h�̃O���[�o���g�����X�t�H�[����ݐόv�Z
    virtual void CumulateTransforms(std::vector<Node>& nodes);

    // CascadedShadowMaps
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShaderCSM;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShaderCSM;

public:
    //�f�t�H���g�R���X�g���N�^
    GltfModelBase() = default;

    GltfModelBase(ID3D11Device* device, const std::string& filename) : filename(filename)
    {
        //renderState = std::make_unique<decltype(renderState)::element_type>(device);
    }

    virtual ~GltfModelBase() = default;

    // ���f���̕`��֐�
    virtual void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, RenderPass pass) = 0;


    //------------------GLTFMODEL�ł̂ݎg�p�̂��߃t�b�N�ɂ��Ă���-------------------//

    // �V���h�E�p�̕`��֐�
    virtual void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes) {};

    //�A�j���[�V������K������֐��@
    virtual void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes) {}

    //�A�j���[�V������ǉ�����֐�
    virtual void AddAnimation(const std::string& filename) {}

    //�A�j���[�V�������u�����h����֐�
    virtual void BlendAnimations(const std::vector<Node>& from_nodes, const std::vector<Node>& to_nodes, float factor, std::vector<Node>& out_nodes) {}

    //�A�j���[�V�����̃m�[�h�p�̕`�揈��     
    virtual void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes, RenderPass pass) {}

    //���f���̃W���C���g��position��Ԃ��֐�
    virtual DirectX::XMFLOAT3 JointPosition(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //���f���̃W���C���g��X�������̃x�N�g���֐�
    virtual DirectX::XMFLOAT3 GetJointRightVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //���f���̃W���C���g��Y�������̃x�N�g���֐�
    virtual DirectX::XMFLOAT3 GetJointUpVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //���f���̃W���C���g��Z�������̃x�N�g���֐�
    virtual DirectX::XMFLOAT3 GetJointForwardVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT3(0, 0, 0); }

    //���f���̃W���C���g�̃��[���h�ϊ��s���Ԃ��֐�
    virtual DirectX::XMFLOAT4X4 GetJointTransform(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform) { return DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); };
};