#ifndef PIPELINELIBRARY_H
#define PIPELINELIBRARY_H

#include <string>
#include <unordered_map>

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Win32Utils.h"

class PipeLineState
{
private:

};


enum RenderPath
{
    Forward,
    Defferd,
    Shadow,
};

enum MaterialAlphaMode
{
    MaterialOpaque,
    MaterialMask,
    MaterialBlend,
};

enum ModelMode
{
    SkeltalComponent,
    StaticComponent,
    InstanceComponent,
};

constexpr const char* const SkeltalMesh_ForwardNames[] =
{
    "forwardOpaqueSkeltalMesh",
    "forwardMaskSkeltalMesh",
    "forwardBlendSkeltalMesh"
};

constexpr const char* const SkeltalMesh_DeferredNames[] =
{
    "deferredOpaqueSkeltalMesh",
    "deferredMaskSkeltalMesh",
    "deferredBlendSkeltalMesh"
};

constexpr const char* const SkeltalMesh_ShadowNames[] =
{
    "CascadeShadowMapSkeltalMesh"
};

constexpr const char* const* const SkeltalMesh_PipelineNames[] =
{
    SkeltalMesh_ForwardNames,
    SkeltalMesh_DeferredNames,
    SkeltalMesh_ShadowNames
};

// RenderPath と materialAlphaMode からパイプライン名を作成する関数
inline std::string GetPipelineName(RenderPath renderPath, MaterialAlphaMode alphaMode, ModelMode modelMode)
{
    switch (modelMode)
    {
    case ModelMode::SkeltalComponent:
        switch (renderPath)
        {
        case RenderPath::Forward:
            switch (alphaMode)
            {
            case MaterialOpaque:
                return "forwardOpaqueSkeltalMesh";
                break;
            case MaterialMask:
                return "forwardMaskSkeltalMesh";
                break;
            case MaterialBlend:
                return "forwardBlendSkeltalMesh";
                break;
            default:
                break;
            }
            break;
        case RenderPath::Defferd:
            switch (alphaMode)
            {
            case MaterialOpaque:
                return "defferdOpaqueSkeltalMesh";
                break;
            case MaterialMask:
                return "defferdMaskSkeltalMesh";
                break;
            case MaterialBlend:
                return "defferdBlendSkeltalMesh";
                break;
            default:
                break;
            }
            break;
        case RenderPath::Shadow:
            return "CascadeShadowMapSkeltalMesh";
            break;
        default:
            break;
        }

        break;
    case ModelMode::StaticComponent:
        switch (renderPath)
        {
        case RenderPath::Forward:
            switch (alphaMode)
            {
            case MaterialOpaque:
                return "forwardOpaqueStaticMesh";
                break;
            case MaterialMask:
                return "forwardMaskStaticMesh";
                break;
            case MaterialBlend:
                return "forwardBlendStaticMesh";
                break;
            default:
                break;
            }
            break;
        case RenderPath::Defferd:
            switch (alphaMode)
            {
            case MaterialOpaque:
                return "defferdOpaqueStaticMesh";
                break;
            case MaterialMask:
                return "defferdMaskStaticMesh";
                break;
            case MaterialBlend:
                return "defferdBlendStaticMesh";
                break;
            default:
                break;
            }
            break;
        case RenderPath::Shadow:
            return "CascadeShadowMapStaticMesh";
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
    // TODO:000ここに出力ウィンドウを出す
    return "";
}

class PipeLineStateSet
{
public:
    PipeLineStateSet() = default;
    virtual ~PipeLineStateSet() {};

    // StaticMesh のパイプラインの設定する関数
    void InitStaticMesh(ID3D11Device* device)
    {
        HRESULT hr = S_OK;
        PipeLineStateDesc desc;

        // StaticMesh
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        desc.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        desc.rasterState = RASTERRIZER_STATE::SOLID_CULL_BACK;
        desc.depthState = DEPTH_STATE::ZT_ON_ZW_ON;
        hr = CreateVsFromCSO(device, "./Shader/GltfModelStaticBatchingVS.cso", desc.vertexShader.ReleaseAndGetAddressOf(), desc.inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));

        // StaticMesh forward Opaque 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("forwardOpaqueStaticMesh", desc);
        }

        // StaticMesh defferd Opaque 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("defferdOpaqueStaticMesh", desc);
        }

        // StaticMesh forward Mask 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("forwardMaskStaticMesh", desc);
        }

        // StaticMesh defferd Mask 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("defferdMaskStaticMesh", desc);
        }

        // StaticMesh forward Blend 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA;
            AddPipeLineState("forwardBlendStaticMesh", desc);
        }

        // StaticMesh defferd Blend 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA;
            AddPipeLineState("defferdBlendStaticMesh", desc);
        }

        // StaticMesh Cascade ShadowMap 用
        {
            desc.pixelShader = nullptr;
            hr = CreateVsFromCSO(device, "./Shader/GltfModelStaticBatchingCsmVS.cso", desc.vertexShader.ReleaseAndGetAddressOf(), NULL, NULL, 0);
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            hr = CreateGsFromCSO(device, "./Shader/GltfModelCsmGS.cso", desc.gemetryShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            desc.blendState = BLEND_STATE::NONE;
            AddPipeLineState("CascadeShadowMapStaticMesh", desc);
        }
    }

    // SkeltalMesh のパイプラインの設定する関数
    void InitSkeltalMesh(ID3D11Device* device)
    {
        HRESULT hr = S_OK;
        PipeLineStateDesc desc;
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "JOINTS", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        desc.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        hr = CreateVsFromCSO(device, "./Shader/GltfModelVS.cso", desc.vertexShader.ReleaseAndGetAddressOf(), desc.inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        desc.rasterState = RASTERRIZER_STATE::SOLID_CULL_BACK;
        desc.depthState = DEPTH_STATE::ZT_ON_ZW_ON;

        // SkeltalMesh forward Opaque 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("forwardOpaqueSkeltalMesh", desc);
        }

        // SkeltalMesh defferd Opaque 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("defferdOpaqueSkeltalMesh", desc);
        }

        // SkeltalMesh forward Mask 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("forwardMaskSkeltalMesh", desc);
        }

        // SkeltalMesh defferd Mask 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE;
            AddPipeLineState("defferdMaskSkeltalMesh", desc);
        }

        // SkeltalMesh forward Blend 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA;
            AddPipeLineState("forwardBlendSkeltalMesh", desc);
        }

        // SkeltalMesh defferd Blend 用
        {
            hr = CreatePsFromCSO(device, "./Shader/GltfModelDefferedPS.cso", desc.pixelShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            desc.blendState = BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA;
            AddPipeLineState("defferdBlendSkeltalMesh", desc);
        }

        // SkeltalMesh Cascade ShadowMap 用
        {
            desc.pixelShader = nullptr;
            hr = CreateVsFromCSO(device, "./Shader/GltfModelCsmVS.cso", desc.vertexShader.ReleaseAndGetAddressOf(), NULL, NULL, 0);
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            hr = CreateGsFromCSO(device, "./Shader/GltfModelCsmGS.cso", desc.gemetryShader.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            AddPipeLineState("CascadeShadowMapSkeltalMesh", desc);
        }
    }

    const PipeLineStateDesc& Get(const std::string& name)const
    {
        auto it = sets_.find(name);
        if (it == sets_.end())
        {// 見つからなかったら
            _ASSERT(L"このパイプラインステートは設定されていません。");
        }
        return it->second;
    }

    void AddPipeLineState(const std::string name, const PipeLineStateDesc& state)
    {
        sets_[name] = state;
    }

    void BindPipeLineState(ID3D11DeviceContext* immediateContext, const std::string& name)
    {
        auto it = sets_.find(name);
        if (it == sets_.end())
        {// 見つからなかったら
            _ASSERT(L"このパイプラインステートは設定されていません。");
        }

        immediateContext->IASetInputLayout(sets_[name].inputLayout.Get());
        immediateContext->IASetPrimitiveTopology(sets_[name].primitiveTopology);

        immediateContext->VSSetShader(sets_[name].vertexShader.Get(), nullptr, 0);
        immediateContext->PSSetShader(sets_[name].pixelShader.Get(), nullptr, 0);
        immediateContext->DSSetShader(sets_[name].domainShader.Get(), nullptr, 0);
        immediateContext->GSSetShader(sets_[name].gemetryShader.Get(), nullptr, 0);
        immediateContext->HSSetShader(sets_[name].hullShader.Get(), nullptr, 0);

        RenderState::BindDepthStencilState(immediateContext, sets_[name].depthState);
        RenderState::BindBlendState(immediateContext, sets_[name].blendState);
        RenderState::BindRasterizerState(immediateContext, sets_[name].rasterState);
    }


private:
    std::unordered_map<std::string, PipeLineStateDesc> sets_;
};




#endif // !PIPELINELIBRARY_H
