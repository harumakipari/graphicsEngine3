#ifndef STATIC_MESH_COLLISION_COMPONENT_H
#define STATIC_MESH_COLLISION_COMPONENT_H

#include <string>
#include <vector>
#include <stack>
#include <functional>

#include <DirectXMath.h>
#include <PxPhysicsAPI.h>
#include <d3d11.h>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <functional>
#include <fbxsdk.h>

#include "Physics/Physics.h"
#include "Components/Base/SceneComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"



//FBXの行列をDirectXのXMFLOAT4x4に変換する関数
inline DirectX::XMFLOAT4X4& ToXmfloat4x4(const FbxAMatrix& fbxamatrix)
{
    DirectX::XMFLOAT4X4 xmfloat4x4;
    // 4x4行列の要素を変換してコピー
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
        }
    }
    return xmfloat4x4;
}

// glTFの画像読み込み関数。画像の読み込みをスキップする設定
static bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
    return true;
}

class StaticMeshCollisionComponent :public SceneComponent
{
public:
    struct Mesh
    {
        std::string name;
        DirectX::XMFLOAT4X4 globalTransform;
        struct Subset
        {
            std::string materialName;
            std::vector<DirectX::XMFLOAT3> positions;
            std::vector<uint32_t> indices;
        };
        std::vector<Subset> subsets;
        DirectX::XMFLOAT3 boundingBox[2]
        {
            { +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX },
            { -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX }
        };
    };
    std::vector<Mesh> meshes;

    StaticMeshCollisionComponent(const std::string& name,std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}
    virtual ~StaticMeshCollisionComponent(){}

    void OnUnregister()override
    {
        for (physx::PxTriangleMesh* pxTriangleMesh : pxTriangleMeshes_)
        {
            if (!pxTriangleMesh)
            {
                pxTriangleMesh->release();
            }
        }
        physx::PxPhysics* pxPhysics = Physics::Instance().GetPhysics();
        physx::PxScene* pxScene = Physics::Instance().GetScene();
        if (pxActors_.size() > 0)
        {
            if (pxScene)
            {
                pxScene->removeActors(pxActors_.data(), static_cast<physx::PxU32>(pxActors_.size()));
            }
        }
    }

    void SetModel(const std::string& filename)
    {
        // glTFファイルを読み込む処理
        tinygltf::TinyGLTF tinyGltf;
        tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

        tinygltf::Model gltfModel;
        std::string error, warning;
        bool succeeded{ false };
        // glTF形式のファイルを読み込む
        if (filename.find(".glb") != std::string::npos)
        {
            succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
        }
        else if (filename.find(".gltf") != std::string::npos)
        {
            succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
        }
        assert(succeeded && "Failed to load glTF file");

        // glTFノードを再帰的に処理
        std::stack<DirectX::XMFLOAT4X4> parentGlobalTransforms;
        std::function<void(int)> traverse = [&](int nodeIndex)
            {
                // ノード情報の取得
                decltype(gltfModel.nodes)::value_type gltfNode = gltfModel.nodes.at(nodeIndex);
                // グローバルトランスフォームの計算
                DirectX::XMFLOAT4X4 globalTransform =
                {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                };
                if (!gltfNode.matrix.empty())
                {
                    DirectX::XMFLOAT4X4 transform =
                    {
                        1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1
                    };
                    // 行列データを変換
                    for (size_t row = 0; row < 4; row++)
                    {
                        for (size_t column = 0; column < 4; column++)
                        {
                            transform(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                        }
                    }
                    DirectX::XMStoreFloat4x4(&globalTransform, DirectX::XMLoadFloat4x4(&transform) * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
                }
                else
                {
                    // スケール、回転、平行移動の適用
                    DirectX::XMMATRIX S = gltfNode.scale.size() > 0 ? DirectX::XMMatrixScaling(static_cast<float>(gltfNode.scale.at(0)), static_cast<float>(gltfNode.scale.at(1)), static_cast<float>(gltfNode.scale.at(2))) : DirectX::XMMatrixIdentity();
                    DirectX::XMMATRIX R = gltfNode.rotation.size() > 0 ? DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(static_cast<float>(gltfNode.rotation.at(0)), static_cast<float>(gltfNode.rotation.at(1)), static_cast<float>(gltfNode.rotation.at(2)), static_cast<float>(gltfNode.rotation.at(3)))) : DirectX::XMMatrixIdentity();
                    DirectX::XMMATRIX T = gltfNode.translation.size() > 0 ? DirectX::XMMatrixTranslation(static_cast<float>(gltfNode.translation.at(0)), static_cast<float>(gltfNode.translation.at(1)), static_cast<float>(gltfNode.translation.at(2))) : DirectX::XMMatrixIdentity();
                    DirectX::XMStoreFloat4x4(&globalTransform, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
                }
                // ノードにメッシュがあればそのデータを格納
                if (gltfNode.mesh > -1)
                {
                    // glTFのメッシュを取得
                    std::vector<tinygltf::Mesh>::const_reference gltfMesh = gltfModel.meshes.at(gltfNode.mesh);
                    decltype(meshes)::reference mesh = meshes.emplace_back();
                    mesh.name = gltfMesh.name;
                    mesh.globalTransform = globalTransform;

                    // glTFのプリミティブを処理
                    for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
                    {
                        // マテリアル情報を取得
                        decltype(mesh.subsets)::reference subset = mesh.subsets.emplace_back();
                        if (gltfPrimitive.material > -1)
                        {
                            std::vector<tinygltf::Material>::const_reference gltfMaterial = gltfModel.materials.at(gltfPrimitive.material);
                            subset.materialName = gltfMaterial.name;
                        }

                        // 頂点座標の取得
                        const tinygltf::Accessor& positionAccessor = gltfModel.accessors.at(gltfPrimitive.attributes.at("POSITION"));
                        const tinygltf::BufferView& positionBufferView = gltfModel.bufferViews.at(positionAccessor.bufferView);
                        DirectX::XMFLOAT3* positions = reinterpret_cast<DirectX::XMFLOAT3*>(gltfModel.buffers.at(positionBufferView.buffer).data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset);

                        // インデックスバッファの取得
                        const tinygltf::Accessor& indexAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
                        const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews.at(indexAccessor.bufferView);


                        assert(indexAccessor.type == TINYGLTF_TYPE_SCALAR);
                        if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        {
                            unsigned int* indices = reinterpret_cast<unsigned int*>(gltfModel.buffers.at(indexBufferView.buffer).data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);

                            // 頂点データを格納（ワールド変換を適用）
                            for (size_t indexIndex = 0; indexIndex < indexAccessor.count; ++indexIndex)
                            {
                                unsigned int index = indices[indexIndex];
                                //unsigned int index = *reinterpret_cast<int *>(indices + indexIndex * 4);
                                DirectX::XMFLOAT3 position = positions[index];
                                DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), DirectX::XMLoadFloat4x4(&globalTransform)));
                                subset.positions.emplace_back(position);
                                subset.indices.push_back(static_cast<uint32_t>(subset.positions.size() - 1)); // ローカルインデックス追加した
                            }
                        }
                        else  if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            unsigned short* indices = reinterpret_cast<unsigned short*>(gltfModel.buffers.at(indexBufferView.buffer).data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);

                            // 頂点データを格納（ワールド変換を適用）
                            for (size_t indexIndex = 0; indexIndex < indexAccessor.count; ++indexIndex)
                            {
                                unsigned int index = indices[indexIndex];
                                DirectX::XMFLOAT3 position = positions[index];
                                DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), DirectX::XMLoadFloat4x4(&globalTransform)));
                                subset.positions.emplace_back(position);
                                subset.indices.push_back(static_cast<uint32_t>(subset.positions.size() - 1)); // ローカルインデックス追加した
                            }
                        }
                        else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            unsigned char* indices = reinterpret_cast<unsigned char*>(gltfModel.buffers.at(indexBufferView.buffer).data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);

                            for (size_t indexIndex = 0; indexIndex < indexAccessor.count; ++indexIndex)
                            {
                                unsigned int index = indices[indexIndex];
                                DirectX::XMFLOAT3 position = positions[index];
                                DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), DirectX::XMLoadFloat4x4(&globalTransform)));
                                subset.positions.emplace_back(position);
                                subset.indices.push_back(static_cast<uint32_t>(subset.positions.size() - 1)); // ローカルインデックス追加した
                            }
                        }
                        else
                        {
                            assert(FALSE);

                        }
                    }
                }

                // 子ノードを再帰的に処理
                for (decltype(gltfNode.children)::value_type childIndex : gltfNode.children)
                {
                    parentGlobalTransforms.push(globalTransform);
                    traverse(childIndex);
                    parentGlobalTransforms.pop();
                }
            };

        // シーンのルートノードを処理
        decltype(gltfModel.scenes)::const_reference gltfScene = gltfModel.scenes.at(0);
        for (decltype(gltfScene.nodes)::value_type rootNode : gltfScene.nodes)
            //decltype(gltfScene.nodes)::value_type rootNode = gltfScene.nodes.at(0);
        {
            parentGlobalTransforms.push(
                {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                });
            traverse(rootNode);
            parentGlobalTransforms.pop();
        }



        // 静的剛体作成
        {
            physx::PxPhysics* pxPhysics = Physics::Instance().GetPhysics();
            physx::PxScene* pxScene = Physics::Instance().GetScene();
            physx::PxMaterial* pxMaterial = Physics::Instance().GetMaterial();

            //const ModelResource* resource = model->GetResource();

            DirectX::XMMATRIX Transform = GetComponentWorldTransform().ToMatrix();


            for (const StaticMeshCollisionComponent::Mesh& mesh : meshes)
            {
                for (const auto& subset : mesh.subsets)
                {
                    // 三角形メッシュ作成
                    physx::PxTriangleMeshDesc pxTriangleMeshDesc;
                    pxTriangleMeshDesc.points.count = static_cast<physx::PxU32>(subset.positions.size());
                    pxTriangleMeshDesc.points.data = subset.positions.data();
                    pxTriangleMeshDesc.points.stride = sizeof(DirectX::XMFLOAT3);


                    pxTriangleMeshDesc.triangles.count = static_cast<physx::PxU32>(subset.indices.size() / 3);
                    pxTriangleMeshDesc.triangles.data = subset.indices.data();
                    pxTriangleMeshDesc.triangles.stride = sizeof(uint32_t) * 3;

                    physx::PxTolerancesScale pxTolerances;
                    const physx::PxCookingParams pxCookingParams(pxTolerances);
                    physx::PxTriangleMesh* pxTriangleMesh = PxCreateTriangleMesh(pxCookingParams, pxTriangleMeshDesc);

                    // 静的剛体作成
                    DirectX::XMMATRIX NodeTransform = DirectX::XMLoadFloat4x4(&mesh.globalTransform) * Transform;
                    physx::PxVec3 pxScale(
                        DirectX::XMVectorGetX(DirectX::XMVector3Length(NodeTransform.r[0])),
                        DirectX::XMVectorGetX(DirectX::XMVector3Length(NodeTransform.r[1])),
                        DirectX::XMVectorGetX(DirectX::XMVector3Length(NodeTransform.r[2]))
                    );
                    NodeTransform.r[0] = DirectX::XMVector3Normalize(NodeTransform.r[0]);
                    NodeTransform.r[1] = DirectX::XMVector3Normalize(NodeTransform.r[1]);
                    NodeTransform.r[2] = DirectX::XMVector3Normalize(NodeTransform.r[2]);

                    DirectX::XMFLOAT4X4 nodeTransform;
                    DirectX::XMStoreFloat4x4(&nodeTransform, NodeTransform);
                    physx::PxTransform pxTransform(physx::PxMat44(
                        physx::PxVec3(nodeTransform._11, nodeTransform._12, nodeTransform._13),
                        physx::PxVec3(nodeTransform._21, nodeTransform._22, nodeTransform._23),
                        physx::PxVec3(nodeTransform._31, nodeTransform._32, nodeTransform._33),
                        physx::PxVec3(nodeTransform._41, nodeTransform._42, nodeTransform._43)
                    ));
                    physx::PxRigidStatic* pxRigidBody = pxPhysics->createRigidStatic(pxTransform);
                    _ASSERT_EXPR(pxRigidBody != nullptr, "Failed pxPhysics->createRigidStatic");
                    pxRigidBody->setName("stage");

                    // 静的剛体にメッシュ形状を関連付け
                    physx::PxMeshScale pxMeshScale(pxScale);
                    physx::PxTriangleMeshGeometry pxTriangleMeshGeometry(pxTriangleMesh, pxMeshScale);
                    physx::PxShape* pxShape = physx::PxRigidActorExt::createExclusiveShape(*pxRigidBody, pxTriangleMeshGeometry, *pxMaterial);

                    // レイキャストなど用のレイヤー設定
                    physx::PxFilterData pxFilterData = pxShape->getQueryFilterData();

                    //pxFilterData.word0 = static_cast<uint32_t>(ShapeComponent::CollisionLayer::Enemy);
                    //pxFilterData.word1 = static_cast<uint32_t>(ShapeComponent::CollisionLayer::Player) | static_cast<uint32_t>(ShapeComponent::CollisionLayer::WorldStatic);
                    pxFilterData.word0 = 1;
                    pxFilterData.word1 = 1;
                    pxShape->setQueryFilterData(pxFilterData);

                    // 物理シーンに追加
                    Physics::Instance().GetScene()->addActor(*pxRigidBody);

                    // 削除用にポインタを保持
                    pxActors_.emplace_back(pxRigidBody);
                    pxTriangleMeshes_.emplace_back(pxTriangleMesh);
                }
            }
        }
    }


private:
    // 削除用にポインタを保持する
    std::vector<physx::PxTriangleMesh*>	pxTriangleMeshes_;
    std::vector<physx::PxActor*>		pxActors_;

};
#endif //STATIC_MESH_COLLISION_COMPONENT_H

