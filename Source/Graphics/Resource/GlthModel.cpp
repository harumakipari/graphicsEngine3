#include "GltfModel.h"
#define TINYGLTF_IMPLEMENTATION 

#include <stack>
#include <functional>
#include "DDSTextureLoader.h"
#include "tiny_gltf.h"
#include "Engine/Utility/Win32Utils.h"
#include "Engine/Debug/Assert.h"
#include "Graphics/Core/Shader.h"
#include "Texture.h"
#include <cstddef>
#include <filesystem>

DXGI_FORMAT ConvertFormat(const tinygltf::Accessor& accessor)
{
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return DXGI_FORMAT_R8_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return DXGI_FORMAT_R16_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return DXGI_FORMAT_R32_UINT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    default:
        _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
        return DXGI_FORMAT_UNKNOWN;
    }
}

bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
    return true;
}

GltfModel::GltfModel(ID3D11Device* device, const std::string& filename) :GltfModelBase(device, filename)
{
    HRESULT hr{ S_OK };

    tinygltf::TinyGLTF tinyGltf;
    tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

    tinygltf::Model gltfModel;
    std::string error, warning;
    bool succeeded{ false };
    if (filename.find(".glb") != std::string::npos)
    {
        succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
    }
    else if (filename.find("gltf") != std::string::npos)
    {
        succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
    }

    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), error.c_str());
    _ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

    for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
    {
        Scene& scene{ scenes.emplace_back() };
        scene.name = gltfScene.name;
        scene.nodes = gltfScene.nodes;
    }

    defaultScene = gltfModel.defaultScene;

    FetchNodes(gltfModel);
    FetchMeshes(device, gltfModel);
    FetchMaterials(device, gltfModel);
    FetchTextures(device, gltfModel);
    FetchAnimations(gltfModel);

    //シェーダーオブジェクトの生成を行う
    //※頂点バッファはSoA (Structure of Array)で構成される（従来はAoS (Array of Structures)）
    // TODO: This is a force-brute programming, may cause bugs.
    const std::map<std::string, BufferView>& vertexBufferViews{ meshes.at(0).primitives.at(0).vertexBufferViews };
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        {"POSITION",0,vertexBufferViews.at("POSITION").format,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,vertexBufferViews.at("NORMAL").format,1,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TANGENT",0,vertexBufferViews.at("TANGENT").format,2,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,vertexBufferViews.at("TEXCOORD_0").format,3,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"JOINTS",0,vertexBufferViews.at("JOINTS_0").format,4,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"JOINTS",1,vertexBufferViews.at("JOINTS_1").format,5,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"WEIGHTS",0,vertexBufferViews.at("WEIGHTS_0").format,6,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"WEIGHTS",1,vertexBufferViews.at("WEIGHTS_1").format,7,0,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    hr = CreateVsFromCSO(device, "./Shader/GltfModelVS.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", pixelShader.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // CASCADED_SHADOW_MAPS
    hr = CreateVsFromCSO(device, "./Shader/GltfModelCsmVS.cso", vertexShaderCSM.ReleaseAndGetAddressOf(), NULL, NULL, 0);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = CreateGsFromCSO(device, "./Shader/GltfModelCsmGS.cso", geometryShaderCSM.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //hr=CreateVsFromCSO(device,)

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //ボーン行列の定数バッファを生成
    bufferDesc.ByteWidth = sizeof(PrimitiveJointConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveJointCbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

#if 0
class ShaderManager
{
public:
    struct ShdaerSets
    {
        ID3D11VertexShader* vertex_shader;
        ID3D11PixelShader* pixel_shader;
    };

    std::map<std::string, ShdaerSets> shaders;

    void Register(std::string name, ShdaerSets shader)
    {
        shaders.insert_or_assign(name, shader);
    }

    ShdaerSets Search(std::string name)
    {
        return shaders.find(name)->second;
    }
};

ShaderManager::Search(shader_name)
#endif

void GltfModel::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes, RenderPass pass)
{

    const std::vector<Node>& nodes{ animatedNodes.size() > 0 ? animatedNodes : GltfModel::nodes };

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);

    //immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::function<void(int)> traverse{ [&](int nodeIndex)->void
    {
            const Node& node{nodes.at(nodeIndex)};
    if (node.skin > -1)
    {
        const Skin& skin{ skins.at(node.skin) };
        PrimitiveJointConstants primitiveJointData{};
        for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex)
        {
            DirectX::XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) * DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) * DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform)));
        }
        immediateContext->UpdateSubresource(primitiveJointCbuffer.Get(), 0, 0, &primitiveJointData, 0, 0);
        immediateContext->VSSetConstantBuffers(2, 1, primitiveJointCbuffer.GetAddressOf());
    }
    if (node.mesh > -1)
    {
        const Mesh& mesh{ meshes.at(node.mesh) };
        for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
        {
            ID3D11Buffer* vertexBuffers[]
            {
                primitive.vertexBufferViews.at("POSITION").buffer.Get(),
                primitive.vertexBufferViews.at("NORMAL").buffer.Get(),
                primitive.vertexBufferViews.at("TANGENT").buffer.Get(),
                primitive.vertexBufferViews.at("TEXCOORD_0").buffer.Get(),
                primitive.vertexBufferViews.at("JOINTS_0").buffer.Get(),
                primitive.vertexBufferViews.at("JOINTS_1").buffer.Get(),
                primitive.vertexBufferViews.at("WEIGHTS_0").buffer.Get(),
                primitive.vertexBufferViews.at("WEIGHTS_1").buffer.Get(),
            };
            UINT strides[]
            {
                static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_1").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_1").strideInBytes),
            };
            UINT offsets[_countof(vertexBuffers)]{ 0 };
            immediateContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
            immediateContext->IASetIndexBuffer(primitive.indexBufferView.buffer.Get(), primitive.indexBufferView.format, 0);

            //CPU側のデータ
            PrimitiveConstants primitiveData{};
            primitiveData.material = primitive.material;
            primitiveData.hasTangent = primitive.vertexBufferViews.at("TANGENT").buffer != NULL;
            primitiveData.skin = node.skin;
            DirectX::XMStoreFloat4x4(&primitiveData.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&world));
            immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
            immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

            const Material& material{ materials.at(primitive.material) };
            //ここで設定
            if (material.replacedPixelShader)
            {
                immediateContext->PSSetShader(material.replacedPixelShader.Get(), nullptr, 0);
            }
            else
            {
                immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
            }

            bool passed = false;
            switch (pass)
            {
            case RenderPass::Opaque:
                if (material.data.alphaMode == 0/*OPAQUE*/)
                {
                    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE);
                    passed = true;
                }
                break;
            case RenderPass::Mask:
                if (material.data.alphaMode == 1/*MASK*/)
                {
                    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE);
                    passed = true;
                }
                break;
            case RenderPass::Blend:
                if (material.data.alphaMode == 2/*BLEND*/)
                {
                    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
                    passed = true;
                }
                break;
            }
            if (!passed)
            {
                continue;
            }

            //if (material.data.doubleSided)
            //{
            //    renderState->BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
            //}
            //else
            //{
            //    renderState->BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_BACK);
            //}

            const int textureIndices[]
            {
                material.data.pbrMetallicRoughness.basecolorTexture.index,
                material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
                material.data.normalTexture.index,
                material.data.emissiveTexture.index,
                material.data.occlusionTexture.index,
            };

            ID3D11ShaderResourceView* nullShaderResourceView{};
            std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
            for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
            {
                shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
            }
            immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());


            immediateContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.count()), 0, 0);
        }


    }
    for (std::vector<int>::value_type childIndex : node.children)
    {
        traverse(childIndex);
    }
    } };

    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        traverse(nodeIndex);
    }
}

void GltfModel::CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes)
{
    const std::vector<Node>& nodes{ animatedNodes.size() > 0 ? animatedNodes : GltfModel::nodes };

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    // CASCADED_SHADOW_MAPS

    immediateContext->VSSetShader(vertexShaderCSM.Get(), nullptr, 0);
    immediateContext->GSSetShader(geometryShaderCSM.Get(), nullptr, 0);

    Microsoft::WRL::ComPtr <ID3D11PixelShader> nullPixelShader{ NULL };
    immediateContext->PSSetShader(nullPixelShader.Get()/*SHADOW*/, nullptr, 0);


    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::function<void(int)> traverse{ [&](int nodeIndex)->void
        {const Node& node{nodes.at(nodeIndex)};
    if (node.skin > -1)
    {
        const Skin& skin{ skins.at(node.skin) };
        PrimitiveJointConstants primitiveJointData{};
        for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex)
        {
            DirectX::XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) * DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) * DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform)));
        }
        immediateContext->UpdateSubresource(primitiveJointCbuffer.Get(), 0, 0, &primitiveJointData, 0, 0);
        immediateContext->VSSetConstantBuffers(2, 1, primitiveJointCbuffer.GetAddressOf());
    }
    if (node.mesh > -1)
    {
        const Mesh& mesh{ meshes.at(node.mesh) };
        for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
        {
            ID3D11Buffer* vertexBuffers[]
            {
                primitive.vertexBufferViews.at("POSITION").buffer.Get(),
                primitive.vertexBufferViews.at("NORMAL").buffer.Get(),
                primitive.vertexBufferViews.at("TANGENT").buffer.Get(),
                primitive.vertexBufferViews.at("TEXCOORD_0").buffer.Get(),
                primitive.vertexBufferViews.at("JOINTS_0").buffer.Get(),
                primitive.vertexBufferViews.at("JOINTS_1").buffer.Get(),
                primitive.vertexBufferViews.at("WEIGHTS_0").buffer.Get(),
                primitive.vertexBufferViews.at("WEIGHTS_1").buffer.Get(),
            };
            UINT strides[]
            {
                static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_1").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_0").strideInBytes),
                static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_1").strideInBytes),
            };
            UINT offsets[_countof(vertexBuffers)]{ 0 };
            immediateContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
            immediateContext->IASetIndexBuffer(primitive.indexBufferView.buffer.Get(), primitive.indexBufferView.format, 0);

            //CPU側のデータ
            PrimitiveConstants primitiveData{};
            primitiveData.material = primitive.material;
            primitiveData.hasTangent = primitive.vertexBufferViews.at("TANGENT").buffer != NULL;
            primitiveData.skin = node.skin;
            DirectX::XMStoreFloat4x4(&primitiveData.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&world));
            immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
            immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

            const Material& material{ materials.at(primitive.material) };
            const int textureIndices[]
            {
                material.data.pbrMetallicRoughness.basecolorTexture.index,
                material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
                material.data.normalTexture.index,
                material.data.emissiveTexture.index,
                material.data.occlusionTexture.index,
            };
            ID3D11ShaderResourceView* nullShaderResourceView{};
            std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
            for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
            {
                shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
            }
            immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

            // CASCADED_SHADOW_MAPS
            immediateContext->DrawIndexedInstanced(static_cast<UINT>(primitive.indexBufferView.count()), 4, 0,0, 0);
            //immediateContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.count()), 0, 0);
        }
    }
    for (std::vector<int>::value_type childIndex : node.children)
    {
        traverse(childIndex);
    }
    } };
    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        traverse(nodeIndex);
    }

    immediateContext->VSSetShader(NULL, NULL, 0);
    immediateContext->GSSetShader(NULL, NULL, 0);
    immediateContext->PSSetShader(NULL, NULL, 0);
}

void GltfModel::Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes)
{
    _ASSERT_EXPR(animations.size() > animationIndex, L"");
    _ASSERT_EXPR(animatedNodes.size() == nodes.size(), L"");

    std::function<size_t(const std::vector<float>&, float, float&)> indexof
    {
        [](const std::vector<float>& timelines,float time,float& interpolationFactor)->size_t {
            const size_t keyframeCount{timelines.size()};
    if (time > timelines.at(keyframeCount - 1))
    {
        interpolationFactor = 1.0f;
        return keyframeCount - 2;
    }
    else if (time < timelines.at(0))
    {
        interpolationFactor = 0.0f;
        return 0;
    }
    size_t keyframeIndex{ 0 };
    for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex)
    {
        if (time < timelines.at(timeIndex))
        {
            keyframeIndex = std::max<size_t>(0LL, timeIndex - 1);
            break;
        }
    }
    interpolationFactor = (time - timelines.at(keyframeIndex + 0)) / (timelines.at(keyframeIndex + 1) - timelines.at(keyframeIndex + 0));
    return keyframeIndex;
    } };

    if (animations.size() > 0)
    {
        const Animation& animation{ animations.at(animationIndex) };
        for (std::vector<Animation::Channel>::const_reference channel : animation.channels)
        {
            const Animation::Sampler& sampler{ animation.samplers.at(channel.sampler) };
            const std::vector<float>& timeline{ animation.timelines.at(sampler.input) };
            if (timeline.size() == 0)
            {
                continue;
            }
            float interpolationFactor{};
            size_t keyframeIndex{ indexof(timeline, time, interpolationFactor) };
            if (channel.targetPath == "scale")
            {
                const std::vector<DirectX::XMFLOAT3>& scales{ animation.scales.at(sampler.output) };
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode).scale, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 0)), DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 1)), interpolationFactor));
            }
            else if (channel.targetPath == "rotation")
            {
                const std::vector<DirectX::XMFLOAT4>& rotations{ animation.rotations.at(sampler.output) };
                DirectX::XMStoreFloat4(&animatedNodes.at(channel.targetNode).rotation,
                    DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 1)), interpolationFactor)));
            }
            else if (channel.targetPath == "translation")
            {
                const std::vector<DirectX::XMFLOAT3>& translations{ animation.translations.at(sampler.output) };
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode).translation,
                    DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 1)), interpolationFactor));
            }
            else if (channel.targetPath == "weights")
            {

            }
            else
            {

            }
        }
        CumulateTransforms(animatedNodes);
    }
}

//モデルのジョイントのpositionを返す関数
DirectX::XMFLOAT3 GltfModel::JointPosition(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
{
    DirectX::XMFLOAT3 position = { 0,0,0 };
    const Node& node = animatedNodes.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);
    DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));
    return position;
}

//モデルのジョイントのX軸方向のベクトル関数
DirectX::XMFLOAT3 GltfModel::GetJointRightVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
{
    const Node& node = animatedNodes.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);

    //X軸方向のベクトル
    DirectX::XMFLOAT3 right;
    DirectX::XMStoreFloat3(&right, M.r[0]);
    return right;
}

//モデルのジョイントのY軸方向のベクトル関数
DirectX::XMFLOAT3 GltfModel::GetJointUpVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
{
    const Node& node = animatedNodes.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);

    //Y軸方向のベクトル
    DirectX::XMFLOAT3 up;
    DirectX::XMStoreFloat3(&up, M.r[1]);    //Y2行目
    return up;
}

//モデルのジョイントのZ軸方向のベクトル関数
DirectX::XMFLOAT3 GltfModel::GetJointForwardVector(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
{
    const Node& node = animatedNodes.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);

    //X軸方向のベクトル
    DirectX::XMFLOAT3 forward;
    DirectX::XMStoreFloat3(&forward, M.r[2]);
    return forward;
}

//モデルのジョイントのワールド変換行列を返す関数
DirectX::XMFLOAT4X4 GltfModel::GetJointTransform(size_t nodeIndex, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
{
    const Node& node = animatedNodes.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);
    DirectX::XMFLOAT4X4 matrix;
    DirectX::XMStoreFloat4x4(&matrix, M);
    return matrix;
}

//アニメーションを追加する関数
void GltfModel::AddAnimation(const std::string& filename)
{
    tinygltf::TinyGLTF tinyGltf;
    tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

    tinygltf::Model gltfModel;
    std::string error, warning;
    bool succeeded{ false };
    if (filename.find(".glb") != std::string::npos)
    {
        succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
    }
    else if (filename.find("gltf") != std::string::npos)
    {
        succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
    }

    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), error.c_str());
    _ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

    for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
    {
        Scene& scene{ scenes.emplace_back() };
        scene.name = gltfScene.name;
        scene.nodes = gltfScene.nodes;
    }
    FetchAnimations(gltfModel);
}

//アニメーションをブレンドする関数
void GltfModel::BlendAnimations(const std::vector<Node>& fromNodes, const std::vector<Node>& toNodes, float factor, std::vector<Node>& outNodes)
{
    using namespace DirectX;
    _ASSERT_EXPR(fromNodes.size() == toNodes.size(), "The size of the two node arrays must be the same.");

    size_t nodeCount{ fromNodes.size() };
    _ASSERT_EXPR(nodeCount == outNodes.size(), "The size of output nodes must be input nodes.");
    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        XMVECTOR S[2]{ XMLoadFloat3(&fromNodes.at(nodeIndex).scale), XMLoadFloat3(&toNodes.at(nodeIndex).scale) };
        XMStoreFloat3(&outNodes.at(nodeIndex).scale, XMVectorLerp(S[0], S[1], factor));

        XMVECTOR R[2]{ XMLoadFloat4(&fromNodes.at(nodeIndex).rotation), XMLoadFloat4(&toNodes.at(nodeIndex).rotation) };
        XMStoreFloat4(&outNodes.at(nodeIndex).rotation, XMQuaternionSlerp(R[0], R[1], factor));

        XMVECTOR T[2]{ XMLoadFloat3(&fromNodes.at(nodeIndex).translation), XMLoadFloat3(&toNodes.at(nodeIndex).translation) };
        XMStoreFloat3(&outNodes.at(nodeIndex).translation, XMVectorLerp(T[0], T[1], factor));
    }
    CumulateTransforms(outNodes);
}

void GltfModel::FetchNodes(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
    {
        Node& node{ nodes.emplace_back() };
        node.name = gltfNode.name;
        node.skin = gltfNode.skin;
        node.mesh = gltfNode.mesh;
        node.children = gltfNode.children;
        if (!gltfNode.matrix.empty())
        {
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++)
            {
                for (size_t column = 0; column < 4; column++)
                {
                    matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                }
            }
            DirectX::XMVECTOR S, T, R;
            bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

            DirectX::XMStoreFloat3(&node.scale, S);
            DirectX::XMStoreFloat4(&node.rotation, R);
            DirectX::XMStoreFloat3(&node.translation, T);
        }
        else
        {
            if (gltfNode.scale.size() > 0)
            {
                node.scale.x = static_cast<float>(gltfNode.scale.at(0));
                node.scale.y = static_cast<float>(gltfNode.scale.at(1));
                node.scale.z = static_cast<float>(gltfNode.scale.at(2));
            }
            if (gltfNode.translation.size() > 0)
            {
                node.translation.x = static_cast<float>(gltfNode.translation.at(0));
                node.translation.y = static_cast<float>(gltfNode.translation.at(1));
                node.translation.z = static_cast<float>(gltfNode.translation.at(2));
            }
            if (gltfNode.rotation.size() > 0)
            {
                node.rotation.x = static_cast<float>(gltfNode.rotation.at(0));
                node.rotation.y = static_cast<float>(gltfNode.rotation.at(1));
                node.rotation.z = static_cast<float>(gltfNode.rotation.at(2));
                node.rotation.w = static_cast<float>(gltfNode.rotation.at(3));
            }
        }
    }
    CumulateTransforms(nodes);
}

void GltfModel::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
    HRESULT hr;
    for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
    {
        Mesh& mesh{ meshes.emplace_back() };
        mesh.name = gltfMesh.name;
        for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
        {
            Mesh::Primitive& primitive{ mesh.primitives.emplace_back() };
            primitive.material = gltfPrimitive.material;

            //Create index buffer
            const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(gltfPrimitive.indices) };
            const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };

            primitive.indexBufferView = MakeBufferView(gltfAccessor);

            D3D11_BUFFER_DESC bufferDesc{};
            bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView.sizeInBytes);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData{};
            subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
            // メモリの確保
            //primitive.indexBufferView.row_data.resize(primitive.indexBufferView.sizeInBytes);
            // インデックスバッファの内容をrow_dataに保存したい
            //memcpy_s(primitive.indexBufferView.row_data.data(), primitive.indexBufferView.row_data.size(),
            //         subresourceData.pSysMem, primitive.indexBufferView.sizeInBytes);
            hr = device->CreateBuffer(&bufferDesc, &subresourceData, primitive.indexBufferView.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            //Create vertex buffer
            for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes)
            {
                tinygltf::Accessor gltfAccessor{ gltfModel.accessors.at(gltfAttribute.second) };
                const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };

                //gltfnodeの場所取得のため追加したところ
                const void* buffer = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                std::vector<USHORT> joints_0;
                std::vector<FLOAT> weights_0;
                if (gltfAttribute.first == "JOINTS_0" || gltfAttribute.first == "JOINTS_1")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            joints_0.emplace_back(static_cast<USHORT>(data[accessorIndex]));
                        }
                        buffer = joints_0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                    }
                }
                else if (gltfAttribute.first == "JOINTS_2")
                {
                    continue;
                }
                else if (gltfAttribute.first == "WEIGHTS_0" || gltfAttribute.first == "WEIGHTS_1")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            weights_0.emplace_back(static_cast<FLOAT>(data[accessorIndex]) / 0xFF);
                        }
                        buffer = weights_0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    }
                }
                else if (gltfAttribute.first == "WEIGHTS_2")
                {
                    continue;
                }

                BufferView vertexBufferView{ MakeBufferView(gltfAccessor) };

                D3D11_BUFFER_DESC bufferDesc{};
                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                D3D11_SUBRESOURCE_DATA subresourceData{};
                //subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                subresourceData.pSysMem = buffer;
                hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

                primitive.vertexBufferViews.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));
            }

            // Add dummy attributes if any are missing.
            const std::unordered_map<std::string, BufferView> attributes
            {
                {"TANGENT",{DXGI_FORMAT_R32G32B32A32_FLOAT}},
                {"TEXCOORD_0",{DXGI_FORMAT_R32G32_FLOAT}},
                {"JOINTS_0",{DXGI_FORMAT_R16G16B16A16_UINT}},
                {"JOINTS_1",{DXGI_FORMAT_R16G16B16A16_UINT}},
                {"WEIGHTS_0",{DXGI_FORMAT_R32G32B32A32_FLOAT}},
                {"WEIGHTS_1",{DXGI_FORMAT_R32G32B32A32_FLOAT}},
            };
            for (std::map<std::string, BufferView>::const_reference attribute : attributes)
            {
                if (primitive.vertexBufferViews.find(attribute.first) == primitive.vertexBufferViews.end())
                {
                    primitive.vertexBufferViews.insert(std::make_pair(attribute.first, attribute.second));
                }
            }
        }
    }
}


void  GltfModel::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
    {
        std::vector<Material>::reference material = materials.emplace_back();

        material.name = gltfMaterial.name;

        material.data.emissiveFactor[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
        material.data.emissiveFactor[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
        material.data.emissiveFactor[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

        material.data.pbrMetallicRoughness.basecolorFactor[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data.pbrMetallicRoughness.basecolorFactor[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data.pbrMetallicRoughness.basecolorFactor[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data.pbrMetallicRoughness.basecolorFactor[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data.pbrMetallicRoughness.basecolorTexture.index = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data.pbrMetallicRoughness.basecolorTexture.texcoord = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
        material.data.pbrMetallicRoughness.metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.data.pbrMetallicRoughness.roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.index = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.texcoord = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        if (gltfMaterial.alphaMode == "OPAQUE")
        {
            material.data.alphaMode = 0;
        }
        else if (gltfMaterial.alphaMode == "MASK")
        {
            material.data.alphaMode = 1;
        }
        else if (gltfMaterial.alphaMode == "BLEND")
        {
            material.data.alphaMode = 2;
        }

        material.data.alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
        material.data.doubleSided = gltfMaterial.doubleSided;

        material.data.normalTexture.index = gltfMaterial.normalTexture.index;
        material.data.normalTexture.texcoord = gltfMaterial.normalTexture.texCoord;
        material.data.normalTexture.scale = static_cast<float>(gltfMaterial.normalTexture.scale);

        material.data.occlusionTexture.index = gltfMaterial.occlusionTexture.index;
        material.data.occlusionTexture.texcoord = gltfMaterial.occlusionTexture.texCoord;
        material.data.occlusionTexture.strength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

        material.data.emissiveTexture.index = gltfMaterial.emissiveTexture.index;
        material.data.emissiveTexture.texcoord = gltfMaterial.emissiveTexture.texCoord;
    }

    // Create material data as shader resource view on GPU 
    std::vector<Material::Cbuffer> materialData;
    for (std::vector<Material>::const_reference material : materials)
    {
        materialData.emplace_back(material.data);
    }

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
    bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subsorceData{};
    subsorceData.pSysMem = materialData.data();
    hr = device->CreateBuffer(&bufferDesc, &subsorceData, materialBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
    hr = device->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void GltfModel::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
    HRESULT hr{ S_OK };
    for (const tinygltf::Texture& gltfTexture : gltfModel.textures)
    {
        Texture& texture{ textures.emplace_back() };
        texture.name = gltfTexture.name;
        texture.source = gltfTexture.source;
    }
    for (const tinygltf::Image& gltfImage : gltfModel.images)
    {
        Image& image{ images.emplace_back() };
        image.name = gltfImage.name;
        image.width = gltfImage.width;
        image.height = gltfImage.height;
        image.component = gltfImage.component;
        image.bits = gltfImage.bits;
        image.pixelType = gltfImage.pixel_type;
        image.bufferView = gltfImage.bufferView;
        image.mimeType = gltfImage.mimeType;
        image.uri = gltfImage.uri;
        image.asIs = gltfImage.as_is;

        if (gltfImage.bufferView > -1)
        {
            const tinygltf::BufferView& bufferView{ gltfModel.bufferViews.at(gltfImage.bufferView) };
            const tinygltf::Buffer& buffer{ gltfModel.buffers.at(bufferView.buffer) };
            //TODO:01 pdf 36 ②　28行目
            //const unsigned char* data = buffer.data.data() + bufferView.byteOffset;
            const std::byte* data = reinterpret_cast<const std::byte*>(buffer.data.data() + bufferView.byteOffset);
            //const unsigned char* data = buffer.data.data() + bufferView.byteOffset;
            ID3D11ShaderResourceView* textureResourceView{};
            hr = LoadTextureFromMemory(device, data, bufferView.byteLength, &textureResourceView);
            if (hr == S_OK)
            {
                textureResourceViews.emplace_back().Attach(textureResourceView);
            }
        }
        else
        {
            const std::filesystem::path path(filename);
            ID3D11ShaderResourceView* shaderResourceView{};
            D3D11_TEXTURE2D_DESC texture2dDesc;
            std::wstring filename{ path.parent_path().concat(L"/").wstring() + std::wstring(gltfImage.uri.begin(),gltfImage.uri.end()) };
            hr = LoadTextureFromFile(device, filename.c_str(), &shaderResourceView, &texture2dDesc);
            if (hr == S_OK)
            {
                textureResourceViews.emplace_back().Attach(shaderResourceView);
            }
        }
    }
}

void GltfModel::FetchAnimations(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Skin>::const_reference transmissionSkin : gltfModel.skins)
    {
        Skin& skin{ skins.emplace_back() };
        const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(transmissionSkin.inverseBindMatrices) };
        const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
        skin.inverseBindMatrices.resize(gltfAccessor.count);
        std::memcpy(skin.inverseBindMatrices.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4X4));
        skin.joints = transmissionSkin.joints;
    }

    for (std::vector<tinygltf::Animation>::const_reference gltfAnimation : gltfModel.animations)
    {
        Animation& animation{ animations.emplace_back() };
        animation.name = gltfAnimation.name;
        for (std::vector<tinygltf::AnimationSampler>::const_reference gltfSampler : gltfAnimation.samplers)
        {
            Animation::Sampler& sampler{ animation.samplers.emplace_back() };
            sampler.input = gltfSampler.input;
            sampler.output = gltfSampler.output;
            sampler.interpolation = gltfSampler.interpolation;

            const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(gltfSampler.input) };
            const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
            _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
            _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_SCALAR, L"");
            const std::pair < std::unordered_map<int, std::vector<float>>::iterator, bool>& timelines{ animation.timelines.emplace(gltfSampler.input,gltfAccessor.count) };
            //std::pair < std::unordered_map<int, std::vector<float>>::iterator, bool>& timelines{ animation.timelines.emplace(gltfSampler.input,gltfAccessor.count) };
            if (timelines.second)
            {
                std::memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
            }
        }
        for (std::vector<tinygltf::AnimationChannel>::const_reference gltfChannel : gltfAnimation.channels)
        {
            Animation::Channel& channel{ animation.channels.emplace_back() };
            channel.sampler = gltfChannel.sampler;
            channel.targetNode = gltfChannel.target_node;
            channel.targetPath = gltfChannel.target_path;

            const tinygltf::AnimationSampler& gltfSampler{ gltfAnimation.samplers.at(gltfChannel.sampler) };
            const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(gltfSampler.output) };
            const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
            if (gltfChannel.target_path == "scale")
            {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"");

                //std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT3>>::iterator, bool>& scales{ animation.scales.emplace(gltfSampler.output,gltfAccessor.count) };
                const std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT3>>::iterator, bool>& scales{ animation.scales.emplace(gltfSampler.output,gltfAccessor.count) };
                if (scales.second)
                {
                    std::memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "rotation")
            {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC4, L"");

                //std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT4>>::iterator, bool>& rotations{ animation.rotations.emplace(gltfSampler.output,gltfAccessor.count) };
                const std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT4>>::iterator, bool>& rotations{ animation.rotations.emplace(gltfSampler.output,gltfAccessor.count) };
                if (rotations.second)
                {
                    std::memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4));
                }
            }
            else if (gltfChannel.target_path == "translation")
            {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"");

                //std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT3>>::iterator, bool>& translations{ animation.translations.emplace(gltfSampler.output,gltfAccessor.count) };
                const std::pair < std::unordered_map<int, std::vector <DirectX::XMFLOAT3>>::iterator, bool>& translations{ animation.translations.emplace(gltfSampler.output,gltfAccessor.count) };
                if (translations.second)
                {
                    std::memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "weights")
            {

            }
            else
            {
                _ASSERT_EXPR(FALSE, L"");
            }

        }
    }
    for (decltype(animations)::reference animation : animations)
    {
        for (decltype(animation.timelines)::reference timelines : animation.timelines)
        {
            animation.duration = std::max<float>(animation.duration, timelines.second.back());
        }
    }
}


GltfModel::BufferView GltfModel::MakeBufferView(const tinygltf::Accessor& accessor)
{
    BufferView bufferView;
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            bufferView.format = DXGI_FORMAT_R16_UINT;
            bufferView.strideInBytes = sizeof(UINT16);
            //bufferView.format = DXGI_FORMAT_R8_UINT;
            //bufferView.strideInBytes = sizeof(UINT8);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format = DXGI_FORMAT_R16_UINT;
            bufferView.strideInBytes = sizeof(USHORT);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format = DXGI_FORMAT_R32_UINT;
            bufferView.strideInBytes = sizeof(UINT);
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32_FLOAT;
            bufferView.strideInBytes = sizeof(FLOAT) * 2;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32B32_FLOAT;
            bufferView.strideInBytes = sizeof(FLOAT) * 3;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            bufferView.format = DXGI_FORMAT_R8G8B8A8_UINT;
            bufferView.strideInBytes = sizeof(BYTE) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format = DXGI_FORMAT_R16G16B16A16_UINT;
            bufferView.strideInBytes = sizeof(USHORT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format = DXGI_FORMAT_R32G32B32A32_UINT;
            bufferView.strideInBytes = sizeof(UINT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            bufferView.strideInBytes = sizeof(FLOAT) * 4;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported");
            break;
        }
        break;
    default:
        _ASSERT_EXPR(FALSE, L"This accessor component type is not supported");
        break;
    }
    bufferView.sizeInBytes = static_cast<UINT>(accessor.count * bufferView.strideInBytes);
    return bufferView;
}

