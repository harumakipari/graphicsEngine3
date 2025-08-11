#include "CollisionMesh.h"

#include <stack>
#include <functional>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <functional>
#include <fbxsdk.h>

using namespace DirectX;

//FBXの行列をDirectXのXMFLOAT4x4に変換する関数
inline XMFLOAT4X4 ToXmfloat4x4(const FbxAMatrix& fbxamatrix)
{
    XMFLOAT4X4 xmfloat4x4;
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

// コンストラクタ: FBXまたはglTFファイルを読み込む
CollisionMesh::CollisionMesh(ID3D11Device* device, const std::string& filename, bool triangulate/*ignored*/)
{
    // FBXかglTFを選択して読み込む
    if (filename.find(".fbx") != std::string::npos)
    {// FBXファイルの場合
        FbxManager* fbxManager = FbxManager::Create();
        FbxScene* fbxScene = FbxScene::Create(fbxManager, "");
        FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
        bool importStatus = false;
        importStatus = fbxImporter->Initialize(filename.c_str());
        assert(importStatus && "Failed to call FbxImporter::Initialize");
        importStatus = fbxImporter->Import(fbxScene);
        assert(importStatus && "Failed to call FbxImporter::Import");

        FbxGeometryConverter fbxConverter(fbxManager);
        // 必要に応じてメッシュを三角形化
        if (triangulate)
        {
            fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
            fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);
        }

        // ノードを再帰的に処理
        std::function<void(FbxNode*)> traverse = [&](FbxNode* fbxNode) {
            if (fbxNode->GetNodeAttribute() && fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
            {
                // メッシュデータの格納
                decltype(meshes)::reference mesh = meshes.emplace_back();
                mesh.name = fbxNode->GetName();

                // ノードのグローバルトランスフォーム
                DirectX::XMFLOAT4X4 nodeTransform = ToXmfloat4x4(fbxNode->EvaluateGlobalTransform());
                XMMATRIX globalTransform = XMLoadFloat4x4(&nodeTransform);

                FbxMesh* fbxMesh = fbxNode->GetMesh();
                const int materialCount = fbxMesh->GetNode()->GetMaterialCount();

                // サブセットのサイズを設定
                mesh.subsets.resize(materialCount > 0 ? materialCount : 1);
                // マテリアル名を取得
                for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
                {
                    const FbxSurfaceMaterial* fbxMaterial = fbxMesh->GetNode()->GetMaterial(materialIndex);
                    mesh.subsets.at(materialIndex).materialName = fbxMaterial->GetName();
                }

                const FbxVector4* controlPoints = fbxMesh->GetControlPoints();
                const int polygonCount = fbxMesh->GetPolygonCount();
                // ポリゴンデータを処理
                for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
                {
                    const int materialIndex = materialCount > 0 ? fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0;
                    decltype(mesh.subsets)::reference subset = mesh.subsets.at(materialIndex);

                    for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
                    {
                        DirectX::XMFLOAT3 position;
                        // 各ポリゴンの頂点座標を取得
                        const int polygonVertex = fbxMesh->GetPolygonVertex(polygonIndex, positionInPolygon);
                        position.x = static_cast<float>(controlPoints[polygonVertex][0]);
                        position.y = static_cast<float>(controlPoints[polygonVertex][1]);
                        position.z = static_cast<float>(controlPoints[polygonVertex][2]);
                        // グローバル変換を適用
                        XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), globalTransform));
                        subset.positions.emplace_back(position);
                    }
                }
                // バウンディングボックスの更新
                for (decltype(mesh.subsets)::const_reference subset : mesh.subsets)
                {
                    for (decltype(subset.positions)::const_reference position : subset.positions)
                    {
                        mesh.boundingBox[0].x = std::min<float>(mesh.boundingBox[0].x, position.x);
                        mesh.boundingBox[0].y = std::min<float>(mesh.boundingBox[0].y, position.y);
                        mesh.boundingBox[0].z = std::min<float>(mesh.boundingBox[0].z, position.z);
                        mesh.boundingBox[1].x = std::max<float>(mesh.boundingBox[1].x, position.x);
                        mesh.boundingBox[1].y = std::max<float>(mesh.boundingBox[1].y, position.y);
                        mesh.boundingBox[1].z = std::max<float>(mesh.boundingBox[1].z, position.z);
                    }
                }
            }
            // 子ノードを再帰的に処理
            for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
            {
                traverse(fbxNode->GetChild(childIndex));
            }
            };
        // ルートノードから処理を開始
        traverse(fbxScene->GetRootNode());
        // FBXリソースを解放
        fbxManager->Destroy();
    }
    else
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
                // メッシュがあればそのデータを格納
                if (gltfNode.mesh > -1)
                {
                    // glTFのメッシュを取得
                    std::vector<tinygltf::Mesh>::const_reference gltfMesh = gltfModel.meshes.at(gltfNode.mesh);
                    decltype(meshes)::reference mesh = meshes.emplace_back();
                    mesh.name = gltfMesh.name;

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
                            }
                        }
                        else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            unsigned char* indices = reinterpret_cast<unsigned char*>(gltfModel.buffers.at(indexBufferView.buffer).data.data() +indexBufferView.byteOffset + indexAccessor.byteOffset);

                            for (size_t indexIndex = 0; indexIndex < indexAccessor.count; ++indexIndex)
                            {
                                unsigned int index = indices[indexIndex];
                                DirectX::XMFLOAT3 position = positions[index];
                                DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), DirectX::XMLoadFloat4x4(&globalTransform)));
                                subset.positions.emplace_back(position);
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
    }

    // AABB（バウンディングボックス）の再計算
    for (decltype(meshes)::reference mesh : meshes)
    {
        for (decltype(mesh.subsets)::reference subset : mesh.subsets)
        {
            for (decltype(subset.positions)::reference position : subset.positions)
            {
                mesh.boundingBox[0].x = std::min<float>(mesh.boundingBox[0].x, position.x);
                mesh.boundingBox[0].y = std::min<float>(mesh.boundingBox[0].y, position.y);
                mesh.boundingBox[0].z = std::min<float>(mesh.boundingBox[0].z, position.z);
                mesh.boundingBox[1].x = std::max<float>(mesh.boundingBox[1].x, position.x);
                mesh.boundingBox[1].y = std::max<float>(mesh.boundingBox[1].y, position.y);
                mesh.boundingBox[1].z = std::max<float>(mesh.boundingBox[1].z, position.z);
            }
        }
    }
}

inline bool intersectRayAabb(const float p[3], const float d[3], const float p0[3], const float p1[3])
{
    float tMin = 0;
    float tMax = +FLT_MAX;

    // 3D空間の各軸（X, Y, Z）ごとにAABBとの交差判定を行う
    for (size_t a = 0; a < 3; ++a)
    {
        float inv_d = 1.0f / d[a];// 逆方向のベクトルを求める
        float t0 = (p0[a] - p[a]) * inv_d;
        float t1 = (p1[a] - p[a]) * inv_d;
        if (inv_d < 0.0f)
        {
            std::swap<float>(t0, t1);
        }
        tMin = std::max<float>(t0, tMin);
        tMax = std::min<float>(t1, tMax);

        if (tMax <= tMin)
        {
            return false;
        }
    }
    return true; // 交差している
}

// レイと衝突するメッシュを判定// ワールド座標に変換してから渡す
bool CollisionMesh::Raycast(_In_ XMFLOAT3 rayPosition, _In_ XMFLOAT3 rayDirection, _In_ const XMFLOAT4X4& transform, _Out_ XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
    _Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ float rayLengthLimit, _In_ bool skipIf) const
{
    // ワールド座標からモデル空間へ変換
    XMMATRIX T = XMLoadFloat4x4(&transform);
    XMMATRIX _T = XMMatrixInverse(NULL, T);

    XMStoreFloat3(&rayPosition, XMVector3TransformCoord(XMLoadFloat3(&rayPosition), _T));
    XMStoreFloat3(&rayDirection, XMVector3TransformNormal(XMLoadFloat3(&rayDirection), _T));

    const XMVECTOR P = XMLoadFloat3(&rayPosition);// レイの開始点
    const XMVECTOR D = XMVector3Normalize(XMLoadFloat3(&rayDirection));// レイの方向を正規化

    int intersectionCount = 0;
    float closestDistance = FLT_MAX;

    // 各メッシュをチェック
    for (decltype(meshes)::const_reference mesh : meshes)
    {
#if 1
        // AABB（境界ボックス）との交差判定
        const float* aabbMin = reinterpret_cast<const float*>(&mesh.boundingBox[0]);
        const float* aabbMax = reinterpret_cast<const float*>(&mesh.boundingBox[1]);
        if (!intersectRayAabb(reinterpret_cast<const float*>(&rayPosition), reinterpret_cast<const float*>(&rayDirection), aabbMin, aabbMax))
        {
            continue;// 交差しない場合、次のメッシュへ
        }
#endif
        // メッシュ内の各サブセット（ポリゴン群）をチェック
        for (decltype(mesh.subsets)::const_reference subset : mesh.subsets)
        {
            const XMFLOAT3* positions = subset.positions.data();
            const size_t triangleCount = subset.positions.size() / 3;

            for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
            {
                // 三角形の各頂点を取得
                const XMVECTOR A = XMLoadFloat3(&positions[triangleIndex * 3 + 0]);
                const XMVECTOR B = XMLoadFloat3(&positions[triangleIndex * 3 + 1]);
                const XMVECTOR C = XMLoadFloat3(&positions[triangleIndex * 3 + 2]);

                // 三角形の法線を計算
                const XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A)));
                const float d = XMVectorGetByIndex(XMVector3Dot(N, A), 0);
                const float denominator{ XMVectorGetByIndex(XMVector3Dot(N, D), 0) };

                // レイが三角形の平面と交差するか判定
                if (denominator < 0) // Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
                {
                    const float numerator = d - XMVectorGetByIndex(XMVector3Dot(N, P), 0);
                    const float t = numerator / denominator;

                    // レイの交差が有効範囲内かチェック
                    if (t > 0 && t < rayLengthLimit) // Forward and Length limit of Ray
                    {
                        XMVECTOR Q = XMVectorAdd(P, XMVectorScale(D, t));// 交差点

                        // 三角形内に点があるかチェック
                        const XMVECTOR QA = XMVectorSubtract(A, Q);
                        const XMVECTOR QB = XMVectorSubtract(B, Q);
                        const XMVECTOR QC = XMVectorSubtract(C, Q);

                        XMVECTOR U = XMVector3Cross(QB, QC);
                        XMVECTOR V = XMVector3Cross(QC, QA);
                        if (XMVectorGetByIndex(XMVector3Dot(U, V), 0) < 0)
                        {
                            continue;
                        }

                        XMVECTOR W = XMVector3Cross(QA, QB);
                        if (XMVectorGetByIndex(XMVector3Dot(U, W), 0) < 0)
                        {
                            continue;
                        }
                        if (XMVectorGetByIndex(XMVector3Dot(V, W), 0) < 0)
                        {
                            continue;
                        }
                        intersectionCount++;

                        // 最も近い交差点を更新
                        if (t < closestDistance)
                        {
                            closestDistance = t;

                            XMStoreFloat3(&intersectionPosition, XMVector3TransformCoord(Q, T));
                            XMStoreFloat3(&intersectionNormal, XMVector3Normalize(XMVector3TransformNormal(N, T)));
                            intersectionMesh = mesh.name;
                            intersectionMaterial = subset.materialName;

                            if (skipIf)// 最初の交差点で終了
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return intersectionCount > 0;// 一つ以上交差があれば true
}

