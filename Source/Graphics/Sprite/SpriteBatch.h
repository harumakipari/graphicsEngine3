#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

class SpriteBatch
{
public:
    SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t maxSprite);
    ~SpriteBatch();

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

    void Begin(ID3D11DeviceContext* immediateContext);
    void End(ID3D11DeviceContext* immediateContext);

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
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* vertexBuffer;//���_�o�b�t�@
    ID3D11ShaderResourceView* shaderResourceView;
    D3D11_TEXTURE2D_DESC texture2dDesc;

    const size_t maxVertices;
    std::vector<Vertex> vertices;
};

