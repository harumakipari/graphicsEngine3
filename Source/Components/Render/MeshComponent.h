#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

// C++ 標準ライブラリ
#include <memory>
#include <string>


// 他ライブラリ
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif


// プロジェクトの他のヘッダ
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Core/PipelineState.h"
#include "Components/Base/SceneComponent.h"
#include "Graphics/Resource/InterleavedGltfModel.h"
#include "Engine/Utility/Win32Utils.h"

class Actor;

//--　描画
class MeshComponent :public SceneComponent
{
public:
    PipeLineStateDesc pipeLineState_;
    std::optional<std::string> overridePipelineName;
public:
    MeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {};
    std::shared_ptr<InterleavedGltfModel> model;
    // モデルのノード情報
    std::vector<InterleavedGltfModel::Node> modelNodes = {};

    virtual void Tick(float deltaTime)override
    {
    }
    virtual void SetModel(const std::string& fileName, bool isSaveVerticesData = false) = 0;

    virtual void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;
    virtual void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;
    virtual void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;

    virtual void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;

    virtual void SetIsVisible(bool isVisible) { this->isVisible_ = isVisible; }

    virtual bool IsVisible() const { return isVisible_; }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  model").c_str()))
        {
            ImGui::Checkbox("isVisible", &isVisible_);
            ImGui::TreePop();
        }
#endif
    }

    void SetPipeLineState(const PipeLineStateDesc& pipelinesState) { this->pipeLineState_ = pipelinesState; }

    PipeLineStateDesc GetPipeLineState()const { return pipeLineState_; }

    void SetIsCastShadow(bool isCastShadow) { this->isCastShadow_ = isCastShadow; }

    virtual bool IsCastShadow() const { return isCastShadow_; }
protected:
    //描画するかどうか
    bool isVisible_ = true;
    // 影をつけるかどうか
    bool isCastShadow_ = true;
};

class SkeltalMeshComponent :public MeshComponent
{
public:
    SkeltalMeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :MeshComponent(name, owner)
    {
    }

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::SkeltalMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }

    void AppendAnimations(const std::vector<std::string>& filenames)
    {
        model->AddAnimations(filenames);
    }

    void Tick(float deltaTime)override
    {

    }

    void SetMaterialPS(const std::string& psFilename, const std::string& materialName)
    {
        ID3D11Device* device = Graphics::GetDevice();
        for (InterleavedGltfModel::Material& material : model->materials)
        {
            if (material.name == materialName)
            {
                CreatePsFromCSO(device, psFilename.c_str(), material.replacedPixelShader.GetAddressOf());
            }
        }
    }

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->CastShadow(immediateContext, world, model->nodes);
        model->CastShadow(immediateContext, world, modelNodes);
    }

    DirectX::XMFLOAT3 GetJointWorldPosition(const std::string& name)
    {
        if (auto parent = attachParent_.lock())
        {
            DirectX::XMFLOAT4X4 parentWorld = parent->GetComponentWorldTransform().ToWorldTransform();
            //return model->GetJointWorldPosition(name, model->nodes, parentWorld);
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        else
        {
            DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
            //return model->GetJointWorldPosition(name, model->nodes, world);
            return model->GetJointWorldPosition(name, modelNodes, world);
        }

        return { 0.0f,0.0f,0.0f };
    }

private:

};

class BuildMeshComponent :public SceneComponent
{
public:
    PipeLineStateDesc pipeLineState_;
    // モデルのノード情報
    std::vector<InterleavedGltfModel::Node> modelNodes = {};

    BuildMeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner)
    {
    }

    virtual ~BuildMeshComponent() {}

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::SkeltalMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }

    void AppendAnimations(const std::vector<std::string>& filenames)
    {
        model->AddAnimations(filenames);
    }

    void Tick(float deltaTime)override
    {

    }

    void SetMaterialPS(const std::string& psFilename, const std::string& materialName)
    {
        ID3D11Device* device = Graphics::GetDevice();
        for (InterleavedGltfModel::Material& material : model->materials)
        {
            if (material.name == materialName)
            {
                CreatePsFromCSO(device, psFilename.c_str(), material.replacedPixelShader.GetAddressOf());
            }
        }
    }

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const
    {
        //model->Animate(animationClip, animationTime, model->nodes);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        //model->Animate(animationClip, animationTime, model->nodes);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        //model->Animate(animationClip, animationTime, model->nodes);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        //model->Animate(animationClip, animationTime, model->nodes);
        model->CastShadow(immediateContext, world, modelNodes);
    }

    DirectX::XMFLOAT3 GetJointWorldPosition(const std::string& name)
    {
        if (auto parent = attachParent_.lock())
        {
            DirectX::XMFLOAT4X4 parentWorld = parent->GetComponentWorldTransform().ToWorldTransform();
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        else
        {
            DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
            return model->GetJointWorldPosition(name, modelNodes, world);
        }

        return { 0.0f,0.0f,0.0f };
    }
    std::shared_ptr<InterleavedGltfModel> model;

    void SetIsCastShadow(bool isCastShadow) { this->isCastShadow_ = isCastShadow; }

    virtual bool IsCastShadow() const { return isCastShadow_; }

    virtual void SetIsVisible(bool isVisible) { this->isVisible_ = isVisible; }

    virtual bool IsVisible() const { return isVisible_; }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  model").c_str()))
        {
            ImGui::Checkbox("isVisible", &isVisible_);
            ImGui::TreePop();
        }
#endif
    }

    void SetPipeLineState(const PipeLineStateDesc& pipelinesState) { this->pipeLineState_ = pipelinesState; }

    PipeLineStateDesc GetPipeLineState()const { return pipeLineState_; }

protected:
    //描画するかどうか
    bool isVisible_ = true;
    // 影をつけるかどうか
    bool isCastShadow_ = true;

};

class StaticMeshComponent :public MeshComponent
{
public:
    StaticMeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :MeshComponent(name, owner)
    {
    }


    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::StaticMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }

    //void Update(float deltaTime)override {}

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->CastShadow(immediateContext, world, modelNodes);
    }
};


class InstancedStaticMeshComponent :public StaticMeshComponent
{
public:
    InstancedStaticMeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :StaticMeshComponent(name, owner)
    {
    }
    //// モデルのノード情報
    //std::vector<InterleavedGltfModel::Node> modelNodes = {};

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::InstancedStaticMesh, isSaveVerticesData);
        model->SetMeshComponent(this);
        modelNodes = model->GetNodes();
    }


    int AddInstance(const Transform& transform)
    {
        DirectX::XMFLOAT4X4 instanceMatrix;
        DirectX::XMStoreFloat4x4(&instanceMatrix, transform.ToMatrix());
        instanceMatrices_.push_back(instanceMatrix);
        return static_cast<int>(instanceMatrices_.size()) - 1;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;

    bool CreateInstancingBuffer(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
#if 0 // これ動くとき
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
#else
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.CPUAccessFlags = 0;
#endif
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT4X4) * instanceMatrices_.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pSysMem = instanceMatrices_.data();
        HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, buffer_.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        return hr == S_OK;
    }

    UINT instanceCount()const
    {
        return static_cast<UINT>(instanceMatrices_.size());
    }

    //void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    //{
    //    //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
    //    model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    //}
    //void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    //{
    //    //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
    //    model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    //}
    //void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    //{
    //    //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
    //    model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    //}


private:
    std::vector<DirectX::XMFLOAT4X4> instanceMatrices_;
};

#endif  //MESH_COMPONENT_H