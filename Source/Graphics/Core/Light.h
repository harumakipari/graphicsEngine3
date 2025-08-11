#ifndef LIGHT_H
#define LIGHT_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <unordered_map>
#include <vector>

#include "Graphics/Core/Graphics.h"
#include "Engine/Utility/Win32Utils.h"

enum class LightType
{
    Directional,	//  平行光源
    Point,			//  点光源
    Spot,			//  スポットライト
};

struct DirectionalLights
{
    DirectX::XMFLOAT4 direction = { 1.0f,-1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
};

struct PointLights
{
    DirectX::XMFLOAT4 position = { 0.0f,0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
    float range = 0.0f;
    DirectX::XMFLOAT3 dummy;
};

struct SpotLights
{
    DirectX::XMFLOAT4 position = { 0.0f,0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 direction = { 1.0f,-1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
    float range{ 0 };
    float innerCorn{ DirectX::XMConvertToRadians(30) };
    float outerCorn{ DirectX::XMConvertToRadians(45) };
    float dummy;
};

struct LightProbes
{
    DirectX::XMFLOAT4 position = { 0.0f,0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 innerArea = { 100, 100, 100, 0 };
    DirectX::XMFLOAT4 outerArea = { 200, 200, 200, 0 };
};


class Light
{
public:
    Light(/*LightType type*/)/* :type(type)*/
    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(LightConstants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        HRESULT hr = Graphics::GetDevice()->CreateBuffer(&bufferDesc, nullptr, lightConstantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    struct LightConstants
    {
        DirectX::XMFLOAT4 ambientColor = { 1.0f,1.0f,1.0f,1.0f };
        DirectX::XMFLOAT4 directionalLightDirection = { 1.0f,1.0f,1.0f,1.0f };
        DirectX::XMFLOAT4 directionalLightColor = { 1.0f,1.0f,1.0f,1.0f };
        float iblIntensity = 1.0f;
        float pads[3];
        SpotLights spotLights[8];
        PointLights pointLights[8];
    };

    DirectX::XMFLOAT4 ambientColor = { 1.0f,1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 directionalLightDirection = { 1.0f,1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 directionalLightColor = { 1.0f,1.0f,1.0f,1.0f };

    Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;

    void Make(ID3D11DeviceContext* immediateContext)
    {
        LightConstants data = {};
        data.ambientColor = ambientColor;
        data.directionalLightColor = directionalLightColor;
        data.directionalLightDirection = directionalLightDirection;
        immediateContext->UpdateSubresource(lightConstantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->PSSetConstantBuffers(9, 1, lightConstantBuffer.GetAddressOf());
    }
private:
    LightType type;
};

#endif // !LIGHT_H
