#pragma once
#include <map>
#include <memory>
#include <cassert>
#include "Actor.h"

#include "Graphics/Renderer/ShapeRenderer.h"
#include "Graphics/Core/ConstantBuffer.h"

#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Effect/EffectComponent.h"
#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Utils/ShockWaveTargetRegistry.h"

#include "Engine/Camera/CameraConstants.h"

class Scene;

class ActorManager
{
protected:
    Scene* ownerScene_ = nullptr;
public:
    void SetOwnerScene(Scene* scene) { ownerScene_ = scene; }
    Scene* GetOwnerScene() const { return ownerScene_; }
    // �A�N�^�[������A�N�^�[�ւ̃|�C���^�������Ɏ擾���邽�߂̃L���b�V���B
    // ���O��������Ȃ��ꍇ�̓A�N�^�[���X�g���������A���ʂ����̃L���b�V���ɕۑ�����B
    std::unordered_map<std::string, std::weak_ptr<Actor>> actorCacheByName_;
    // ���ݑ��݂��Ă��邷�ׂẴA�N�^�[
    std::vector<std::shared_ptr<Actor>> allActors_;

    // �A�N�^�[�𖼑O�t���ō쐬�E�o�^����i�����A�N�^�[�����݂���ꍇ��"_1","_2"�Ƃ��ă��j�[�N�Ȗ��O�ɂ���j ��ڂ̈����͏�������auto�ł��邩�ǂ��������肷��
    template <class T>
    std::shared_ptr<T> CreateAndRegisterActor(const std::string& actorName, bool autoInitialize = true)
    {
#if 0 // �����̎��Ɍx������
        auto findByName = [&actorName](const std::shared_ptr<Actor>& actor)
            {
                return actor->GetName() == actorName;
            };
        // ���O����v����A�N�^�[��T��
        std::vector<std::shared_ptr<Actor>>::iterator it = std::find_if(allActors_.begin(), allActors_.end(), findByName);

        // �����̃A�N�^�[�����łɑ��݂��Ă�����x��
        _ASSERT_EXPR(it == allActors_.end(), L"An actor with this name has already been registered.");
        std::shared_ptr<T> newActor = std::make_shared<T>(actorName);

#else // �����̎��Ƀ��j�[�N�Ȗ��O������
        // ����������� "_1", "_2", ... �����ă��j�[�N�Ȗ��O�ɂ���
        std::string finalName = actorName;
        int suffix = 1;
        auto nameExists = [&](const std::string& name)
            {
                return std::any_of(allActors_.begin(), allActors_.end(), [&](const std::shared_ptr<Actor>& actor)
                    {
                        return actor->GetName() == name;
                    });
            };

        while (nameExists(finalName))
        {
            finalName = actorName + "_" + std::to_string(suffix++);
        }
        std::shared_ptr<T> newActor = std::make_shared<T>(finalName);
        // Scene��n��
        newActor->SetOwnerScene(ownerScene_);  
        allActors_.push_back(newActor);
#endif
        if (autoInitialize)
        {
            newActor->Initialize();
            newActor->PostInitialize();
        }
        return newActor;
    }


    // �A�N�^�[�𖼑O�t���ō쐬�E�o�^����i�����A�N�^�[�����݂���ꍇ��"_1","_2"�Ƃ��ă��j�[�N�Ȗ��O�ɂ���j ��ڂ̈����͏�������auto�ł��邩�ǂ��������肷��
    template <class T>
    std::shared_ptr<T> CreateAndRegisterActorWithTransform(const std::string& actorName, const Transform& transform = {})
    {
#if 0 // �����̎��Ɍx������
        auto findByName = [&actorName](const std::shared_ptr<Actor>& actor)
            {
                return actor->GetName() == actorName;
            };
        // ���O����v����A�N�^�[��T��
        std::vector<std::shared_ptr<Actor>>::iterator it = std::find_if(allActors_.begin(), allActors_.end(), findByName);

        // �����̃A�N�^�[�����łɑ��݂��Ă�����x��
        _ASSERT_EXPR(it == allActors_.end(), L"An actor with this name has already been registered.");
        std::shared_ptr<T> newActor = std::make_shared<T>(actorName);

#else // �����̎��Ƀ��j�[�N�Ȗ��O������
        // ����������� "_1", "_2", ... �����ă��j�[�N�Ȗ��O�ɂ���
        std::string finalName = actorName;
        int suffix = 1;
        auto nameExists = [&](const std::string& name)
            {
                return std::any_of(allActors_.begin(), allActors_.end(), [&](const std::shared_ptr<Actor>& actor)
                    {
                        return actor->GetName() == name;
                    });
            };

        while (nameExists(finalName))
        {
            finalName = actorName + "_" + std::to_string(suffix++);
        }
        std::shared_ptr<T> newActor = std::make_shared<T>(finalName);
        // Scene��n��
        newActor->SetOwnerScene(ownerScene_);
        allActors_.push_back(newActor);
#endif
        newActor->Initialize(transform);
        newActor->PostInitialize();
        return newActor;
    }

    const std::vector<std::shared_ptr<Actor>>& GetAllActors() const
    {
        return allActors_;
    }

    // ���O����A�N�^�[���擾�i�L���b�V���t�������j
    std::shared_ptr<Actor> GetActorByName(const std::string& actorName)
    {
        // �L���b�V���ɂ���΂����Ԃ�
        auto cached = actorCacheByName_.find(actorName);
        if (cached != actorCacheByName_.end())
        {
            if (!cached->second.expired())
            {
                return cached->second.lock();
            }
        }

        // �Ȃ���ΑS�A�N�^�[����T��
        auto found = std::find_if(allActors_.begin(), allActors_.end(),
            [&actorName](const std::shared_ptr<Actor>& actor) {
                return actor->GetName() == actorName;
            });

        // ���������ꍇ�̓L���b�V�����ĕԂ�
        if (found != allActors_.end()) {
            actorCacheByName_[actorName] = *found;
            return *found;
        }

        // ������Ȃ�����
        return nullptr;
    }

    // �o�^�ς݃A�N�^�[�ƃL���b�V�������ׂăN���A����
    void ClearAll()
    {
        for (const std::shared_ptr<Actor>& actor : allActors_)
        {
            //for (std::shared_ptr<SceneComponent>& component : actor->ownedSceneComponents_)
            //{
            //    component->OnUnregister();
            //}
            //for (std::shared_ptr<Component>& component : actor->ownedLogicComponents_)
            //{
            //    component->OnUnregister();
            //}
            //actor->Finalize();
            actor->Destroy();
        }
        allActors_.clear();
        actorCacheByName_.clear();
    }

    // Actor�̃|�C���^���ꊇ�Ő��|�C���^�`���Ŏ擾����i�`���V�[���p�ȂǂɁj
    void ConvineActor(std::vector<Actor*>& outActorPointers)
    {
        outActorPointers.clear();
        for (const std::shared_ptr<Actor>& actor : allActors_)
        {
            outActorPointers.push_back(actor.get());
        }
    }

    // �S�A�N�^�[��Update�������Ăяo���iRootComponent��OwnedComponent�j
    void Update(float deltaTime)
    {
        // allActors_ �̃R�s�[�����i��Q�ƂȂ�shared_ptr���R�s�[�����j
        auto updateActors = allActors_;

        for (auto it = updateActors.begin(); it != updateActors.end(); ++it)
        {
            auto& actor = *it;
            if (!actor /*|| !actor->isActive*/) continue;

            //char buf[256];
            //sprintf_s(buf, "Update Loop: actor=%s, isValid=%d, isActive=%d\n", actor->GetName().c_str(), actor->isValid, actor->isActive);
            //OutputDebugStringA(buf);

            //if (!actor->isValid)
            if (actor->isPendingDestroy)
            {
                //char buf[256];
                //sprintf_s(buf, "actor=%s, isValid=%d, isActive=%d\n �� Destroy() ���ĂԁI\n", actor->GetName().c_str(), actor->isValid, actor->isActive);
                //OutputDebugStringA(buf);

                //OutputDebugStringA(" �� Destroy() ���ĂԁI\n");
                actor->Destroy();
                continue;
            }

            for (auto& component : actor->ownedSceneComponents_)
            {
                component->Tick(deltaTime);
            }

            //for (auto& component : actor->ownedLogicComponents_)
            //{
            //    component->Tick(deltaTime);
            //}

            if (actor->rootComponent_)
            {
                actor->rootComponent_->UpdateComponentToWorld();
            }
            actor->Update(deltaTime);

            //if (!actor->isValid)
            //{
            //    actor->Destroy();
            //    continue;
            //}

            actor->PostDestroyComponents();
        }

        // isValid == false �̃A�N�^�[�������폜
        allActors_.erase(
            std::remove_if(allActors_.begin(), allActors_.end(),
                [](const std::shared_ptr<Actor>& a) { return !a || !a->isValid; }),
            allActors_.end());
    }

    void DrawImGuiAllActors()
    {
#ifdef USE_IMGUI
        // ��ʃT�C�Y���擾
        ImGuiIO& io = ImGui::GetIO();
        float windowWidth = io.DisplaySize.x * 0.25f;
        float windowHeight = io.DisplaySize.y;

        // ���̃E�B���h�E�̈ʒu�ƃT�C�Y���w��
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - windowWidth, 0));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

        // �t���O�����ČŒ�\���Ɂi�T�C�Y�ύX�Ȃǂ��֎~�������ꍇ�j
        ImGui::Begin("Actor Inspector", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse
        );

        for (const auto& actor : allActors_)
        {
            actor->DrawImGuiInspector();
        }

        ImGui::End();
#endif
    }
};

class Renderer
{
private:
    std::unique_ptr<ConstantBuffer<ViewConstants>> viewBuffer;

public:
    Renderer()
    {
        ID3D11Device* device = Graphics::GetDevice();
        itemModel = std::make_shared<InterleavedGltfModel>(device, "./Data/Models/Items/PickUpEnergyCore/pick_up_item.gltf", InterleavedGltfModel::Mode::InstancedStaticMesh);
        CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        itemModel->emission = 3.0f;

        viewBuffer = std::make_unique<ConstantBuffer<ViewConstants>>(device);
    }

    virtual ~Renderer() {}

    // View�֘A�̒萔�o�b�t�@���X�V����
    void UpdateViewConstants(ID3D11DeviceContext* immediateContext, ViewConstants data)
    {
        viewBuffer->data = data;
        viewBuffer->Activate(immediateContext, 8);
    }

    void RenderParticle(ID3D11DeviceContext* immediateContext);

    void RenderOpaque(ID3D11DeviceContext* immediateContext);

    void RenderMask(ID3D11DeviceContext* immediateContext);

    void RenderBlend(ID3D11DeviceContext* immediateContext);
    
    void RenderInstanced(ID3D11DeviceContext* immediateContext);

    std::vector<DirectX::XMFLOAT4X4> instanceDatas;

#if 0
    void RenderMesh(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animated_nodes, RenderPass pass, const PipeLineState& pipeline)
    {
        const std::vector<Node>& nodes = animated_nodes.size() > 0 ? animated_nodes : InterleavedGltfModel::nodes;

        immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

        immediateContext->VSSetShader(pipeline.vertexShader ? pipeline.vertexShader.Get() : vertexShader.Get(), nullptr, 0);
        //immediateContext->PSSetShader(pipeline.pixelShader ? pipeline.pixelShader.Get() : pixelShader.Get(), nullptr, 0);
        immediateContext->IASetInputLayout(inputLayout.Get());
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        std::function<void(int)> traverse = [&](int nodeIndex)->void {
            const Node& node = nodes.at(nodeIndex);
            if (node.skin > -1)
            {
                const Skin& skin = skins.at(node.skin);
                _ASSERT_EXPR(skin.joints.size() <= PRIMITIVE_MAX_JOINTS, L"The size of the joint array is insufficient, please expand it.");
                PrimitiveJointConstants primitiveJointData{};
                for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex)
                {
                    DirectX::XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                        DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) *
                        DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) *
                        DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform))
                    );
                }
                immediateContext->UpdateSubresource(primitiveJointCbuffer.Get(), 0, 0, &primitiveJointData, 0, 0);
                immediateContext->VSSetConstantBuffers(2, 1, primitiveJointCbuffer.GetAddressOf());
            }
            if (node.mesh > -1)
            {
                const Mesh& mesh = meshes.at(node.mesh);
                for (const Mesh::Primitive& primitive : mesh.primitives)
                {
                    // INTERLEAVED_GLTF_MODEL
                    UINT stride = sizeof(Mesh::Vertex);
                    UINT offset = 0;
                    immediateContext->IASetVertexBuffers(0, 1, buffers.at(primitive.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

                    PrimitiveConstants primitiveData = {};
                    primitiveData.material = primitive.material;
                    primitiveData.hasTangent = primitive.has("TANGENT");
                    primitiveData.skin = node.skin;
                    primitiveData.color = { cpuColor.x,cpuColor.y,cpuColor.z,alpha };
                    primitiveData.emission = emission;
                    primitiveData.disolveFactor = disolveFactor;
                    // �����Ń��f�����W�n��ϊ�����H
                    //���W�n�̕ϊ����s��
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
                        scaleFactor = 1.0f;//���[�g���P�ʂ̎�
                    }
                    else
                    {
                        scaleFactor = 0.01f;//�p�P�ʂ̎�
                    }
                    DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[static_cast<int>(modelCoordinateSystem)]) * DirectX::XMMatrixScaling(scaleFactor,scaleFactor,scaleFactor) };

                    DirectX::XMStoreFloat4x4(&primitiveData.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * C * DirectX::XMLoadFloat4x4(&world));
                    immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
                    immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
                    immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

                    //int currentMaterialIndex = primitive.GetCurrentMaterialIndex();
                    //auto& material = materials[currentMaterialIndex];
                    const Material& material = materials.at(primitive.material);
                    //�����Őݒ�
                    if (material.replacedPixelShader)
                    {
                        immediateContext->PSSetShader(material.replacedPixelShader.Get(), nullptr, 0);
                    }
                    else
                    {
                        immediateContext->PSSetShader(pipeline.pixelShader ? pipeline.pixelShader.Get() : pixelShader.Get(), nullptr, 0);
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
                    case RenderPass::All:
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
                        shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
                    }
                    immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

                    if (primitive.indexBufferView.buffer > -1)
                    {
                        // INTERLEAVED_GLTF_MODEL
                        immediateContext->IASetIndexBuffer(buffers.at(primitive.indexBufferView.buffer).Get(), primitive.indexBufferView.format, 0);
                        immediateContext->DrawIndexed(primitive.indexBufferView.sizeInBytes / _SizeofComponent(primitive.indexBufferView.format), 0, 0);
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
        for (std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes)
        {
            traverse(nodeIndex);
        }

    }

#endif // 0
private:
    std::shared_ptr<InterleavedGltfModel> itemModel;
    PipeLineStateDesc pipeLineState_ = {};
public:
    void RenderBuilding(ID3D11DeviceContext* immediateContext)
    {
        for (const auto& actor : ShockWaveTargetRegistry::GetTargets())
        {
            if (!actor->rootComponent_)
            {
                continue;
            }

            if (!actor->isActive)
            {// actor�����݂��Ă��Ȃ�������X�L�b�v
                continue;
            }

            std::vector<BuildMeshComponent*> meshComponents;
            actor->GetComponents<BuildMeshComponent>(meshComponents);

            for (const BuildMeshComponent* meshComponent : meshComponents)
            {
                //  �e MeshComponent ���g�̍ŐV���[���h�s������o��
                const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
                //bool rendered = false;

                if (/*!rendered &&*/ meshComponent->IsVisible())
                {
                    //  �`��Ăяo���� meshComponent �x�[�X�̍s���n��
                    meshComponent->RenderOpaque(immediateContext, worldMat);
                    meshComponent->RenderMask(immediateContext, worldMat);
                    meshComponent->RenderBlend(immediateContext, worldMat);
                }
            }
        }
    }
    void CastShadowRender(ID3D11DeviceContext* immediateContext);
};



class ActorColliderManager
{
public:
    ActorColliderManager()
    {

    }

    //�R���W�����̃��C���[����
    // ���f���V�F�[�_�[�̎�ށ@ShaderID

    static inline std::vector<std::pair<Actor*, ShapeComponent*>> allShapes;

    void DebugRender(ID3D11DeviceContext* immediateContext);

    //�f�o�b�N��`��
    //std::unique_ptr<ShapeRenderer> shapeRenderer = nullptr;
    //�f�o�b�N�̐F�����
    DirectX::XMFLOAT4 debugColor = { 1.0f,1.0f,0.0f,1.0f };
};


