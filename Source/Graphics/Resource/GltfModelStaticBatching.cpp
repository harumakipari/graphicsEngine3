#include "GltfModelStaticBatching.h"
#include <stack>
#include <functional>
#include <filesystem>
#include <sstream>
#include <fstream>

#include "Engine/Utility/Win32Utils.h"
#include "Engine/Debug/Assert.h"
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Timer.h"

#include "Texture.h"

bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*);

GltfModelStaticBatching::GltfModelStaticBatching(ID3D11Device* device, const std::string& filename) : GltfModelBase(device,filename)
{
    std::filesystem::path cerealFilename(filename);
    cerealFilename.replace_extension("cereal");
#if 0
    if (std::filesystem::exists(cerealFilename.c_str()))
    {
        HRESULT hr{ S_OK };

        tinygltf::TinyGLTF tiny_gltf;
        tiny_gltf.SetImageLoader(NullLoadImageData, nullptr);

        tinygltf::Model gltf_model;
        std::string error, warning;
        bool succeeded{ false };
        if (filename.find(".glb") != std::string::npos)
        {
            succeeded = tiny_gltf.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
        }
        else if (filename.find(".gltf") != std::string::npos)
        {
            succeeded = tiny_gltf.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
        }

        _ASSERT_EXPR_A(warning.empty(), warning.c_str());
        _ASSERT_EXPR_A(error.empty(), error.c_str());
        _ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

        for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltf_model.scenes)
        {
            Scene& scene{ scenes.emplace_back() };
            scene.name = gltf_model.scenes.at(0).name;
            scene.nodes = gltf_model.scenes.at(0).nodes;
        }

        FetchNodes(gltf_model);
        std::ifstream  ifs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(combined_buffers);
        FetchMaterials(device, gltf_model);
        FetchTextures(device, gltf_model);
        // TODO: This is a force-brute programming, may cause bugs.
        const std::map<std::string, BufferView>& vertex_buffer_views{ primitives.at(0).vertexBufferViews };
        D3D11_INPUT_ELEMENT_DESC input_element_desc[]
        {
            { "POSITION", 0, vertex_buffer_views.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            { "NORMAL", 0, vertex_buffer_views.at("NORMAL").format, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, vertex_buffer_views.at("TANGENT").format, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, vertex_buffer_views.at("TEXCOORD_0").format, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = CreateVsFromCSO(device, "./Shader/GltfModelStaticBatchingVS.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", pixelShader.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(primitive_constants);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&buffer_desc, nullptr, primitiveCbuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    }
    else
#endif
    {
        HRESULT hr{ S_OK };

        tinygltf::TinyGLTF tiny_gltf;
        tiny_gltf.SetImageLoader(NullLoadImageData, nullptr);

        tinygltf::Model gltf_model;
        std::string error, warning;
        bool succeeded{ false };
        if (filename.find(".glb") != std::string::npos)
        {
            succeeded = tiny_gltf.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
        }
        else if (filename.find(".gltf") != std::string::npos)
        {
            succeeded = tiny_gltf.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
        }

        _ASSERT_EXPR_A(warning.empty(), warning.c_str());
        _ASSERT_EXPR_A(error.empty(), error.c_str());
        _ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

        for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltf_model.scenes)
        {
            Scene& scene{ scenes.emplace_back() };
            scene.name = gltf_model.scenes.at(0).name;
            scene.nodes = gltf_model.scenes.at(0).nodes;
        }
        benchmark nodeTimer;
        benchmark meshTimer;
        benchmark materialTimer;
        benchmark textureTimer;
        nodeTimer.begin();
        FetchNodes(gltf_model);
        nodeTimer.end();
        meshTimer.begin();
        FetchMeshes(device, gltf_model);
        meshTimer.end();
        materialTimer.begin();
        FetchMaterials(device, gltf_model);
        materialTimer.end();
        textureTimer.begin();
        FetchTextures(device, gltf_model);
        textureTimer.end();

        char debugActorMessage[256];
        char debugStageMessage[256];
        char debugEnemyMessage[256];
        char debugTextureMessage[256];
        sprintf_s(debugActorMessage, sizeof(debugActorMessage), "node loading time = %.6f sec\n", textureTimer.end());
        sprintf_s(debugStageMessage, sizeof(debugStageMessage), "mesh loading time = %.6f sec\n", meshTimer.end());
        sprintf_s(debugEnemyMessage, sizeof(debugEnemyMessage), "material loading time = %.6f sec\n", materialTimer.end());
        sprintf_s(debugTextureMessage, sizeof(debugTextureMessage), "texture loading time = %.6f sec\n", textureTimer.end());
        //sprintf_s(aaa, sizeof(aaa), "time = %d\n", timeGetTime() - time);
        //time = timeGetTime();
        //OutputDebugStringA(aaa);
        OutputDebugStringA(debugActorMessage);
        OutputDebugStringA(debugStageMessage);
        OutputDebugStringA(debugEnemyMessage);
        OutputDebugStringA(debugTextureMessage);


        // TODO: This is a force-brute programming, may cause bugs.
        const std::map<std::string, BufferView>& vertex_buffer_views{ primitives.at(0).vertexBufferViews };
        D3D11_INPUT_ELEMENT_DESC input_element_desc[]
        {
            { "POSITION", 0, vertex_buffer_views.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            { "NORMAL", 0, vertex_buffer_views.at("NORMAL").format, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, vertex_buffer_views.at("TANGENT").format, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, vertex_buffer_views.at("TEXCOORD_0").format, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = CreateVsFromCSO(device, "./Shader/GltfModelStaticBatchingVS.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", pixelShader.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // CASCADED_SHADOW_MAPS
        hr = CreateVsFromCSO(device, "./Shader/GltfModelStaticBatchingCsmVS.cso", vertexShaderCSM.ReleaseAndGetAddressOf(), NULL, NULL, 0);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = CreateGsFromCSO(device, "./Shader/GltfModelCsmGS.cso", geometryShaderCSM.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(primitive_constants);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&buffer_desc, nullptr, primitiveCbuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#if 0
        std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(vertices);
        std::ifstream  ifs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(vertices);
        int i = 0;
#endif
    }
}
void GltfModelStaticBatching::FetchNodes(const tinygltf::Model& gltf_model)
{
    for (std::vector<tinygltf::Node>::const_reference gltf_node : gltf_model.nodes)
    {
        Node& node{ nodes.emplace_back() };
        node.name = gltf_node.name;
        node.skin = gltf_node.skin;
        node.mesh = gltf_node.mesh;
        node.children = gltf_node.children;
        if (!gltf_node.matrix.empty())
        {
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++)
            {
                for (size_t column = 0; column < 4; column++)
                {
                    matrix(row, column) = static_cast<float>(gltf_node.matrix.at(4 * row + column));
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
            if (gltf_node.scale.size() > 0)
            {
                node.scale.x = static_cast<float>(gltf_node.scale.at(0));
                node.scale.y = static_cast<float>(gltf_node.scale.at(1));
                node.scale.z = static_cast<float>(gltf_node.scale.at(2));
            }
            if (gltf_node.translation.size() > 0)
            {
                node.translation.x = static_cast<float>(gltf_node.translation.at(0));
                node.translation.y = static_cast<float>(gltf_node.translation.at(1));
                node.translation.z = static_cast<float>(gltf_node.translation.at(2));
            }
            if (gltf_node.rotation.size() > 0)
            {
                node.rotation.x = static_cast<float>(gltf_node.rotation.at(0));
                node.rotation.y = static_cast<float>(gltf_node.rotation.at(1));
                node.rotation.z = static_cast<float>(gltf_node.rotation.at(2));
                node.rotation.w = static_cast<float>(gltf_node.rotation.at(3));
            }
        }
    }
    CumulateTransforms(nodes);
}
GltfModelStaticBatching::BufferView GltfModelStaticBatching::MakeBufferView(const tinygltf::Accessor& accessor)
{
    BufferView buffer_view;
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            buffer_view.format = DXGI_FORMAT_R16_UINT;
            buffer_view.strideInBytes = sizeof(USHORT);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            buffer_view.format = DXGI_FORMAT_R32_UINT;
            buffer_view.strideInBytes = sizeof(UINT);
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32_FLOAT;
            buffer_view.strideInBytes = sizeof(FLOAT) * 2;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32B32_FLOAT;
            buffer_view.strideInBytes = sizeof(FLOAT) * 3;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            buffer_view.format = DXGI_FORMAT_R16G16B16A16_UINT;
            buffer_view.strideInBytes = sizeof(USHORT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            buffer_view.format = DXGI_FORMAT_R32G32B32A32_UINT;
            buffer_view.strideInBytes = sizeof(UINT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            buffer_view.strideInBytes = sizeof(FLOAT) * 4;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    default:
        _ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
        break;
    }
    buffer_view.sizeInBytes = static_cast<UINT>(accessor.count * buffer_view.strideInBytes);
    return buffer_view;
}
void GltfModelStaticBatching::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    using namespace DirectX;

    HRESULT hr = S_OK;

    std::unordered_map<int/*material*/, CombinedBuffer> combined_buffers;

    // Collect primitives with same material
    for (decltype(nodes)::reference node : nodes)
    {
        const XMMATRIX transform = XMLoadFloat4x4(&node.globalTransform);

        if (node.mesh > -1)
        {
            const tinygltf::Mesh& gltf_mesh = gltf_model.meshes.at(node.mesh);
            //const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[node.mesh];

            for (std::vector<tinygltf::Primitive>::const_reference gltf_primitive : gltf_mesh.primitives)
            {
                CombinedBuffer& combined_buffer = combined_buffers[gltf_primitive.material];
                if (gltf_primitive.indices > -1)
                {
                    //const tinygltf::Accessor gltf_accessor = gltf_model.accessors[gltf_primitive.indices];
                    //const tinygltf::BufferView& gltf_buffer_view = gltf_model.bufferViews[gltf_accessor.bufferView];
                    const tinygltf::Accessor gltf_accessor = gltf_model.accessors.at(gltf_primitive.indices);
                    const tinygltf::BufferView& gltf_buffer_view = gltf_model.bufferViews.at(gltf_accessor.bufferView);

                    if (gltf_accessor.count == 0)
                    {
                        continue;
                    }

                    const size_t vertex_offset = combined_buffer.vertices.positions.size();
                    if (gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const unsigned char* buffer = gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;
                        //const unsigned char* buffer = gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            combined_buffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessor_index] + vertex_offset));
                        }
                    }
                    else if (gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        const unsigned short* buffer = reinterpret_cast<const unsigned short*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const unsigned short* buffer = reinterpret_cast<const unsigned short*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            combined_buffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessor_index] + vertex_offset));
                        }
                    }
                    else if (gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        const unsigned int* buffer = reinterpret_cast<const unsigned int*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const unsigned int* buffer = reinterpret_cast<const unsigned int*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            combined_buffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessor_index] + vertex_offset));
                        }
                    }
                    else
                    {
                        _ASSERT_EXPR(false, L"This index format is not supported.");
                    }
                }

                // Combine primitives using the same material into a single vertex buffer. In addition, apply a coordinate transformation to position, normal and tangent of primitives.
                for (decltype(gltf_primitive.attributes)::const_reference gltf_attribute : gltf_primitive.attributes)
                {
                    //const tinygltf::Accessor gltf_accessor = gltf_model.accessors[gltf_attribute.second];
                    //const tinygltf::BufferView& gltf_buffer_view = gltf_model.bufferViews[gltf_accessor.bufferView];
                    const tinygltf::Accessor gltf_accessor = gltf_model.accessors.at(gltf_attribute.second);
                    const tinygltf::BufferView& gltf_buffer_view = gltf_model.bufferViews.at(gltf_accessor.bufferView);

                    if (gltf_accessor.count == 0)
                    {
                        continue;
                    }

                    if (gltf_attribute.first == "POSITION")
                    {
                        _ASSERT_EXPR(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltf_accessor.type == TINYGLTF_TYPE_VEC3, L"'POSITION' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC3.");
                        const XMFLOAT3* buffer = reinterpret_cast<const XMFLOAT3*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const XMFLOAT3* buffer = reinterpret_cast<const XMFLOAT3*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            XMFLOAT3 position = buffer[accessor_index];
                            DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), transform));
                            combined_buffer.vertices.positions.emplace_back(position);
                            vertices.position = position;
                        }
                    }
                    else if (gltf_attribute.first == "NORMAL")
                    {
                        _ASSERT_EXPR(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltf_accessor.type == TINYGLTF_TYPE_VEC3, L"'NORMAL' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC3.");
                        const XMFLOAT3* buffer = reinterpret_cast<const XMFLOAT3*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const XMFLOAT3* buffer = reinterpret_cast<const XMFLOAT3*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            XMFLOAT3 normal = buffer[accessor_index];
                            XMStoreFloat3(&normal, XMVector3TransformNormal(XMLoadFloat3(&normal), transform));
                            combined_buffer.vertices.normals.emplace_back(normal);
                        }
                    }
                    else if (gltf_attribute.first == "TANGENT")
                    {
                        _ASSERT_EXPR(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltf_accessor.type == TINYGLTF_TYPE_VEC4, L"'TANGENT' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC4.");
                        const XMFLOAT4* buffer = reinterpret_cast<const XMFLOAT4*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const XMFLOAT4* buffer = reinterpret_cast<const XMFLOAT4*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            XMFLOAT4 tangent = buffer[accessor_index];
                            float sigma = tangent.w;
                            tangent.w = 0;
                            XMStoreFloat4(&tangent, XMVector4Transform(XMLoadFloat4(&tangent), transform));
                            tangent.w = sigma;
                            combined_buffer.vertices.tangents.emplace_back(tangent);
                        }
                    }
                    else if (gltf_attribute.first == "TEXCOORD_0")
                    {
                        _ASSERT_EXPR(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltf_accessor.type == TINYGLTF_TYPE_VEC2, L"'TEXCOORD_0' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC2.");
                        const XMFLOAT2* buffer = reinterpret_cast<const XMFLOAT2*>(gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        //const XMFLOAT2* buffer = reinterpret_cast<const XMFLOAT2*>(gltf_model.buffers[gltf_buffer_view.buffer].data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset);
                        for (size_t accessor_index = 0; accessor_index < gltf_accessor.count; ++accessor_index)
                        {
                            combined_buffer.vertices.texcoords.emplace_back(buffer[accessor_index]);
                        }
                    }
                }
            }
        }
    }

    // Create GPU buffers
    for (decltype(combined_buffers)::const_reference combined_buffer : combined_buffers)
    {
#if 1
        if (combined_buffer.second.vertices.positions.size() == 0)
        {
            continue;
        }
#endif
        Primitive& primitive = primitives.emplace_back();
        primitive.material = combined_buffer.first;

        D3D11_BUFFER_DESC buffer_desc = {};
        D3D11_SUBRESOURCE_DATA subresource_data = {};

        if (combined_buffer.second.indices.size() > 0)
        {
            primitive.indexBufferView.format = DXGI_FORMAT_R32_UINT;
            primitive.indexBufferView.strideInBytes = sizeof(UINT);
            primitive.indexBufferView.sizeInBytes = combined_buffer.second.indices.size() * primitive.indexBufferView.strideInBytes;

            buffer_desc.ByteWidth = static_cast<UINT>(primitive.indexBufferView.sizeInBytes);
            buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            buffer_desc.CPUAccessFlags = 0;
            buffer_desc.MiscFlags = 0;
            buffer_desc.StructureByteStride = 0;
            subresource_data.pSysMem = combined_buffer.second.indices.data();
            subresource_data.SysMemPitch = 0;
            subresource_data.SysMemSlicePitch = 0;
            hr = device->CreateBuffer(&buffer_desc, &subresource_data, primitive.indexBufferView.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }

        BufferView vertex_buffer_view;
        if (combined_buffer.second.vertices.positions.size() > 0)
        {
            vertex_buffer_view.format = DXGI_FORMAT_R32G32B32_FLOAT;
            vertex_buffer_view.strideInBytes = sizeof(FLOAT) * 3;
            vertex_buffer_view.sizeInBytes = combined_buffer.second.vertices.positions.size() * vertex_buffer_view.strideInBytes;

            buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.sizeInBytes);
            buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            subresource_data.pSysMem = combined_buffer.second.vertices.positions.data();
            hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            primitive.vertexBufferViews.emplace(std::make_pair("POSITION", vertex_buffer_view));
        }
        if (combined_buffer.second.vertices.normals.size() > 0)
        {
            vertex_buffer_view.format = DXGI_FORMAT_R32G32B32_FLOAT;
            vertex_buffer_view.strideInBytes = sizeof(FLOAT) * 3;
            vertex_buffer_view.sizeInBytes = combined_buffer.second.vertices.normals.size() * vertex_buffer_view.strideInBytes;

            buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.sizeInBytes);
            buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            subresource_data.pSysMem = combined_buffer.second.vertices.normals.data();
            hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            primitive.vertexBufferViews.emplace(std::make_pair("NORMAL", vertex_buffer_view));
        }
        if (combined_buffer.second.vertices.tangents.size() > 0)
        {
            vertex_buffer_view.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            vertex_buffer_view.strideInBytes = sizeof(FLOAT) * 4;
            vertex_buffer_view.sizeInBytes = combined_buffer.second.vertices.tangents.size() * vertex_buffer_view.strideInBytes;

            buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.sizeInBytes);
            buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            subresource_data.pSysMem = combined_buffer.second.vertices.tangents.data();
            hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            primitive.vertexBufferViews.emplace(std::make_pair("TANGENT", vertex_buffer_view));
        }
        if (combined_buffer.second.vertices.texcoords.size() > 0)
        {
            vertex_buffer_view.format = DXGI_FORMAT_R32G32_FLOAT;
            vertex_buffer_view.strideInBytes = sizeof(FLOAT) * 2;
            vertex_buffer_view.sizeInBytes = combined_buffer.second.vertices.texcoords.size() * vertex_buffer_view.strideInBytes;

            buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.sizeInBytes);
            buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            subresource_data.pSysMem = combined_buffer.second.vertices.texcoords.data();
            hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            primitive.vertexBufferViews.emplace(std::make_pair("TEXCOORD_0", vertex_buffer_view));
        }


        // Add dummy attributes if any are missing.
        const std::unordered_map<std::string, BufferView> attributes =
        {
            { "POSITION", { DXGI_FORMAT_R32G32B32_FLOAT } },
            { "NORMAL", { DXGI_FORMAT_R32G32B32_FLOAT } },
            { "TANGENT", { DXGI_FORMAT_R32G32B32A32_FLOAT } },
            { "TEXCOORD_0", { DXGI_FORMAT_R32G32_FLOAT } },
        };
        for (std::unordered_map<std::string, BufferView>::const_reference attribute : attributes)
        {
            if (primitive.vertexBufferViews.find(attribute.first) == primitive.vertexBufferViews.end())
            {
                primitive.vertexBufferViews.insert(std::make_pair(attribute.first, attribute.second));
            }
        }
    }
}

void GltfModelStaticBatching::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, RenderPass pass)
{

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    //immediate_context->PSSetShader(pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (decltype(primitives)::const_reference primitive : primitives)
    {
        ID3D11Buffer* vertex_buffers[] = {
            primitive.vertexBufferViews.at("POSITION").buffer.Get(),
            primitive.vertexBufferViews.at("NORMAL").buffer.Get(),
            primitive.vertexBufferViews.at("TANGENT").buffer.Get(),
            primitive.vertexBufferViews.at("TEXCOORD_0").buffer.Get(),
        };
        UINT strides[] = {
            static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").strideInBytes),
        };
        UINT offsets[_countof(vertex_buffers)] = {};
        immediateContext->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
        immediateContext->IASetIndexBuffer(primitive.indexBufferView.buffer.Get(), primitive.indexBufferView.format, 0);

        primitive_constants primitive_data = {};
        primitive_data.material = primitive.material;
        primitive_data.has_tangent = primitive.vertexBufferViews.at("TANGENT").buffer != NULL;
        XMStoreFloat4x4(&primitive_data.world, XMLoadFloat4x4(&world));
        immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitive_data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
        immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

        const Material& material = materials.at(primitive.material);
        //‚±‚±‚ÅÝ’è
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

        const int texture_indices[] =
        {
            material.data.pbrMetallicRoughness.basecolorTexture.index,
            material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
            material.data.normalTexture.index,
            material.data.emissiveTexture.index,
            material.data.occlusionTexture.index,
        };
        ID3D11ShaderResourceView* null_shader_resource_view = {};
        std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
        for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
        {
            shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? textureResourceViews.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
        }
        immediateContext->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

        immediateContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.count()), 0, 0);

    }
}

void GltfModelStaticBatching::CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<Node>& animatedNodes)
{
    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    // CASCADED_SHADOW_MAPS
    immediateContext->VSSetShader(vertexShaderCSM.Get(), nullptr, 0);
    immediateContext->GSSetShader(geometryShaderCSM.Get(), nullptr, 0);

    Microsoft::WRL::ComPtr <ID3D11PixelShader> nullPixelShader{ NULL };
    immediateContext->PSSetShader(nullPixelShader.Get()/*SHADOW*/, nullptr, 0);

    //immediate_context->PSSetShader(pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (decltype(primitives)::const_reference primitive : primitives)
    {
        ID3D11Buffer* vertex_buffers[] = {
            primitive.vertexBufferViews.at("POSITION").buffer.Get(),
            primitive.vertexBufferViews.at("NORMAL").buffer.Get(),
            primitive.vertexBufferViews.at("TANGENT").buffer.Get(),
            primitive.vertexBufferViews.at("TEXCOORD_0").buffer.Get(),
        };
        UINT strides[] = {
            static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes),
            static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").strideInBytes),
        };
        UINT offsets[_countof(vertex_buffers)] = {};
        immediateContext->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
        immediateContext->IASetIndexBuffer(primitive.indexBufferView.buffer.Get(), primitive.indexBufferView.format, 0);

        // CPU‘¤‚Ìƒf[ƒ^
        primitive_constants primitive_data = {};
        primitive_data.material = primitive.material;
        primitive_data.has_tangent = primitive.vertexBufferViews.at("TANGENT").buffer != NULL;
        XMStoreFloat4x4(&primitive_data.world, XMLoadFloat4x4(&world));
        immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitive_data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
        immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

        const Material& material = materials.at(primitive.material);
        ////‚±‚±‚ÅÝ’è ‰e‚¾‚©‚ç‚¢‚ç‚È‚¢
        //if (material.replacedPixelShader)
        //{
        //    immediateContext->PSSetShader(material.replacedPixelShader.Get(), nullptr, 0);
        //}
        //else
        //{
            //immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
        //}

        const int texture_indices[] =
        {
            material.data.pbrMetallicRoughness.basecolorTexture.index,
            material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
            material.data.normalTexture.index,
            material.data.emissiveTexture.index,
            material.data.occlusionTexture.index,
        };
        ID3D11ShaderResourceView* null_shader_resource_view = {};
        std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
        for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
        {
            shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? textureResourceViews.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
        }
        immediateContext->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

        // CASCADED_SHADOW_MAPS
        immediateContext->DrawIndexedInstanced(static_cast<UINT>(primitive.indexBufferView.count()), 4, 0, 0, 0);
        //immediateContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.count()), 0, 0);
    }

    immediateContext->VSSetShader(NULL, NULL, 0);
    immediateContext->GSSetShader(NULL, NULL, 0);
    immediateContext->PSSetShader(NULL, NULL, 0);

}

void GltfModelStaticBatching::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    for (std::vector<tinygltf::Material>::const_reference gltf_material : gltf_model.materials)
    {
        std::vector<Material>::reference material = materials.emplace_back();

        material.name = gltf_material.name;

        material.data.emissiveFactor[0] = static_cast<float>(gltf_material.emissiveFactor.at(0));
        material.data.emissiveFactor[1] = static_cast<float>(gltf_material.emissiveFactor.at(1));
        material.data.emissiveFactor[2] = static_cast<float>(gltf_material.emissiveFactor.at(2));

        material.data.alphaMode = gltf_material.alphaMode == "OPAQUE" ? 0 : gltf_material.alphaMode == "MASK" ? 1 : gltf_material.alphaMode == "BLEND" ? 2 : 0;
        material.data.alphaCutoff = static_cast<float>(gltf_material.alphaCutoff);
        material.data.doubleSided = gltf_material.doubleSided ? 1 : 0;

        material.data.pbrMetallicRoughness.basecolorFactor[0] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data.pbrMetallicRoughness.basecolorFactor[1] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data.pbrMetallicRoughness.basecolorFactor[2] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data.pbrMetallicRoughness.basecolorFactor[3] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data.pbrMetallicRoughness.basecolorTexture.index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
        material.data.pbrMetallicRoughness.basecolorTexture.texcoord = gltf_material.pbrMetallicRoughness.baseColorTexture.texCoord;
        material.data.pbrMetallicRoughness.metallicFactor = static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor);
        material.data.pbrMetallicRoughness.roughnessFactor = static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor);
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.texcoord = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        material.data.normalTexture.index = gltf_material.normalTexture.index;
        material.data.normalTexture.texcoord = gltf_material.normalTexture.texCoord;
        material.data.normalTexture.scale = static_cast<float>(gltf_material.normalTexture.scale);

        material.data.occlusionTexture.index = gltf_material.occlusionTexture.index;
        material.data.occlusionTexture.texcoord = gltf_material.occlusionTexture.texCoord;
        material.data.occlusionTexture.strength = static_cast<float>(gltf_material.occlusionTexture.strength);

        material.data.emissiveTexture.index = gltf_material.emissiveTexture.index;
        material.data.emissiveTexture.texcoord = gltf_material.emissiveTexture.texCoord;
    }

    // Create material data as shader resource view on GPU
    std::vector<Material::Cbuffer> material_data;
    for (std::vector<Material>::const_reference material : materials)
    {
        material_data.emplace_back(material.data);
    }

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> material_buffer;
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * material_data.size());
    buffer_desc.StructureByteStride = sizeof(Material::Cbuffer);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = material_data.data();
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, material_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
    shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
    shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(material_data.size());
    hr = device->CreateShaderResourceView(material_buffer.Get(), &shader_resource_view_desc, materialResourceView.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}
void GltfModelStaticBatching::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    HRESULT hr{ S_OK };
    for (const tinygltf::Texture& gltf_texture : gltf_model.textures)
    {
        Texture& texture{ textures.emplace_back() };
        texture.name = gltf_texture.name;
        texture.source = gltf_texture.source;
    }
    for (const tinygltf::Image& gltf_image : gltf_model.images)
    {
        Image& image{ images.emplace_back() };
        image.name = gltf_image.name;
        image.width = gltf_image.width;
        image.height = gltf_image.height;
        image.component = gltf_image.component;
        image.bits = gltf_image.bits;
        image.pixelType = gltf_image.pixel_type;
        image.bufferView = gltf_image.bufferView;
        image.mimeType = gltf_image.mimeType;
        image.uri = gltf_image.uri;
        image.asIs = gltf_image.as_is;

        if (gltf_image.bufferView > -1)
        {
            const tinygltf::BufferView& buffer_view{ gltf_model.bufferViews.at(gltf_image.bufferView) };
            const tinygltf::Buffer& buffer{ gltf_model.buffers.at(buffer_view.buffer) };
            //const byte* data = buffer.data.data() + buffer_view.byteOffset;
            const unsigned char* data = buffer.data.data() + buffer_view.byteOffset;

            ID3D11ShaderResourceView* texture_resource_view{};
            hr = LoadTextureFromMemory(device, data, buffer_view.byteLength, &texture_resource_view);
            if (hr == S_OK)
            {
                textureResourceViews.emplace_back().Attach(texture_resource_view);
            }
        }
        else
        {
            const std::filesystem::path path(filename);
            ID3D11ShaderResourceView* shader_resource_view{};
            D3D11_TEXTURE2D_DESC texture2d_desc;
            std::wstring filename{ path.parent_path().concat(L"/").wstring() + std::wstring(gltf_image.uri.begin(), gltf_image.uri.end()) };
            hr = LoadTextureFromFile(device, filename.c_str(), &shader_resource_view, &texture2d_desc);
            if (hr == S_OK)
            {
                textureResourceViews.emplace_back().Attach(shader_resource_view);
            }
        }
    }
}

