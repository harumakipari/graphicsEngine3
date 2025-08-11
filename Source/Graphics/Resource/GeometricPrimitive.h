#pragma once
#include <directXmath.h>
//COM�I�u�W�F�N�g��ComPtr�X�}�[�g�|�C���^�[�e���v���[�g���g�����ϐ��錾�ɕύX����
#include <wrl.h>

///ID3D11
#include <d3d11.h>

class GeometricPrimitive
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
    };
    struct Constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 materialColor;
    };

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
    virtual ~GeometricPrimitive() = default;

    void Render(DirectX::XMFLOAT4X4& world,DirectX::XMFLOAT4& materialColor);

protected:
    GeometricPrimitive();

    //���_���ƒ��_�̏��Ԃ�GPU�ɓ`����
    void CreateComBuffers(Vertex* vertices, size_t vertexCount,
        uint32_t* indices, size_t indexCount);
};

class GeometricCube :public GeometricPrimitive
{
public:
    GeometricCube();
};

class GeometricCylinder : public GeometricPrimitive
{
public:
    GeometricCylinder(uint32_t slices);
};

class GeometricSphere : public GeometricPrimitive
{
public:
    GeometricSphere( DirectX::XMFLOAT3 center, float radius, uint32_t slices, uint32_t stacks);
    GeometricSphere(float radius, uint32_t slices, uint32_t stacks);
};

class GeometricCapsule : public GeometricPrimitive
{
public:
    GeometricCapsule(float mantle_height, const DirectX::XMFLOAT3& radius, uint32_t slices, uint32_t ellipsoid_stacks, uint32_t mantle_stacks);
};
