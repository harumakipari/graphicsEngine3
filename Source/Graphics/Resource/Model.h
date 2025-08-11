#ifndef MODEL_H
#define MODEL_H

#define NOMINMAX // max ���ʎq���������邩��

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
        _ASSERT_EXPR(models_.find(filename) != models_.end(), L"���̃t�@�C�����Ń��f���͓o�^����Ă��܂���B");
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

    // �}�e���A���� variants
    std::vector<std::string> variants;

    struct Scene
    {
        std::string name;
        std::vector<int> nodes;// ���[�g�m�[�h�̔z��
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

        // ���[�J���̃g�����X�t�H�[��
        DirectX::XMFLOAT4 rotation = { 0,0,0,1 };
        DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
        DirectX::XMFLOAT3 translation = { 0,0,0 };
        std::vector<double> weights; // ���[�t�^�[�Q�b�g�̏d��

        DirectX::XMFLOAT4X4 globalTransform =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1,
        };

        // �O���[�o����Ԃł� AABB�i�����s���E�{�b�N�X�j�ŏ��E�ő���W
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

        //���̃m�[�h���Q�Ƃ��郉�C�g�̃C���f�b�N�X�B-1 �Ȃ烉�C�g�Ȃ��B
        int light = -1;
    };

    std::vector<Node> nodes;

    // ���C�g�C���f�b�N�X�ɑ΂��āA������Q�Ƃ��Ă���m�[�h�̈ꗗ��ێ�����}�b�v�B
    std::unordered_map<int/*light*/, std::vector<int>/*nodes*/> lightNodesBridge;

    struct Vertex
    {
        DirectX::XMFLOAT3 position = { 0, 0, 0 };   // ���_�̈ʒu���W�i���[�J����ԁj
        DirectX::XMFLOAT3 normal = { 0, 0, 1 };     // �@���x�N�g���B���C�e�B���O�v�Z�Ɏg�p�����B
        DirectX::XMFLOAT4 tangent = { 1, 0, 0, 1 }; // �ڐ��x�N�g���i�@���}�b�v�Ŏg����j�B�Ō�� w ��bitangent�̌����B
        DirectX::XMFLOAT2 texcoord = { 0, 0 };      // �e�N�X�`�����W�iUV�j�B�摜�̓\��t���ʒu�B
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
        int material = -1;  // �g�p����}�e���A���̃C���f�b�N�X�i-1�͖��w��j
        // �O�p�`�A���A�p�b�`�ȂǁA�`��v���~�e�B�u�̎��
        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        // ���[�J�����W�n�ł�AABB�i�o�E���f�B���O�{�b�N�X�j
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

        // ���_�������i"POSITION", "NORMAL"�Ȃǁj�Ƃ��̃t�H�[�}�b�g�iDXGI_FORMAT�j�̑Ή�
        std::unordered_map<std::string, DXGI_FORMAT> attributes;

        // �w�肵�������������Ă��邩�m�F���郆�[�e�B���e�B�֐�
        bool Has(const char* attribute)const
        {
            return attributes.find(attribute) != attributes.end();
        }

        // �C���f�b�N�X���̎擾
        UINT indexCount() const
        {
            return indexBufferView.sizeInBytes / _SizeofComponent(indexBufferView.format);
        }

        // ���_���̎擾
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

        std::vector<Vertex> cachedVertices; // CPU ���_�f�[�^
        VertexBufferView vertexBufferView;  // GPU ���_�o�b�t�@

        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        std::unordered_map<std::string, DXGI_FORMAT> attributes;    // ���_�������

        // �w�肵�������������Ă��邩�m�F���郆�[�e�B���e�B�֐�
        bool Has(const char* attribute)const
        {
            return attributes.find(attribute) != attributes.end();
        }

        // �C���f�b�N�X���̎擾
        UINT indexCount() const
        {
            return indexBufferView.sizeInBytes / _SizeofComponent(indexBufferView.format);
        }

        // ���_���̎擾
        UINT vertexCount() const
        {
            return vertexBufferView.sizeInBytes / vertexBufferView.strideInBytes;
        }
    };
    // �o�b�`�œK�����ꂽ���b�V���Q
    std::vector<BatchMesh> batchMeshes;
    // �쐬���ꂽGPU�o�b�t�@�̈ꗗ�i���_�E�C���f�b�N�X�E�X�L���Ȃǁj
    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> buffers;

    // ���f���S�̂�AABB�i�O���[�o�����W�n�j
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

    // �X�P���^���A�j���[�V�����ɕK�v�ȏ��
    struct Skin
    {
        // inverseBindMatrices �ɂ���āA���b�V���̒��_���{�[���̃��[�J����Ԃɖ߂����߂̕ϊ����s����
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices;
        std::vector<int> joints;
    };
    std::vector<Skin> skins;
    // �����̃X�L�����i�[�BglTF�ł̓��b�V���������̃X�L�����Q�Ɖ\�B

    struct Animation
    {
        std::string name;
        float duration = 0.0f;

        // �A�j���[�V�����̃`�����l���F�ǂ̃m�[�h�̂ǂ̃p�����[�^���A�j���[�V���������邩
        struct Channel
        {
            int sampler = -1;       // �Ή�����T���v���[�̃C���f�b�N�X�i�K�{�j
            int targetNode = -1;    // �Ώۂ̃m�[�h�̃C���f�b�N�X�i�K�{�j
            std::string targetPath; // �A�j���[�V�����Ώۂ̃p�X�i"translation", "rotation", "scale", "weights"�j
        };
        std::vector<Channel> channels;  // �����̃`�����l�������i��F����m�[�h�ɉ�]�ƈړ��j
        
        // �L�[�t���[���Ԃ̕�ԏ����ƁA���́^�o�̓f�[�^�̃C���f�b�N�X
        struct Sampler
        {
            int input = -1;      // ���́i���ԁj�̃A�N�Z�b�T�C���f�b�N�X
            int output = -1;     // �o�́i�l�j�̃A�N�Z�b�T�C���f�b�N�X
            std::string interpolation;  // ��ԕ��@�i"LINEAR", "STEP", "CUBICSPLINE"�Ȃǁj
        };
        std::vector<Sampler> samplers;  // �����̕�ԕ��@�ɑΉ��\

        // �L�[�t���[�����Ƃ̎��ԏ��inode_index �� ���Ԕz��j
        std::unordered_map<int, std::vector<float>> keyframeTimestamps;
        // �e�m�[�h���Ƃ̃X�P�[���̃L�[�t���[��
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> keyframeScales;
        // �e�m�[�h���Ƃ̉�]�̃L�[�t���[���i�N�H�[�^�j�I���j
        std::unordered_map<int, std::vector<DirectX::XMFLOAT4>> keyframeRotations;
        // �e�m�[�h���Ƃ̕��s�ړ��i�ʒu�j�̃L�[�t���[��
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> keyframeTranslations;
    };
    std::vector<Animation> animations;

    // KHR_texture_transform�� glTF �̌����g���@�\�ŁA
    // �e�N�X�`���� UV ���W�̃I�t�Z�b�g�E��]�E�X�P�[�����O���\�ɂ���
    struct KhrTextureTransform
    {
        float offset[2] = { 0,0 };
        float rotation = 0;
        float scale[2] = { 1,1 };
        int texcoord = 0;

        void Init(const tinygltf::ExtensionMap& extensions)
        {
            // �g���}�b�v���� "KHR_texture_transform" ��T��
            auto extension = extensions.find("KHR_texture_transform");
            if (extension != extensions.end())
            {
                // offset �����݂���ꍇ
                if (extension->second.Has("offset"))
                {
                    auto& value = extension->second.Get("offset");
                    offset[0]=static_cast<float>(value.Get<tinygltf::Value::Array>().at(0).GetNumberAsDouble());
                    offset[1]=static_cast<float>(value.Get<tinygltf::Value::Array>().at(1).GetNumberAsDouble());
                }
            }
            // rotation �����݂���ꍇ
            if (extension->second.Has("rotation"))
            {
                auto& value = extension->second.Get("rotation");
                rotation = static_cast<float>(value.GetNumberAsDouble());
            }
            // scale �����݂���ꍇ
            if (extension->second.Has("scale"))
            {
                auto& value = extension->second.Get("scale");
                scale[0] = static_cast<float>(value.Get<tinygltf::Value::Array>().at(0).GetNumberAsDouble());
                scale[1] = static_cast<float>(value.Get<tinygltf::Value::Array>().at(1).GetNumberAsDouble());
            }
            // texCoord �����݂���ꍇ�i�w�肪����Ύg�p�j
            if (extension->second.Has("texCoord"))
            {
                auto& value = extension->second.Get("texCoord");
                texcoord = value.GetNumberAsInt();
            }
        }
    };

    // ���b�V���Ƃ�����g�p���Ă���m�[�h�̑Ή��t��
    std::unordered_map<int/*mesh*/, std::vector<int>/*nodes*/> meshNodesBridge;


};
#endif // !MODEL_H