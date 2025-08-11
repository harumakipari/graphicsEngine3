#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>
#include <functional>

class CascadedShadowMaps
{
public:
    CascadedShadowMaps(ID3D11Device* device, UINT width, UINT height, UINT cascadeCount = 4);
    virtual ~CascadedShadowMaps() = default;
    CascadedShadowMaps(const CascadedShadowMaps&) = delete;
    CascadedShadowMaps& operator=(const CascadedShadowMaps&) = delete;
    CascadedShadowMaps(CascadedShadowMaps&&) noexcept = delete;
    CascadedShadowMaps& operator=(CascadedShadowMaps&&)noexcept = delete;

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    Microsoft::WRL::ComPtr <ID3D11DepthStencilView> depthStencilView;
    D3D11_VIEWPORT viewport;

    std::vector<DirectX::XMFLOAT4X4> cascadedMatrices;
    std::vector<float> cascadedPlaneDistances;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

    struct Constants
    {
        DirectX::XMFLOAT4X4 cascadedMatrices[4];
        float cascadedPlaneDistances[4];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
    void Activate(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& cameraView, const DirectX::XMFLOAT4X4& cameraProjection,const DirectX::XMFLOAT4& lightDirection,
        float criticalDepthValue/* If this value is 0, the camera's far panel distance is used.*/,UINT cbSlot);
    void Deactive(ID3D11DeviceContext* immediateContext);
    void Clear(ID3D11DeviceContext* immediateContext)
    {
        immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1, 0);
    }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& depthMap()
    {
        return shaderResourceView;
    }

public:
    const UINT cascadeCount;
    //float splitSchemeWeight = 0.7f; // logarithmic_split_scheme * _split_scheme_weight + uniform_split_scheme * (1 - _split_scheme_weight)
    float splitSchemeWeight = 0.463f; // logarithmic_split_scheme * _split_scheme_weight + uniform_split_scheme * (1 - _split_scheme_weight)

    bool fitToCascade = true;

    //float zMult = 10.0f;
    float zMult = 8.0f;

private:
    D3D11_VIEWPORT catchedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    UINT viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> catchedRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> catchedDepthStencilView;
};