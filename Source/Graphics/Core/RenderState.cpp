#include "RenderState.h"

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/Graphics.h"


void RenderState::Initialize()
{
    HRESULT hr{ S_OK };
    ID3D11Device* device = Graphics::GetDevice();

    //�T���v���[�X�e�[�g�I�u�W�F�N�g�𐶐��i�e�N�X�`���̎�舵�����@�j
    // �摜�̃T���v�����O�i�e�N�X�`���̃s�N�Z�����擾���邽�߁j���s�����߂̐ݒ���쐬���܂��B
    D3D11_SAMPLER_DESC samplerDesc;

    // �|�C���g�T���v���[�i�s�N�Z�����Ƃɂ��̂܂܂̒l���擾�j
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;// U���W�iX�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;// V���W�iY�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;// W���W�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.MipLODBias = 0;// �~�b�v�}�b�v�̃o�C�A�X�ݒ�
    samplerDesc.MaxAnisotropy = 16;// �ő�ٕ����T���v�����O���i�掿�����コ����j
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;// ��r�֐�
    samplerDesc.BorderColor[0] = 0;// ���E�F�i�����j
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // ���`�t�B���^�[�T���v���[�i���炩�ȉ摜�̕ϊ��j
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;// U���W�iX�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;// V���W�iY�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;// W���W�̃��b�s���O�i�J��Ԃ��j
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // �ٕ����t�B���^�[�T���v���[�i���掿�ȃe�N�X�`����\���j
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;// U���W�iX�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;// V���W�iY�������j�̃��b�s���O�i�J��Ԃ��j
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;// W���W�̃��b�s���O�i�J��Ԃ��j
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // ���`�t�B���^�[�T���v���[�i���炩�ȉ摜�̕ϊ��j���E���@���F
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // ���`�t�B���^�[�T���v���[�i���炩�ȉ摜�̕ϊ��j���E���@���F
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 1;
    samplerDesc.BorderColor[1] = 1;
    samplerDesc.BorderColor[2] = 1;
    samplerDesc.BorderColor[3] = 1;
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_CLAMP)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // VOLUMETRIC_CLOUDSCAPES
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_MIRROR)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // SHADOW
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.MipLODBias = 0;// CascadeShadowMaps
    samplerDesc.MaxAnisotropy = 16;// CascadeShadowMaps
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; //D3D11_COMPARISON_LESS_EQUAL
    samplerDesc.BorderColor[0] = 1;
    samplerDesc.BorderColor[1] = 1;
    samplerDesc.BorderColor[2] = 1;
    samplerDesc.BorderColor[3] = 1;
    samplerDesc.MinLOD = 0;// CascadeShadowMaps
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;// CascadeShadowMaps
    hr = device->CreateSamplerState(&samplerDesc, samplerStates[static_cast<size_t>(SAMPLER_STATE::COMPARISON_LINEAR_BORDER_WHITE)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));



    // �[�x�e�X�g��X�e���V���o�b�t�@�̐ݒ���s���i��ʂ̉��s���������j
    // �[�x�e�X�gON�A�[�x��������ON
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;// �[�x�e�X�g��L���ɂ���
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // �[�x����������
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // �[�x��r���@�i�߂����̂���`��j
        hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    // �[�x�e�X�gON�A�[�x��������OFF
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;// �[�x�e�X�g��L���ɂ���
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;// �[�x�������݂𖳌��ɂ���
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_OFF)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    // �[�x�e�X�gOFF�A�[�x��������ON
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = FALSE;// �[�x�e�X�g�𖳌��ɂ���
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // �[�x�������݂�L���ɂ���
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_ON)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    // �[�x�e�X�gOFF�A�[�x��������OFF
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = FALSE; // �[�x�e�X�g�𖳌��ɂ���
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;// �[�x�������݂𖳌��ɂ���
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    // �u�����f�B���O�X�e�[�g���쐬���鏈��
    D3D11_BLEND_DESC blendDesc{}; // �V�����u�����f�B���O�̐ݒ���s�����߂̍\����
    // �����ȃu�����h�ݒ�i�u�����h�Ȃ��j
    {
        blendDesc.AlphaToCoverageEnable = FALSE; // AlphaToCoverage�𖳌��ɂ���
        blendDesc.IndependentBlendEnable = FALSE; // �����̃^�[�Q�b�g���g��Ȃ�
        blendDesc.RenderTarget[0].BlendEnable = FALSE; // �u�����h�𖳌��ɂ���
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // �u�����h���Z�͉��Z
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // �A���t�@�̃\�[�X�͏��1
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // �A���t�@�̈����0
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // �A���t�@���Z�͉��Z
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // �F�S�̂���������
        hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::NONE)].GetAddressOf()); // �u�����h�ݒ�̍쐬
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�ɐ����������m�F
    }

    // �A���t�@�u�����h�ݒ� (���ߏ���)
    {
        blendDesc.AlphaToCoverageEnable = FALSE; // AlphaToCoverage�𖳌��ɂ���i�����x�Ń}�X�N�������Ȃ��j
        blendDesc.IndependentBlendEnable = FALSE; // �����̃����_�^�[�^�[�Q�b�g���g��Ȃ�
        blendDesc.RenderTarget[0].BlendEnable = TRUE; // ���߂�L���ɂ���
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // �\�[�X�̃A���t�@�l���g���ău�����h
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ����̃A���t�@�l�𔽓]�����ău�����h
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // �u�����h���Z�F���Z
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // �A���t�@�̃\�[�X�u�����h�͏��1
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; // ����̃A���t�@�l�𔽓]�����ău�����h
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // �A���t�@�p�̉��Z�͉��Z
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // �F�̂��ׂẴ`���l������������
        hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::ALPHA)].GetAddressOf()); // ��L�ݒ���g���ău�����f�B���O�X�e�[�g���쐬
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�ɐ����������`�F�b�N    }
    }
    // ���Z�u�����h�ݒ�
    {
        blendDesc.AlphaToCoverageEnable = FALSE; // AlphaToCoverage�𖳌��ɂ���
        blendDesc.IndependentBlendEnable = FALSE; // �����^�[�Q�b�g���g��Ȃ�
        blendDesc.RenderTarget[0].BlendEnable = TRUE; // �u�����h��L���ɂ���
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // ����̐F���\�[�X�Ƃ��Ďg�p
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE; // ����u�����h�̓[���i�F�����Z����j
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // �u�����h���Z�͉��Z
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO; // �A���t�@�̃\�[�X�͏��1
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE; // �A���t�@�̈���̓[��
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // �A���t�@���Z�͉��Z
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // �F�S�̂���������
        hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::ADD)].GetAddressOf()); // ���Z�u�����h�ݒ���쐬
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�ɐ����������`�F�b�N
    }

    {
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO; //D3D11_BLEND_DEST_COLOR
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR; //D3D11_BLEND_SRC_COLOR
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::MULTIPLY)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    // MULTIPLE_RENDER_TARGETS
    // �A���t�@�u�����h�ݒ� (���ߏ���)
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = TRUE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[1].BlendEnable = FALSE;
    blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[2].BlendEnable = FALSE;
    blendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[3].BlendEnable = FALSE;
    blendDesc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[4].BlendEnable = FALSE;
    blendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[5].BlendEnable = FALSE;
    blendDesc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[6].BlendEnable = FALSE;
    blendDesc.RenderTarget[6].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[7].BlendEnable = FALSE;
    blendDesc.RenderTarget[7].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // MULTIPLE_RENDER_TARGETS
    // �����ȃu�����h�ݒ�i�u�����h�Ȃ��j
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = TRUE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE; // �u�����h�𖳌��ɂ���
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // �u�����h���Z�͉��Z
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // �A���t�@�̃\�[�X�͏��1
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // �A���t�@�̈����0
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // �A���t�@���Z�͉��Z
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // �F�S�̂���������
    blendDesc.RenderTarget[1].BlendEnable = FALSE;
    blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[2].BlendEnable = FALSE;
    blendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[3].BlendEnable = FALSE;
    blendDesc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[4].BlendEnable = FALSE;
    blendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[5].BlendEnable = FALSE;
    blendDesc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[6].BlendEnable = FALSE;
    blendDesc.RenderTarget[6].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[7].BlendEnable = FALSE;
    blendDesc.RenderTarget[7].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateBlendState(&blendDesc, blendStates[static_cast<size_t>(BLEND_STATE::MULTIPLY_RENDER_TARGET_NONE)].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // ���X�^���C�U�[�X�e�[�g�̍쐬
    {
        D3D11_RASTERIZER_DESC rasterizerDesc{}; // ���X�^���C�U�̐ݒ�
        rasterizerDesc.FillMode = D3D11_FILL_SOLID; // �h��Ԃ����[�h
        rasterizerDesc.CullMode = D3D11_CULL_BACK; // �w�ʃJ�����O�i���ʂ�`�悵�Ȃ��j
        rasterizerDesc.FrontCounterClockwise = TRUE; // �������͎��v���
        rasterizerDesc.DepthBias = 0; // �[�x�o�C�A�X�i����͂Ȃ��j
        rasterizerDesc.DepthBiasClamp = 0; // �[�x�o�C�A�X�̐���
        rasterizerDesc.SlopeScaledDepthBias = 0; // �X�΃X�P�[���̐[�x�o�C�A�X
        rasterizerDesc.DepthClipEnable = TRUE; // �[�x�N���b�v��L���ɂ���
        rasterizerDesc.ScissorEnable = FALSE; // �X�N���[���̐؂蔲���𖳌��ɂ���
        rasterizerDesc.MultisampleEnable = FALSE; // �}���`�T���v���A���`�G�C���A�X�𖳌��ɂ���
        rasterizerDesc.AntialiasedLineEnable = FALSE; // ���C���A���`�G�C���A�X�𖳌��ɂ���
        hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState[static_cast<size_t>(RASTERRIZER_STATE::SOLID_CULL_BACK)].GetAddressOf()); // ���X�^���C�U�[�X�e�[�g�쐬
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�������m�F

        // ���C���[�t���[���`��̃��X�^���C�U�[�X�e�[�g�ݒ�
        rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME; // ���C���[�t���[���`��ɕύX
        rasterizerDesc.CullMode = D3D11_CULL_BACK; // �w�ʃJ�����O��L���ɂ���
        rasterizerDesc.AntialiasedLineEnable = TRUE; // �A���`�G�C���A�X����L���ɂ���
        hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState[static_cast<size_t>(RASTERRIZER_STATE::WIREFRAME_CULL_BACK)].GetAddressOf()); // ���C���[�t���[���ݒ�̃��X�^���C�U�[�X�e�[�g���쐬
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�������m�F

        //pdf21�ō�������X�^���C�U�X�e�[�g
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        rasterizerDesc.AntialiasedLineEnable = TRUE; // �A���`�G�C���A�X����L���ɂ���
        hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState[static_cast<size_t>(RASTERRIZER_STATE::SOLID_CULL_NONE)].GetAddressOf()); // �J�����O�Ȃ��̃��C���[�t���[���ݒ�
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�������m�F

        // ���C���[�t���[���`��i�J�����O�Ȃ��j�̃��X�^���C�U�[�X�e�[�g�ݒ�
        rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME; // ���C���[�t���[���`��ɕύX
        rasterizerDesc.CullMode = D3D11_CULL_NONE; // �J�����O�Ȃ�
        rasterizerDesc.AntialiasedLineEnable = TRUE; // �A���`�G�C���A�X����L���ɂ���
        hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState[static_cast<size_t>(RASTERRIZER_STATE::WIREFRAME_CULL_NONE)].GetAddressOf()); // �J�����O�Ȃ��̃��C���[�t���[���ݒ�
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr)); // �쐬�������m�F

        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.ScissorEnable = TRUE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState[static_cast<int>(RASTERRIZER_STATE::USE_SCISSOR_RECTS)].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

}

// �T���v���X�e�[�g�ݒ�
void RenderState::SetSamplerState(ID3D11DeviceContext* immediateContext)
{
    //�T���v���[�X�e�[�g�I�u�W�F�N�g���o�C���h����
    // �s�N�Z���V�F�[�_�[�Ŏg�p����T���v���[�i�e�N�X�`���̎擾���@�j��ݒ�
    immediateContext->PSSetSamplers(0, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
    immediateContext->PSSetSamplers(1, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
    immediateContext->PSSetSamplers(2, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
    immediateContext->PSSetSamplers(3, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
    immediateContext->PSSetSamplers(4, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());
    immediateContext->PSSetSamplers(5, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::COMPARISON_LINEAR_BORDER_WHITE)].GetAddressOf());

    // �R���s���[�g�V�F�[�_�[
    immediateContext->CSSetSamplers(0, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
    immediateContext->CSSetSamplers(1, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
    immediateContext->CSSetSamplers(2, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
    immediateContext->CSSetSamplers(3, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
    immediateContext->CSSetSamplers(4, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());
    immediateContext->CSSetSamplers(5, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::COMPARISON_LINEAR_BORDER_WHITE)].GetAddressOf());

    // �W�I���g���b�N�V�F�[�_�[
    immediateContext->GSSetSamplers(0, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
    immediateContext->GSSetSamplers(1, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
    immediateContext->GSSetSamplers(2, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
    immediateContext->GSSetSamplers(3, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
    immediateContext->GSSetSamplers(4, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());
    immediateContext->GSSetSamplers(5, 1, samplerStates[static_cast<size_t>(SAMPLER_STATE::COMPARISON_LINEAR_BORDER_WHITE)].GetAddressOf());

}