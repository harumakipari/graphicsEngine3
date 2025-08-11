#include "SkyMap.h"
#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/Shader.h"

// SkyMap �N���X�̃R���X�g���N�^
SkyMap::SkyMap(ID3D11Device* device, const wchar_t* filename, bool generateMips)
{
    HRESULT hr = S_OK;

    // �e�N�X�`�����t�@�C������ǂݍ���
    D3D11_TEXTURE2D_DESC texture2dDesc;
    LoadTextureFromFile(device, filename, shaderResourceView.GetAddressOf(), &texture2dDesc);

    // �e�N�X�`�����L���[�u�}�b�v���ǂ�������
    if (texture2dDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
    {
        isTexturecube = true;
    }

    // ���_�V�F�[�_�[�̓ǂݍ���
    hr = CreateVsFromCSO(device, "./Shader/SkyMapVS.cso", skyMapVs.GetAddressOf(), NULL, NULL, 0);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    // �s�N�Z���V�F�[�_�[�̓ǂݍ��݁i�ʏ�̃X�J�C�}�b�v�p�j
    hr = CreatePsFromCSO(device, "./Shader/SkyMapPS.cso", skyMapPs.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    // �s�N�Z���V�F�[�_�[�̓ǂݍ��݁i�X�J�C�{�b�N�X�p�j
    hr = CreatePsFromCSO(device, "./Shader/SkyBoxPS.cso", skyBoxPs.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // �萔�o�b�t�@�̍쐬�i�V�F�[�_�[�ɓn���f�[�^���i�[����j
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Constants);// �萔�o�b�t�@�̃T�C�Y
    bufferDesc.Usage = D3D11_USAGE_DEFAULT; // GPU ����̂݃A�N�Z�X�\
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // �萔�o�b�t�@�Ƃ��ăo�C���h
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// SkyMap ��`�悷��֐�
void SkyMap::Blit(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& viewProjection)
{
    // ���_�o�b�t�@��ݒ肵�Ȃ��i�X�J�C�{�b�N�X�͒P���Ȏl�p�`�j
    immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    // �v���~�e�B�u�g�|���W�[���g���C�A���O���X�g���b�v�ɐݒ�
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // ���̓��C�A�E�g��ݒ肵�Ȃ��i���̃V�F�[�_�[�ł͕s�v�j
    immediateContext->IASetInputLayout(NULL);
    // ���_�V�F�[�_�[���Z�b�g
    immediateContext->VSSetShader(skyMapVs.Get(), 0, 0);
    // �s�N�Z���V�F�[�_�[���Z�b�g�i�L���[�u�}�b�v���ǂ����Ő؂�ւ��j
    immediateContext->PSSetShader(isTexturecube ? skyBoxPs.Get() : skyMapPs.Get(), 0, 0);
    // �X�J�C�}�b�v�̃e�N�X�`�����V�F�[�_�[�ɃZ�b�g
    immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
    // �V�F�[�_�[�ɓn���s��f�[�^���쐬
    Constants data;
    //�J������ ViewProjection �s��̋t�s������߂āA�X�J�C�}�b�v�̃V�F�[�_�[�p�ɕۑ����鏈�� 


    DirectX::XMStoreFloat4x4(&data.inverseViewProjection, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&viewProjection)));

    // �萔�o�b�t�@���X�V�i�V�F�[�_�[�ɍs���n���j
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
    immediateContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
    // �l�p�`��`��i4 ���_�j
    immediateContext->Draw(4, 0);
    // �g�p�����V�F�[�_�[�������i��̕`��ɉe����^���Ȃ��悤�ɂ���j
    immediateContext->VSSetShader(NULL, 0, 0);
    immediateContext->PSSetShader(NULL, 0, 0);
}
