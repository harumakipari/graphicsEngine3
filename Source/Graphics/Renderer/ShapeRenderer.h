#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Graphics/Resource/GltfModel.h"

class ShapeRenderer
{
private:
    ShapeRenderer() = default;
    //ShapeRenderer(ID3D11Device* device);
    virtual ~ShapeRenderer() = default;

public:
    static ShapeRenderer& Instance()
    {
        static ShapeRenderer instance;
        return instance;
    }

    void Initalize(ID3D11Device* device);

    enum class Type
    {
        Line,
        Segment,
        Point
    };

    // 箱描画
    void DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position,const DirectX::XMFLOAT3& angle,const DirectX::XMFLOAT3& size,const DirectX::XMFLOAT4& color);

    //void DrawBox(
    //    const DirectX::XMFLOAT4X4& transform,
    //    const DirectX::XMFLOAT3& size,
    //    const DirectX::XMFLOAT4& color) {
    //}

    void DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color);

    // 球描画
    void DrawSphere(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, const DirectX::XMFLOAT4& color);

    // カプセル描画
    void DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);

    void DrawCapsule(ID3D11DeviceContext* immediateContext,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT4& rotation, // ← クォータニオン追加
        float radius, float height,
        const DirectX::XMFLOAT4& color);

    void DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition, float radius, const DirectX::XMFLOAT4& color);

    void DrawCapsule(ID3D11DeviceContext* immediateContext,const DirectX::XMFLOAT4X4& worldTransform,float radius,float height,const DirectX::XMFLOAT4& color);

    //線描画
    void DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type);

    //線描画 数珠つなぎ
    void DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition);
private:
    struct DebugConstants
    {
        DirectX::XMFLOAT4 color;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer[2];

    const size_t maxPoints = 500;

    std::unique_ptr<GltfModel> sphere = nullptr;
    std::unique_ptr<GltfModel> topHalfSphere = nullptr;
    std::unique_ptr<GltfModel> bottomHalfSphere = nullptr;
    std::unique_ptr<GltfModel> cylinder = nullptr;
    std::unique_ptr<GltfModel> capsule = nullptr;
    std::unique_ptr<GltfModel> cube = nullptr;
};


//デバック直線、線分、点を描画する
class LineSegment
{
public:
    enum class Type
    {
        Line,
        Segment,
        Point
    };

    LineSegment(ID3D11Device* device, size_t maxSegments);

    virtual ~LineSegment() = default;

    void Draw(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& viewProjection, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type);
private:
    struct Constants
    {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT4 color;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    const size_t max_points;

};