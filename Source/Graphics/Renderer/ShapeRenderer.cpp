#include "ShapeRenderer.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Win32Utils.h"

//ShapeRenderer::ShapeRenderer(ID3D11Device* device)
//{
//    HRESULT hr{ S_OK };
//
//    D3D11_BUFFER_DESC bufferDesc{};
//    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * maxPoints);
//    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
//    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//    bufferDesc.MiscFlags = 0;
//    bufferDesc.StructureByteStride = 0;
//    hr = device->CreateBuffer(&bufferDesc, NULL, vertexBuffer.ReleaseAndGetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//
//    bufferDesc.ByteWidth = sizeof(DebugConstants);
//    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//    bufferDesc.CPUAccessFlags = 0;
//    bufferDesc.MiscFlags = 0;
//    bufferDesc.StructureByteStride = 0;
//    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer[1].GetAddressOf());
//
//    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
//    {
//        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
//    };
//    CreateVsFromCSO(device, "./Shader/LineSegmentVS.cso", vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
//    CreatePsFromCSO(device, "./Shader/LineSegmentPS.cso", pixelShader.GetAddressOf());
//
//    sphere = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\sphere.glb");
//    capsule = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\capsule.glb");
//    topHalfSphere = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\topHalfSphere.glb");
//    bottomHalfSphere = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\bottomHalfSphere.glb");
//    cylinder = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\cylinder.glb");
//    cube = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\cube.glb");
//
//    std::vector<GltfModelBase::Material>& sphereMaterials = sphere->materials;
//    std::vector<GltfModelBase::Material>& capsuleMaterials = capsule->materials;
//    std::vector<GltfModelBase::Material>& topHalfSphereMaterials = topHalfSphere->materials;
//    std::vector<GltfModelBase::Material>& bottomHalfSphereMaterials = bottomHalfSphere->materials;
//    std::vector<GltfModelBase::Material>& cylinderMaterials = cylinder->materials;
//    std::vector<GltfModelBase::Material>& cubeMaterials = cube->materials;
//
//    //デバック用のPSに変更する
//    for (GltfModelBase::Material& material : sphereMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//    for (GltfModelBase::Material& material : capsuleMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//    for (GltfModelBase::Material& material : topHalfSphereMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//    for (GltfModelBase::Material& material : bottomHalfSphereMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//    for (GltfModelBase::Material& material : cylinderMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//    for (GltfModelBase::Material& material : cubeMaterials)
//    {//色だけを返すPS
//        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
//    }
//
//    //capsule = std::make_unique<GltfModel>(device, ".\\resources\\geometricPrimitive\\capsule01.glb");   //原点が一番下
//}

void ShapeRenderer::Initalize(ID3D11Device* device)
{
    HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * maxPoints);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, NULL, vertexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    bufferDesc.ByteWidth = sizeof(DebugConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer[1].GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    CreateVsFromCSO(device, "./Shader/LineSegmentVS.cso", vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
    CreatePsFromCSO(device, "./Shader/LineSegmentPS.cso", pixelShader.GetAddressOf());

    sphere = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/sphere.glb");
    capsule = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/capsule.glb");
    topHalfSphere = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/topHalfSphere.glb");
    bottomHalfSphere = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/bottomHalfSphere.glb");
    cylinder = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/cylinder.glb");
    cube = std::make_unique<GltfModel>(device, "./Data/Debug/Primitives/cube.glb");

    std::vector<GltfModelBase::Material>& sphereMaterials = sphere->materials;
    std::vector<GltfModelBase::Material>& capsuleMaterials = capsule->materials;
    std::vector<GltfModelBase::Material>& topHalfSphereMaterials = topHalfSphere->materials;
    std::vector<GltfModelBase::Material>& bottomHalfSphereMaterials = bottomHalfSphere->materials;
    std::vector<GltfModelBase::Material>& cylinderMaterials = cylinder->materials;
    std::vector<GltfModelBase::Material>& cubeMaterials = cube->materials;

    //デバック用のPSに変更する
    for (GltfModelBase::Material& material : sphereMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }
    for (GltfModelBase::Material& material : capsuleMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }
    for (GltfModelBase::Material& material : topHalfSphereMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }
    for (GltfModelBase::Material& material : bottomHalfSphereMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }
    for (GltfModelBase::Material& material : cylinderMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }
    for (GltfModelBase::Material& material : cubeMaterials)
    {//色だけを返すPS
        CreatePsFromCSO(device, "./Shader/GltfModelBaseColorPS.cso", material.replacedPixelShader.GetAddressOf());
    }


}

// 球描画
void ShapeRenderer::DrawSphere(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
    };
#if 1
    const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
    const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif
    DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
    //DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,(radius * 2 + height) * 0.25f,radius) };//半径 1 だから scale.x は 1
    DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,radius,radius) };
    DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0,0,0) };
    DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y,position.z) };
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, C * S * R * T);

    sphere->Render(immediateContext, world, RenderPass::Opaque);
}

// カプセル描画
void ShapeRenderer::DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
    };
#if 1
    const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
    const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif
    {//cylinder
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,(height - radius) ,radius) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0,0,0) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y + radius,position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);
        //TODO:03 debugShapeはRenderPassをOpaqueにしている
        cylinder->Render(immediateContext, world, RenderPass::Opaque);
    }
    {//topHalfSphere
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,radius ,radius) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0,0,0) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y + height,position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);

        topHalfSphere->Render(immediateContext, world, RenderPass::Opaque);
    }
    {//bottomHalfSphere
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,radius ,radius) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0,0,0) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y + radius,position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);

        bottomHalfSphere->Render(immediateContext, world, RenderPass::Opaque);
    }
}

void ShapeRenderer::DrawCapsule(ID3D11DeviceContext* immediateContext,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT4& rotation, // ← クォータニオン追加
    float radius, float height,
    const DirectX::XMFLOAT4& color)
{
  DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    const float cylinderHeight = height - 2.0f * radius;  // 両端半球分を除く
    const float halfCylinderHeight = cylinderHeight * 0.5f;

    // 基本変換
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
    DirectX::XMMATRIX rotateZ90 = DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2); // X軸→Y軸
    DirectX::XMMATRIX finalRotation = rotateZ90 * R;

    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

    // カプセルの円柱部分
    {
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, cylinderHeight * 0.5f); // Z方向が長軸
        DirectX::XMMATRIX world = S * finalRotation * T;
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, world);
        cylinder->Render(immediateContext, m, RenderPass::Opaque);
    }

    // 上側の半球
    {
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, radius);
        DirectX::XMMATRIX offset = DirectX::XMMatrixTranslation(0, 0, halfCylinderHeight);
        DirectX::XMMATRIX world = S * offset * finalRotation * T;
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, world);
        topHalfSphere->Render(immediateContext, m, RenderPass::Opaque);
    }

    // 下側の半球
    {
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, radius);
        DirectX::XMMATRIX offset = DirectX::XMMatrixTranslation(0, 0, -halfCylinderHeight);
        DirectX::XMMATRIX world = S * offset * finalRotation * T;
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, world);
        bottomHalfSphere->Render(immediateContext, m, RenderPass::Opaque);
    }
}

void ShapeRenderer::DrawCapsule(
    ID3D11DeviceContext* immediateContext,
    const DirectX::XMFLOAT4X4& worldTransform,
    float radius,
    float height,
    const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
    };
#if 1
    const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
    const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif

    // ベースの変換行列（外部から与えられたワールド行列 × 座標系変換 × スケール変換）
    DirectX::XMMATRIX baseTransform =
        DirectX::XMLoadFloat4x4(&worldTransform) *
        DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) *
        DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor);

    {// Cylinder
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, (height - radius), radius);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(0, radius, 0); // 中央配置
        DirectX::XMMATRIX world = S * T;
        DirectX::XMFLOAT4X4 finalMatrix;
        DirectX::XMStoreFloat4x4(&finalMatrix, world * baseTransform);
        cylinder->Render(immediateContext, finalMatrix, RenderPass::Opaque);
    }

    {// Top Half Sphere
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, radius);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(0, height, 0);
        DirectX::XMMATRIX world = S * T;
        DirectX::XMFLOAT4X4 finalMatrix;
        DirectX::XMStoreFloat4x4(&finalMatrix, world * baseTransform);
        topHalfSphere->Render(immediateContext, finalMatrix, RenderPass::Opaque);
    }

    {// Bottom Half Sphere
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, radius);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(0, radius, 0);
        DirectX::XMMATRIX world = S * T;
        DirectX::XMFLOAT4X4 finalMatrix;
        DirectX::XMStoreFloat4x4(&finalMatrix, world * baseTransform);
        bottomHalfSphere->Render(immediateContext, finalMatrix, RenderPass::Opaque);
    }
}


void ShapeRenderer::DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition, float radius, const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    using namespace DirectX;

    XMVECTOR Start = XMLoadFloat3(&startPosition);
    XMVECTOR End = XMLoadFloat3(&endPosition);
    XMVECTOR Center = 0.5f * (Start + End);     //カプセルの中心
    XMVECTOR Up = XMVector3Normalize(End - Start);  //カプセルの向き
    float height = XMVectorGetX(XMVector3Length(End - Start));

    const XMFLOAT4X4 coordinateSystemTransform =
    {
        -1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };

    XMMATRIX C = XMLoadFloat4x4(&coordinateSystemTransform);

    // Z軸を up に合わせるための回転行列
    XMVECTOR defaultUp = XMVectorSet(0, 1, 0, 0);
    XMVECTOR axis = XMVector3Cross(defaultUp, Up);


    float angle = acosf(XMVectorGetX(XMVector3Dot(defaultUp, Up)));
    XMMATRIX R = (XMVector3Equal(axis, XMVectorZero())) ? XMMatrixIdentity() : XMMatrixRotationAxis(axis, angle);



    // スケール行列
    XMMATRIX scaleCylinder = XMMatrixScaling(radius, height * 0.5f, radius);
    XMMATRIX scaleSphere = XMMatrixScaling(radius, radius, radius);

    // 平行移動行列
    XMMATRIX T_center = XMMatrixTranslationFromVector(Center);
    XMMATRIX T_top = XMMatrixTranslationFromVector(End);
    XMMATRIX T_bottom = XMMatrixTranslationFromVector(Start);

    XMFLOAT4X4 world;

    // Cylinder（中心部分）
    XMStoreFloat4x4(&world, C * scaleCylinder * R * T_center);
    cylinder->Render(immediateContext, world, RenderPass::Opaque);

    // 上の半球
    XMStoreFloat4x4(&world, C * scaleSphere * R * T_top);
    topHalfSphere->Render(immediateContext, world, RenderPass::Opaque);

    // 下の半球
    XMStoreFloat4x4(&world, C * scaleSphere * R * T_bottom);
    bottomHalfSphere->Render(immediateContext, world, RenderPass::Opaque);
}
// 箱描画
void ShapeRenderer::DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& angle, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
    };
#if 1
    const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
    const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif
    {//cube
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(size.x,size.y,size.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y,position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);
        //TODO:03 debugShapeはRenderPassをOpaqueにしている
        cube->Render(immediateContext, world, RenderPass::Opaque);
    }
}

void ShapeRenderer::DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color)
{
    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);
    Transform.r[0] = DirectX::XMVectorScale(Transform.r[0], size.x);
    Transform.r[1] = DirectX::XMVectorScale(Transform.r[1], size.y);
    Transform.r[2] = DirectX::XMVectorScale(Transform.r[2], size.z);
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, Transform);
    cube->Render(immediateContext, world, RenderPass::Opaque);

}

//線描画
void ShapeRenderer::DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type)
{
    _ASSERT_EXPR(points.size() <= maxPoints, L"Points are sizeover!!");

    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
    hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    std::memcpy(mappedSubresource.pData, points.data(), points.size() * sizeof(DirectX::XMFLOAT3));
    immediateContext->Unmap(vertexBuffer.Get(), 0);

    UINT stride{ sizeof(DirectX::XMFLOAT3) };
    UINT offset{ 0 };
    immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

    if (type == Type::Line)
    {
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    }
    else if (type == Type::Segment)
    {
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    }
    else// Type::Point
    {
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    }

    immediateContext->VSSetShader(vertexShader.Get(), NULL, 0);
    immediateContext->PSSetShader(pixelShader.Get(), NULL, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());

    DebugConstants data1{ color };
    immediateContext->UpdateSubresource(constantBuffer[1].Get(), 0, 0, &data1, 0, 0);
    immediateContext->VSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());
    immediateContext->PSSetConstantBuffers(12, 1, constantBuffer[1].GetAddressOf());

    immediateContext->Draw(static_cast<UINT>(points.size()), 0);
}

//線描画
void ShapeRenderer::DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition)
{
    using namespace DirectX;
    const float radius = 0.05f;
    // directionVector = endPosition - startPosition 
    DirectX::XMFLOAT3 vector = { endPosition.x - startPosition.x,endPosition.y - startPosition.y,endPosition.z - startPosition.z };
    DirectX::XMVECTOR Vec = DirectX::XMLoadFloat3(&vector);
    float height = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Vec));
    Vec = DirectX::XMVector3Normalize(Vec);

    const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
    };
#if 1
    const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
    const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif


    //　高さ　÷　直径　切り上げ
    int sphereNum = static_cast<int>(std::ceil(height / (radius * 2)));

    for (int i = 0; i < sphereNum; i++)
    {
        // forwardVec 方向に radius * 2 ずつ
        DirectX::XMVECTOR Offset = Vec * (radius * 2 * i);
        //offsetを加味したposition
        DirectX::XMVECTOR NewPosition = DirectX::XMLoadFloat3(&startPosition) + Offset;

        DirectX::XMFLOAT3 center;
        DirectX::XMStoreFloat3(&center, NewPosition);

        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(radius,radius,radius) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0,0,0) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(center.x,center.y,center.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);

        sphere->Render(immediateContext, world, RenderPass::Opaque);
    }
}

LineSegment::LineSegment(ID3D11Device* device, size_t maxPoints) : max_points(maxPoints)
{
}

void LineSegment::Draw(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& view_projection, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type)
{
}