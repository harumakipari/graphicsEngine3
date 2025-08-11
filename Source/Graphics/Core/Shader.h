#pragma once
#include <wrl.h>
#include <d3d11.h>

HRESULT CreateVsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11VertexShader** vertexShader,
    ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc,
    UINT numElements);

HRESULT CreatePsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11PixelShader** pixelShader);

HRESULT CreateGsFromCSO(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader);

HRESULT CreateCsFromCSO(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader);
