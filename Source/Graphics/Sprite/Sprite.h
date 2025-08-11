#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

class Sprite
{
public:
    Sprite(ID3D11Device* device,const wchar_t* filename);
    ~Sprite();

    void Render(ID3D11DeviceContext* immediate_context,
        float dx, float dy,//��`�̍���̍��W�i�X�N���[�����W�n)
        float dw, float dh,//��`�̃T�C�Y�i�X�N���[�����W�n�j
        float r, float g, float b, float a,//�F
        float angle/*degree*/);
   
    void Render(ID3D11DeviceContext* immediate_context,
        float dx, float dy,//��`�̍���̍��W�i�X�N���[�����W�n)
        float dw, float dh,//��`�̃T�C�Y�i�X�N���[�����W�n�j
        float r, float g, float b, float a,//�F
        float angle/*degree*/,
        float sx, float sy, float sw, float sh);//������W�ƃT�C�Y

    void Render(ID3D11DeviceContext* immediate_context,
        float dx, float dy,//��`�̍���̍��W�i�X�N���[�����W�n)
        float dw, float dh);//��`�̃T�C�Y�i�X�N���[�����W�n�j

    //�t�H���g�摜�t�@�C�����g�p���ĔC�ӂ̕��������ʂɏo�͂���
    void Textout(ID3D11DeviceContext* immediateContext, std::string s,
        float x, float y, float w, float h, float r, float g, float b, float a);
private:
    //���_�t�H�[�}�b�g
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        //�e�N�X�`�����W�ϐ�
        DirectX::XMFLOAT2 texCoord;
    };

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;//���_�o�b�t�@
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    D3D11_TEXTURE2D_DESC texture2dDesc;
};

