#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <vector>
// �T���v���X�e�[�g
enum class SAMPLER_STATE
{
    POINT,
    LINEAR,
    ANISOTROPIC,
    LINEAR_BORDER_BLACK,
    LINEAR_BORDER_WHITE,
    LINEAR_CLAMP,
    LINEAR_MIRROR,
    COMPARISON_LINEAR_BORDER_WHITE,     // SHADOW
    ENUM_COUNT,
};

//�f�v�X�X�e�[�g
enum class DEPTH_STATE
{
    ZT_ON_ZW_ON,    //�[�x�e�X�g����@�@�[�x�������݂���
    ZT_ON_ZW_OFF,
    ZT_OFF_ZW_ON,
    ZT_OFF_ZW_OFF,

    ENUM_COUNT,
};

//�u�����f�B���O�X�e�[�g
enum class BLEND_STATE
{
    NONE,
    ALPHA,
    ADD,
    MULTIPLY,
    MULTIPLY_RENDER_TARGET_ALPHA,
    MULTIPLY_RENDER_TARGET_NONE,
    ENUM_COUNT,
};

//���X�^���C�U�X�e�[�g
enum class RASTERRIZER_STATE
{
    SOLID_CULL_BACK,
    WIREFRAME_CULL_BACK,
    SOLID_CULL_NONE,
    WIREFRAME_CULL_NONE,
    USE_SCISSOR_RECTS,

    ENUM_COUNT,
};

// �����_�[�X�e�[�g
class RenderState
{
private:
    RenderState() = default;
    virtual ~RenderState() = default;
public:
    // �C���X�^���X�擾
    //static RenderState& Instance()
    //{
    //    static RenderState instance;
    //    return instance;
    //}
    // ������
    static void Initialize();

    // �T���v���X�e�[�g�ݒ�
    static void SetSamplerState(ID3D11DeviceContext* immediateContext);

    // �T���v���X�e�[�g�擾
    static ID3D11SamplerState* GetSamplerState(SAMPLER_STATE state) 
    {
        return samplerStates[static_cast<int>(state)].Get();
    }

    // �f�v�X�X�e�[�g�擾
    static ID3D11DepthStencilState* GetDepthStencilState(DEPTH_STATE state) 
    {
        return depthStencilStates[static_cast<int>(state)].Get();
    }

    // �u�����h�X�e�[�g�擾
    static ID3D11BlendState* GetBlendState(BLEND_STATE state) 
    {
        return blendStates[static_cast<int>(state)].Get();
    }

    // ���X�^���C�U�[�X�e�[�g�擾
    static ID3D11RasterizerState* GetRasterizerState(RASTERRIZER_STATE state) 
    {
        return rasterizerState[static_cast<int>(state)].Get();
    }

    static void BindDepthStencilState(ID3D11DeviceContext* immediate_context, DEPTH_STATE depth_stencil_state, UINT stencil_ref = 0)
    {
        immediate_context->OMSetDepthStencilState(depthStencilStates[static_cast<size_t>(depth_stencil_state)].Get(), stencil_ref);
    }

    //�S�ẴT���v���[�X�e�[�g�� PS GS CS ���ꂼ��ɐݒ�
    static void BindSamplerStates(ID3D11DeviceContext* immediate_context)
    {
        std::vector<ID3D11SamplerState*> samplers;
        for (const Microsoft::WRL::ComPtr<ID3D11SamplerState>& sampler : samplerStates)
        {
            samplers.emplace_back(sampler.Get());
        }
        immediate_context->PSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
        immediate_context->GSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
        immediate_context->CSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
    }

    static void BindBlendState(ID3D11DeviceContext* immediate_context, BLEND_STATE blend_state)
    {
        immediate_context->OMSetBlendState(blendStates[static_cast<size_t>(blend_state)].Get(), NULL, 0xFFFFFFFF);
    }

    static void BindRasterizerState(ID3D11DeviceContext* immediate_context, RASTERRIZER_STATE rasterizer_state)
    {
        immediate_context->RSSetState(rasterizerState[static_cast<size_t>(rasterizer_state)].Get());
    }

private:
    static inline Microsoft::WRL::ComPtr<ID3D11SamplerState>		samplerStates[static_cast<int>(SAMPLER_STATE::ENUM_COUNT)];
    static inline Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[static_cast<int>(DEPTH_STATE::ENUM_COUNT)];
    //�摜�̔w�i�F�𓧉߂�����
    static inline Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[static_cast<int>(BLEND_STATE::ENUM_COUNT)];
    //���C���[�t���[����`�悷�邽�߂̃��X�^���C�U
    static inline Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState[static_cast<int>(RASTERRIZER_STATE::ENUM_COUNT)];
};