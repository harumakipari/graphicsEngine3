#include "Texture.h"
#include <memory>
#include <filesystem>
#include <DDSTextureLoader.h>

#include <WICTextureLoader.h>
#include <wrl.h>
#include <map>
#include <string>   

#include "Engine/Utility/Win32Utils.h"
using namespace DirectX;

using namespace std;

#include <wrl/client.h>
using namespace Microsoft::WRL;

static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;


// �e�N�X�`�����t�@�C������ǂݍ��݁A�V�F�[�_�[���\�[�X�r���[���쐬����֐�
HRESULT LoadTextureFromFile(ID3D11Device* device,
    const wchar_t* filename, ID3D11ShaderResourceView** shaderResourceView,
    D3D11_TEXTURE2D_DESC* texture2dDesc)
{
    HRESULT hr{ S_OK };
    ComPtr<ID3D11Resource> resource;// DirectX�̃��\�[�X�i�e�N�X�`���j�̊i�[�ꏊ

    // ���łɃ��[�h���ꂽ�e�N�X�`�����L���b�V���ɂ��邩�m�F
    auto it = resources.find(filename);// resources�Ƃ����L���b�V���̒���filename�ɑΉ�����e�N�X�`����T��
    if (it != resources.end())// ���������ꍇ
    {
        *shaderResourceView = it->second.Get();// �����̃��\�[�X�r���[���擾
        (*shaderResourceView)->AddRef();// �Q�ƃJ�E���g��1���₵�ă��\�[�X��ی�
        (*shaderResourceView)->GetResource(resource.GetAddressOf());// ���\�[�X���̂��擾
    }
    else  // ������Ȃ��ꍇ�A�V�����ǂݍ���
    {
        //DDS�t�@�C�������[�h���ăV�F�[�_�[���\�[�X�r���[
        std::filesystem::path ddsFilename(filename);
        ddsFilename.replace_extension("dds");
        if (std::filesystem::exists(ddsFilename.c_str()))
        {
            hr = CreateDDSTextureFromFile(device, ddsFilename.c_str(), resource.GetAddressOf(), shaderResourceView);
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }
        else
        {
            hr = CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shaderResourceView);
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }
        resources.insert(make_pair(filename, *shaderResourceView));
    }

    if (texture2dDesc)
    {
        // �ǂݍ��񂾃e�N�X�`����ID3D11Texture2D�i2D�e�N�X�`���̌`���j�ɕϊ�
        ComPtr<ID3D11Texture2D> texture2d;// 2D�e�N�X�`���p�̃|�C���^
        hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf()); // �ϊ�
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        // �ϊ������e�N�X�`���̏��i�𑜓x�A�t�H�[�}�b�g�Ȃǁj���擾
        texture2d->GetDesc(texture2dDesc);
    }

    return hr;
}

//�_�~�[�e�N�X�`���̍쐬
HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView,
    DWORD value/*0xAABBGGRR*/, UINT dimension)
{
    HRESULT hr{ S_OK };

    D3D11_TEXTURE2D_DESC texture2dDesc{};
    texture2dDesc.Width = dimension;
    texture2dDesc.Height = dimension;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    size_t texels = dimension * dimension;
    std::unique_ptr<DWORD[]> sysmem{ std::make_unique<DWORD[]>(texels) };
    for (size_t i = 0; i < texels; i++)
    {
        sysmem[i] = value;
    }

    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = sysmem.get();
    subresourceData.SysMemPitch = sizeof(DWORD) * dimension;

    ComPtr<ID3D11Texture2D> texture2d;
    hr = device->CreateTexture2D(&texture2dDesc, &subresourceData, &texture2d);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = texture2dDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc, shaderResourceView);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}


HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView)
{
    HRESULT hr{ S_OK };
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;

    hr = CreateDDSTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
    if (hr != S_OK)
    {
        hr = CreateWICTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    return hr;
}


// �L���b�V�����ꂽ���ׂẴe�N�X�`�����������֐�
void ReleaseAllTextures()
{
    resources.clear();// �e�N�X�`���L���b�V�����N���A����
}