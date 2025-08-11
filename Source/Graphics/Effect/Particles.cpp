#include "Particles.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Win32Utils.h"

#include "Graphics/Resource/Texture.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

#include "Graphics/Core/RenderState.h"

#include "Widgets/Utils/Dialog.h"
#include "Widgets/Utils/stdUtiles.h"

using namespace DirectX;

ParticleSystem::ParticleSystem(ID3D11Device* device, int particleCount) :maxParticleCount(particleCount)
{
    HRESULT hr{ S_OK };
    D3D11_BUFFER_DESC bufferDesc{};
    // ���q�f�[�^�p�o�b�t�@�̐ݒ�
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Particle) * particleCount);
    bufferDesc.StructureByteStride = sizeof(Particle);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    // ���q�o�b�t�@���쐬
    hr = device->CreateBuffer(&bufferDesc, NULL, particleBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    // �V�F�[�_���\�[�X�r���[ (SRV) �̐ݒ�
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.ElementOffset = 0;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
    // SRV���쐬
    hr = device->CreateShaderResourceView(particleBuffer.Get(), &shaderResourceViewDesc, particleBufferSrv.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    // �A���I�[�_�[�h�A�N�Z�X�r���[ (UAV) �̐ݒ�
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
    unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    unorderedAccessViewDesc.Buffer.FirstElement = 0;
    unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
    unorderedAccessViewDesc.Buffer.Flags = 0;
    // UAV���쐬
    hr = device->CreateUnorderedAccessView(particleBuffer.Get(), &unorderedAccessViewDesc, particleBufferUav.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    // �萔�o�b�t�@�̐ݒ�
    bufferDesc.ByteWidth = sizeof(ParticleSystemConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    // �萔�o�b�t�@���쐬
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    CreateVsFromCSO(device, "./Shader/ParticleVS.cso", particleVertexShader.ReleaseAndGetAddressOf(), NULL, NULL, 0);
    CreatePsFromCSO(device, "./Shader/ParticlePS.cso", particlePixelShader.ReleaseAndGetAddressOf());
    CreateGsFromCSO(device, "./Shader/ParticleGS.cso", particleGeometricShader.ReleaseAndGetAddressOf());
    CreateCsFromCSO(device, "./Shader/IntegrateParticleCS.cso", particleComputeShader.ReleaseAndGetAddressOf());
    CreateCsFromCSO(device, "./Shader/InitializeParticleCS.cso", particleInitializerComputeShader.ReleaseAndGetAddressOf());

}

// �w�肳�ꂽ�A���C�����g�ɍ��킹�Đ��l�𒲐�����֐�
UINT Align(UINT num, UINT alignment)
{
    return (num + (alignment - 1)) & ~(alignment - 1);
}

// �p�[�e�B�N���̕����X�V���s���֐�
void ParticleSystem::Integrate(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    // UAV (Unordered Access View) ���R���s���[�g�V�F�[�_�[�Ƀo�C���h
    immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

    // �p�[�e�B�N���V�X�e���̃f�[�^���X�V
    particleSystemData.time += deltaTime;
    particleSystemData.deltaTime = deltaTime;
    particleSystemData.maxParticleCount = maxParticleCount;

    // �R���X�^���g�o�b�t�@���X�V
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
    immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

    // �R���s���[�g�V�F�[�_�[��ݒ�
    immediateContext->CSSetShader(particleComputeShader.Get(), NULL, 0);

    // �X���b�h�O���[�v�����v�Z���A�R���s���[�g�V�F�[�_�[�����s
    const UINT threadGroupCountX = Align(static_cast<UINT>(maxParticleCount), NUMTHREAD_X) / NUMTHREAD_X;
    immediateContext->Dispatch(threadGroupCountX, 1, 1);

    // UAV �������i���̏����ɉe����^���Ȃ��悤�ɂ��邽�߁j
    ID3D11UnorderedAccessView* nullUnorderedAccessView{};
    immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, NULL);
}

// �p�[�e�B�N���̏��������s���֐�
void ParticleSystem::Initialize(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    // UAV (Unordered Access View) ���R���s���[�g�V�F�[�_�[�Ƀo�C���h
    immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

    // �p�[�e�B�N���V�X�e���̃f�[�^��������
    particleSystemData.time = 0;
    particleSystemData.deltaTime = deltaTime;
    particleSystemData.maxParticleCount = maxParticleCount;

    // �R���X�^���g�o�b�t�@���X�V
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
    immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

    // �������p�̃R���s���[�g�V�F�[�_�[��ݒ�
    immediateContext->CSSetShader(particleInitializerComputeShader.Get(), NULL, 0);

    // �X���b�h�O���[�v�����v�Z���A�R���s���[�g�V�F�[�_�[�����s
    const UINT threadGroupCountX = Align(static_cast<UINT>(maxParticleCount), NUMTHREAD_X) / NUMTHREAD_X;
    immediateContext->Dispatch(threadGroupCountX, 1, 1);

    // UAV ������
    ID3D11UnorderedAccessView* nullUnorderedAccessView{};
    immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, NULL);
}

// �p�[�e�B�N���̕`����s���֐�
void ParticleSystem::Render(ID3D11DeviceContext* immediateContext)
{
    //�u�����h�X�e�[�g�ݒ�
    if (blendMode == 0)
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    }
    else
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ADD);
    }

    // �e�V�F�[�_�[���Z�b�g
    immediateContext->VSSetShader(particleVertexShader.Get(), NULL, 0);
    immediateContext->PSSetShader(particlePixelShader.Get(), NULL, 0);
    immediateContext->GSSetShader(particleGeometricShader.Get(), NULL, 0);
    
    // �W�I���g���V�F�[�_�[�Ŏg�p����SRV�iShader Resource View�j���o�C���h
    immediateContext->GSSetShaderResources(9, 1, particleBufferSrv.GetAddressOf());

    // �R���X�^���g�o�b�t�@���X�V
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
    immediateContext->VSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
    immediateContext->PSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
    immediateContext->GSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

    // ���_���C�A�E�g��o�b�t�@�̐ݒ�i�p�[�e�B�N���V�X�e���ł͕s�v�Ȃ���NULL�ݒ�j
    immediateContext->IASetInputLayout(NULL);
    immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    immediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // �p�[�e�B�N����`��
    immediateContext->Draw(static_cast<UINT>(maxParticleCount), 0);

    // �g�p����SRV������
    ID3D11ShaderResourceView* nullShaderResourceView{};
    immediateContext->GSSetShaderResources(9, 1, &nullShaderResourceView);

    // �g�p�����V�F�[�_�[������
    immediateContext->VSSetShader(NULL, NULL, 0);
    immediateContext->PSSetShader(NULL, NULL, 0);
    immediateContext->GSSetShader(NULL, NULL, 0);
}

void ParticleSystem::DrawGUI()
{
#ifdef USE_IMGUI
    ImGui::Checkbox("loop", &particleSystemData.loop);

    const char* items[] = {
        "Alpha",
        "Add"
    };
    ImGui::Combo("blend mode", &blendMode, items, IM_ARRAYSIZE(items));

    const char* item[] = {
        "Normal",
        "ToCenter"
    };
    ImGui::Combo("type", &particleSystemData.type, item, IM_ARRAYSIZE(item));

    ImGui::Separator();

    if (ImGui::TreeNodeEx("Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::DragFloat3("spawnPos", &particleSystemData.emissionPosition.x);
        ImGui::Separator();

        ImGui::SliderFloat("offsetMin", &particleSystemData.emissionOffset.x, -10.f, 10.f, "%.4f");
        ImGui::SliderFloat("offsetMax", &particleSystemData.emissionOffset.y, -10.f, 10.f, "%.4f");
        ImGui::Separator();

        ImGui::SliderFloat("coneAngleMin", &particleSystemData.emissionConeAngle.x, +0.0f, +3.141592653f, "%.4f");
        ImGui::SliderFloat("coneAngleMax", &particleSystemData.emissionConeAngle.y, +0.0f, +3.141592653f, "%.4f");
        ImGui::Separator();

        ImGui::SliderFloat("speed Min", &particleSystemData.emissionSpeed.x, 0.f, 20.f, "%.4f");
        ImGui::SliderFloat("speed Max", &particleSystemData.emissionSpeed.y, 0.f, 20.f, "%.4f");
        ImGui::Separator();

        ImGui::SliderFloat("angulerSpeedMin", &particleSystemData.emissionAngularSpeed.x, 0.f, 10.f, "%.4f");
        ImGui::SliderFloat("angulerSpeedMax", &particleSystemData.emissionAngularSpeed.y, 0.f, 10.f, "%.4f");
        ImGui::Separator();

        ImGui::SliderFloat("spawnDelay min", &particleSystemData.spawnDelay.x, +0.0f, +10.0f, "%.4f");
        ImGui::SliderFloat("spawnDelay max", &particleSystemData.spawnDelay.y, +0.0f, +10.0f, "%.4f");

        ImGui::TreePop();
    }
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::SliderFloat("gravity", &particleSystemData.gravity, -10.0f, +10.0f, "%.4f");
        ImGui::SliderFloat("noiseScale", &particleSystemData.noiseScale, +0.0f, +1.0f, "%.4f");
        ImGui::DragFloat3("direction", &particleSystemData.direction.x, 0.1f, 0.f, 1.f, "%.4f");
        ImGui::SliderFloat("strength", &particleSystemData.strength, -10.0f, +10.0f, "%.4f");

        ImGui::TreePop();
    }
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Visual", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::ColorEdit4("startColor", &particleSystemData.emissionStartColor.x);
        ImGui::ColorEdit4("endColor", &particleSystemData.emissionEndColor.x);
        ImGui::Separator();

        ImGui::SliderFloat("startSize", &particleSystemData.emissionSize.x, +0.0f, +5.0f, "%.4f");
        ImGui::SliderFloat("endSize", &particleSystemData.emissionSize.y, +0.0f, +5.0f, "%.4f");
        ImGui::Separator();

        ImGui::SliderFloat("fadeIn duration", &particleSystemData.fadeDuration.x, +0.0f, 10.0f, "%.4f");
        ImGui::SliderFloat("fadeOut duration", &particleSystemData.fadeDuration.y, +0.0f, 10.0f, "%.4f");
        ImGui::Separator();

        if (ImGui::Button("Particle Texture")) {
            static const char* filter = "Image Files(*.png;*.dds;*.tif)\0*.png;*.dds;*.tif;\0All Files(*.*)\0*.*;\0\0)";

            char filePath[256] = { 0 };
            HWND hwnd = Graphics::GetWindowHandle();
            DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
            if (result == DialogResult::OK) {
                std::wstring wpath = StringToWstring(std::string(filePath));
                HRESULT hr = LoadTextureFromFile(Graphics::GetDevice(), wpath.c_str(), particleTexture.ReleaseAndGetAddressOf(), NULL);
                _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            }
        }
        ImGui::Image(particleTexture.Get(), ImVec2(100, 100));
        ImGui::InputInt2("grid", reinterpret_cast<int*>(&particleSystemData.spriteSheetGrid.x));

        ImGui::TreePop();
    }
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::SliderFloat("lifespan min", &particleSystemData.lifespan.x, +0.0f, +10.0f, "%.4f");
        ImGui::SliderFloat("lifespan max", &particleSystemData.lifespan.y, +0.0f, +10.0f, "%.4f");

        ImGui::TreePop();
    }
#endif // USE_IMGUI

}