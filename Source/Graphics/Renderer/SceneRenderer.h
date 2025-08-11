#pragma once
#include<d3d11.h>
#include <vector>
#include <memory>

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/Core/PipleLineLibrary.h"
#include "Engine/Camera/CameraConstants.h"


class SceneRenderer
{
public:

    SceneRenderer()
    {
        // 定数バッファ
        viewBuffer = std::make_unique<ConstantBuffer<ViewConstants>>(Graphics::GetDevice());
        primitiveJointCBuffer = std::make_unique<ConstantBuffer<PrimitiveJointConstants>>(Graphics::GetDevice());
        primitiveCBuffer = std::make_unique<ConstantBuffer<PrimitiveConstants>>(Graphics::GetDevice());

        // パイプラインステート
        pipeLineStateSet = std::make_unique<PipeLineStateSet>();
        pipeLineStateSet->InitStaticMesh(Graphics::GetDevice());
        pipeLineStateSet->InitSkeltalMesh(Graphics::GetDevice());
    }

    virtual ~SceneRenderer() {}

    // View関連の定数バッファを更新する
    void UpdateViewConstants(ID3D11DeviceContext* immediateContext, const ViewConstants& data)
    {
        viewBuffer->data = data;
        viewBuffer->Activate(immediateContext, 8);
    }

    void RenderOpaque(ID3D11DeviceContext* immediateContext/*, std::vector<std::shared_ptr<Actor>> allActors*/);

    void RenderMask(ID3D11DeviceContext* immediateContext);

    void RenderBlend(ID3D11DeviceContext* immediateContext);

    void CastShadowRender(ID3D11DeviceContext* immediateContext, std::vector<std::shared_ptr<Actor>> allActors)
    {
        for (auto actor : allActors)
        {
            if (!actor->rootComponent_)
            {
                continue;
            }

            if (!actor->isActive)
            {// actorが存在していなかったらスキップ
                continue;
            }

            // actor に付属している全ての meshComponent を取り出す
            std::vector<MeshComponent*> meshComponents;
            actor->GetComponents<MeshComponent>(meshComponents);

            for (const MeshComponent* meshComponent : meshComponents)
            {
                if (!meshComponent->IsVisible())
                { // 描画フラグが false ならスキップ
                    continue;
                }
                const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
                meshComponent->CastShadow(immediateContext, worldMat);
            }
        }
    }

    void Draw(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass);

    void DrawWithStaticBatching(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass);
private:
    // カメラの定数バッファ
    std::unique_ptr<ConstantBuffer<ViewConstants>> viewBuffer;

    // パイプラインステート
    std::unique_ptr<PipeLineStateSet> pipeLineStateSet;

    // 
    static const int PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    std::unique_ptr<ConstantBuffer<PrimitiveJointConstants>> primitiveJointCBuffer;

    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;

        DirectX::XMFLOAT4 color;

        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        float disolveFactor = 0.0f;

        float emission = 0.0f;
        float pads[3];
    };
    std::unique_ptr<ConstantBuffer<PrimitiveConstants>> primitiveCBuffer;

public:
    // 今のRenderPath
    RenderPath currentRenderPath = RenderPath::Defferd;
};

