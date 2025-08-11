#include "SceneRenderer.h"

#include "Engine/Scene/Scene.h"

UINT SizeofComponent(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R8_UINT: return 1;
    case DXGI_FORMAT_R16_UINT: return 2;
    case DXGI_FORMAT_R32_UINT: return 4;
    case DXGI_FORMAT_R32G32_FLOAT: return 8;
    case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
    case DXGI_FORMAT_R8G8B8A8_UINT: return 3;
    case DXGI_FORMAT_R16G16B16A16_UINT: return 8;
    case DXGI_FORMAT_R32G32B32A32_UINT: return 16;
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
    }
    _ASSERT_EXPR(FALSE, L"Not supported");
    return 0;
}

//bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
//{
//    return true;
//}

void SceneRenderer::RenderOpaque(ID3D11DeviceContext* immediateContext/*, std::vector<std::shared_ptr<Actor>> allActors*/)
{
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();

    for (auto actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actorが存在していなかったらスキップ
            continue;
        }

        // actor に付属している全ての meshComponent を取り出す
        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        for (MeshComponent* meshComponent : meshComponents)
        {
            if (!meshComponent->IsVisible())
            { // 描画フラグが false ならスキップ
                continue;
            }
            // 各 MeshComponent 自身の最新ワールド行列を取り出す
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            // 各 MeshComponent の model を取り出す
            const InterleavedGltfModel* model = meshComponent->model.get();

            auto* convexComponent = actor->GetComponent<ConvexCollisionComponent>();
            if (convexComponent = dynamic_cast<ConvexCollisionComponent*>(convexComponent))
            {
                //meshComponent->model->Render(immediateContext, worldMat, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Opaque);
            }


            if (meshComponent->model->mode == InterleavedGltfModel::Mode::SkeltalMesh)
            {// 
                Draw(immediateContext, meshComponent, worldMat, meshComponent->modelNodes, InterleavedGltfModel::RenderPass::Opaque);
            }

            //  描画呼び出し
            //meshComponent->RenderOpaque(immediateContext, worldMat);
        }
    }
}

void SceneRenderer::RenderMask(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();

    for (auto actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actorが存在していなかったらスキップ
            continue;
        }

        // actor に付属している全ての meshComponent を取り出す
        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        for (const MeshComponent* meshComponent : meshComponents)
        {
            if (!meshComponent->IsVisible())
            { // 描画フラグが false ならスキップ
                continue;
            }
            // 各 MeshComponent 自身の最新ワールド行列を取り出す
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            // 各 MeshComponent の model を取り出す
            const InterleavedGltfModel* model = meshComponent->model.get();

            if (meshComponent->model->mode == InterleavedGltfModel::Mode::SkeltalMesh)
            {// 
                Draw(immediateContext, meshComponent, worldMat, meshComponent->modelNodes, InterleavedGltfModel::RenderPass::Mask);
            }
        }
    }
}

void SceneRenderer::RenderBlend(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();

    for (auto actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actorが存在していなかったらスキップ
            continue;
        }

        // actor に付属している全ての meshComponent を取り出す
        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        for (const MeshComponent* meshComponent : meshComponents)
        {
            if (!meshComponent->IsVisible())
            { // 描画フラグが false ならスキップ
                continue;
            }
            // 各 MeshComponent 自身の最新ワールド行列を取り出す
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            // 各 MeshComponent の model を取り出す
            const InterleavedGltfModel* model = meshComponent->model.get();

            if (meshComponent->model->mode == InterleavedGltfModel::Mode::SkeltalMesh)
            {// 
                Draw(immediateContext, meshComponent, worldMat, meshComponent->modelNodes, InterleavedGltfModel::RenderPass::Blend);
            }
        }
    }
}

void SceneRenderer::Draw(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass)
{
    // 各 MeshComponent の model を取り出す
    const InterleavedGltfModel* model = meshComponent->model.get();
    immediateContext->PSSetShaderResources(0, 1, model->materialResourceView.GetAddressOf());
    //std::string pName = GetPipelineName(currentRenderPath, static_cast<MaterialAlphaMode>(pass), static_cast<ModelMode>(model->mode));
    //pipeLineStateSet->BindPipeLineState(immediateContext, pName);
    //pipeLineStateSet->BindPipeLineState(immediateContext, "forwardOpaqueSkeltalMesh");
    std::function<void(int)> traverse = [&](int nodeIndex)->void {
        const InterleavedGltfModel::Node& node = animatedNodes.at(nodeIndex);
        if (node.skin > -1)
        {
            const InterleavedGltfModel::Skin& skin = model->skins.at(node.skin);
            _ASSERT_EXPR(skin.joints.size() <= PRIMITIVE_MAX_JOINTS, L"The size of the joint array is insufficient, please expand it.");
            for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex)
            {
                DirectX::XMStoreFloat4x4(&primitiveJointCBuffer->data.matrices[jointIndex],
                    DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) *
                    DirectX::XMLoadFloat4x4(&animatedNodes.at(skin.joints.at(jointIndex)).globalTransform) *
                    DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform))
                );
            }
            // 2番に定数バッファを送る
            primitiveJointCBuffer->Activate(immediateContext, 2);
        }
        if (node.mesh > -1)
        {
            const InterleavedGltfModel::Mesh& mesh = model->meshes.at(node.mesh);
            for (const InterleavedGltfModel::Mesh::Primitive& primitive : mesh.primitives)
            {
                // INTERLEAVED_GLTF_MODEL
                UINT stride = sizeof(InterleavedGltfModel::Mesh::Vertex);
                UINT offset = 0;
                immediateContext->IASetVertexBuffers(0, 1, model->buffers.at(primitive.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

                primitiveCBuffer->data.material = primitive.material;
                primitiveCBuffer->data.hasTangent = primitive.has("TANGENT");
                primitiveCBuffer->data.skin = node.skin;
                primitiveCBuffer->data.color = { model->cpuColor.x,model->cpuColor.y,model->cpuColor.z,model->alpha };
                primitiveCBuffer->data.emission = model->emission;
                primitiveCBuffer->data.disolveFactor = model->disolveFactor;

                //座標系の変換を行う
                const DirectX::XMFLOAT4X4 coordinateSystemTransforms[]
                {
                    {//RHS Y-UP
                        -1,0,0,0,
                         0,1,0,0,
                         0,0,1,0,
                         0,0,0,1,
                    },
                    {//LHS Y-UP
                        1,0,0,0,
                        0,1,0,0,
                        0,0,1,0,
                        0,0,0,1,
                    },
                    {//RHS Z-UP
                        -1,0, 0,0,
                         0,0,-1,0,
                         0,1, 0,0,
                         0,0, 0,1,
                    },
                    {//LHS Z-UP
                        1,0,0,0,
                        0,0,1,0,
                        0,1,0,0,
                        0,0,0,1,
                    },
                };

                float scaleFactor;

                if (model->isModelInMeters)
                {
                    scaleFactor = 1.0f;//メートル単位の時
                }
                else
                {
                    scaleFactor = 0.01f;//㎝単位の時
                }
                DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[static_cast<int>(model->modelCoordinateSystem)]) * DirectX::XMMatrixScaling(scaleFactor,scaleFactor,scaleFactor) };

                DirectX::XMStoreFloat4x4(&primitiveCBuffer->data.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * C * DirectX::XMLoadFloat4x4(&world));
                // 0番に定数バッファを送る
                primitiveCBuffer->Activate(immediateContext, 0);

                const InterleavedGltfModel::Material& material = model->materials.at(primitive.material);

                std::string pipelineName;
                if (material.overridePipelineName.has_value())
                {
                    pipelineName = *material.overridePipelineName;
                }
                else if (meshComponent->overridePipelineName.has_value())
                {
                    pipelineName = *meshComponent->overridePipelineName;
                }
                else
                {
                    pipelineName = GetPipelineName(currentRenderPath, static_cast<MaterialAlphaMode>(material.data.alphaMode), static_cast<ModelMode>(model->mode));
                }
                pipeLineStateSet->BindPipeLineState(immediateContext, pipelineName);
                ////ここで設定
                //if (material.replacedPixelShader)
                //{
                //    immediateContext->PSSetShader(material.replacedPixelShader.Get(), nullptr, 0);
                //}
                //else
                //{
                //    immediateContext->PSSetShader(pipeline.pixelShader ? pipeline.pixelShader.Get() : pixelShader.Get(), nullptr, 0);
                //}
                bool passed = false;
                switch (pass)
                {
                case InterleavedGltfModel::RenderPass::Opaque:
                    if (material.data.alphaMode == 0/*OPAQUE*/)
                    {
                        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE);
                        passed = true;
                    }
                    break;
                case InterleavedGltfModel::RenderPass::Mask:
                    if (material.data.alphaMode == 1/*MASK*/)
                    {
                        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE);
                        passed = true;
                    }
                    break;
                case InterleavedGltfModel::RenderPass::Blend:
                    if (material.data.alphaMode == 2/*BLEND*/)
                    {
                        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
                        passed = true;
                    }
                    break;
                case InterleavedGltfModel::RenderPass::All:
                    passed = true;
                    break;
                }
                if (!passed)
                {
                    continue;
                }

                const int textureIndices[] =
                {
                    material.data.pbrMetallicRoughness.basecolorTexture.index,
                    material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
                    material.data.normalTexture.index,
                    material.data.emissiveTexture.index,
                    material.data.occlusionTexture.index,
                };

                ID3D11ShaderResourceView* nullShaderResourceView = {};
                std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
                {
                    shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? model->textureResourceViews.at(model->textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
                }
                immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

                if (primitive.indexBufferView.buffer > -1)
                {
                    // INTERLEAVED_GLTF_MODEL
                    immediateContext->IASetIndexBuffer(model->buffers.at(primitive.indexBufferView.buffer).Get(), primitive.indexBufferView.format, 0);
                    immediateContext->DrawIndexed(primitive.indexBufferView.sizeInBytes / SizeofComponent(primitive.indexBufferView.format), 0, 0);
                }
                else
                {
                    // INTERLEAVED_GLTF_MODEL
                    immediateContext->Draw(primitive.vertexBufferView.sizeInBytes / primitive.vertexBufferView.strideInBytes, 0);
                }
            }
        }
        for (std::vector<int>::value_type childIndex : node.children)
        {
            traverse(childIndex);
        }
        };
    for (std::vector<int>::value_type nodeIndex : model->scenes.at(model->defaultScene).nodes)
    {
        traverse(nodeIndex);
    }

}

void SceneRenderer::DrawWithStaticBatching(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass)
{
#if 0
    _ASSERT_EXPR(mode == Mode::StaticMesh, L"This function only works with static_batching data.");

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    immediateContext->VSSetShader(pipeline.vertexShader ? pipeline.vertexShader.Get() : vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pipeline.pixelShader ? pipeline.pixelShader.Get() : pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const BatchMesh& batchMesh : batchMeshes)
    {
        UINT stride = sizeof(BatchMesh::Vertex);
        UINT offset = 0;
        immediateContext->IASetVertexBuffers(0, 1, buffers.at(batchMesh.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

        PrimitiveConstants primitiveData = {};
        primitiveData.material = batchMesh.material;
        primitiveData.hasTangent = batchMesh.has("TANGENT");
        primitiveData.skin = -1;
        const DirectX::XMFLOAT4X4 coordinateSystemTransforms[]
        {
            {//RHS Y-UP
                -1,0,0,0,
                 0,1,0,0,
                 0,0,1,0,
                 0,0,0,1,
            },
            {//LHS Y-UP
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1,
            },
            {//RHS Z-UP
                -1,0, 0,0,
                 0,0,-1,0,
                 0,1, 0,0,
                 0,0, 0,1,
            },
            {//LHS Z-UP
                1,0,0,0,
                0,0,1,0,
                0,1,0,0,
                0,0,0,1,
            },
        };

        float scaleFactor;

        if (isModelInMeters)
        {
            scaleFactor = 1.0f;//メートル単位の時
        }
        else
        {
            scaleFactor = 0.01f;//㎝単位の時
        }
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[static_cast<int>(modelCoordinateSystem)]) * DirectX::XMMatrixScaling(scaleFactor,scaleFactor,scaleFactor) };
        //primitiveData.world = world;
        DirectX::XMStoreFloat4x4(&primitiveData.world, C * DirectX::XMLoadFloat4x4(&world));
        immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
        immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

        const Material& material = materials.at(batchMesh.material);

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

        const int textureIndices[]
        {
            material.data.pbrMetallicRoughness.basecolorTexture.index,
            material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
            material.data.normalTexture.index,
            material.data.emissiveTexture.index,
            material.data.occlusionTexture.index,
        };
        //RenderState::BindRasterizerState(immediateContext, RASTER_STATE::SOLID_CULL_NONE);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        ID3D11ShaderResourceView* nullShaderResourceView{};
        std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
        for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
        {
            shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
        }
        immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

        if (batchMesh.indexBufferView.buffer > -1)
        {
            immediateContext->IASetIndexBuffer(buffers.at(batchMesh.indexBufferView.buffer).Get(), batchMesh.indexBufferView.format, 0);
            immediateContext->DrawIndexed(batchMesh.indexBufferView.sizeInBytes / _SizeofComponent(batchMesh.indexBufferView.format), 0, 0);
        }
        else
        {
            immediateContext->Draw(batchMesh.vertexBufferView.sizeInBytes / batchMesh.vertexBufferView.strideInBytes, 0);
        }
    }

#endif // 0
}