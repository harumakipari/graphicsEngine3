#include "Shader.h"

#include <sstream>
#include <memory>

#include "Engine/Utility/Win32Utils.h"
#include "Engine/Debug/Assert.h"
HRESULT CreateVsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11VertexShader** vertexShader,
    ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc,
    UINT numElements)
{
    // �t�@�C���|�C���^��������
    FILE* fp{ nullptr };
    // CSO�t�@�C����ǂݍ��ނ��߂Ƀt�@�C�����o�C�i�����[�h�ŊJ��
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);     // �t�@�C���̖����Ɉړ�
    long cso_sz{ ftell(fp) };   // �t�@�C���T�C�Y���擾
    fseek(fp, 0, SEEK_SET);     // �t�@�C���̐擪�Ɉړ�

    // CSO�f�[�^���i�[���邽�߂̃��������m��
    std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
    fread(cso_data.get(), cso_sz, 1, fp);   // �t�@�C������f�[�^��ǂݍ���
    fclose(fp);     // �t�@�C�������

    HRESULT hr{ S_OK };
    // ���_�V�F�[�_�[���쐬
    hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertexShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    
    // ���̓��C�A�E�g���ݒ肳��Ă���΁A������쐬
    if (inputLayout)
    {
        hr = device->CreateInputLayout(inputElementDesc, numElements,
            cso_data.get(), cso_sz, inputLayout);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    return hr;
}


HRESULT CreatePsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11PixelShader** pixelShader)
{
    // �t�@�C���|�C���^��������
    FILE* fp{ nullptr };
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);     // �t�@�C���̖����Ɉړ�
    long cso_sz{ ftell(fp) };   // �t�@�C���T�C�Y���擾
    fseek(fp, 0, SEEK_SET);     // �t�@�C���̐擪�Ɉړ�

    // CSO�f�[�^���i�[���邽�߂̃��������m��
    std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
    fread(cso_data.get(), cso_sz, 1, fp);   // �t�@�C������f�[�^��ǂݍ���
    fclose(fp);     // �t�@�C�������

    HRESULT hr{ S_OK };
    // �s�N�Z���V�F�[�_�[���쐬
    hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixelShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}

HRESULT CreateGsFromCSO(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long csoSz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
    fread(csoData.get(), csoSz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateGeometryShader(csoData.get(), csoSz, nullptr, geometryShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}

HRESULT CreateCsFromCSO(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long csoSz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
    fread(csoData.get(), csoSz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateComputeShader(csoData.get(), csoSz, nullptr, computeShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}
