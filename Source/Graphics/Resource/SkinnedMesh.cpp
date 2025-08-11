#include "SkinnedMesh.h"

#include <sstream>
#include <fstream>
#include <functional>
#include <filesystem>

#include "misc.h"
#include "Shader.h"
#include "Graphics/Texture.h"

struct BoneInfluence
{
    uint32_t boneIndex;
    float boneWeight;
};
using BoneInfluencePerControlPoint = std::vector<BoneInfluence>;

void FetchBoneInfluences(const FbxMesh* fbxMesh, std::vector<BoneInfluencePerControlPoint>& boneInfluences)
{
    const int controlPointsCount{ fbxMesh->GetControlPointsCount() };
    boneInfluences.resize(controlPointsCount);

    const int skinCount{ fbxMesh->GetDeformerCount(FbxDeformer::eSkin) };
    for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
    {
        const FbxSkin* fbxSkin
        { static_cast<FbxSkin*> (fbxMesh->GetDeformer(skinIndex,FbxDeformer::eSkin)) };

        const int clusterCount{ fbxSkin->GetClusterCount() };
        for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
        {
            const FbxCluster* fbxCluster{ fbxSkin->GetCluster(clusterIndex) };

            const int controlPointIndeicesCount{ fbxCluster->GetControlPointIndicesCount() };
            for (int controlPointIndicesIndex = 0; controlPointIndicesIndex < controlPointIndeicesCount; ++controlPointIndicesIndex)
            {
                int controlPointIndex
                { fbxCluster->GetControlPointIndices()[controlPointIndicesIndex] };
                double controlPointWeight
                { fbxCluster->GetControlPointWeights()[controlPointIndicesIndex] };
                BoneInfluence& boneInfluence{ boneInfluences.at(controlPointIndex).emplace_back() };
                boneInfluence.boneIndex = static_cast<uint32_t>(clusterIndex);
                boneInfluence.boneWeight = static_cast<float>(controlPointWeight);
            }
        }
    }
}

inline DirectX::XMFLOAT4X4 ToXmfloat4x4(const FbxAMatrix& fbxamatrix)
{
    DirectX::XMFLOAT4X4 xmfloat4x4;
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
        }
    }
    return xmfloat4x4;
}

inline DirectX::XMFLOAT3 ToXmfloat3(const FbxDouble3& fbxdouble3)
{
    DirectX::XMFLOAT3 xmfloat3;
    xmfloat3.x = static_cast<float>(fbxdouble3[0]);
    xmfloat3.y = static_cast<float>(fbxdouble3[1]);
    xmfloat3.z = static_cast<float>(fbxdouble3[2]);
    return xmfloat3;
}

inline DirectX::XMFLOAT4 ToXmfloat4(const FbxDouble4& fbxdouble4)
{
    DirectX::XMFLOAT4 xmfloat4;
    xmfloat4.x = static_cast<float>(fbxdouble4[0]);
    xmfloat4.y = static_cast<float>(fbxdouble4[1]);
    xmfloat4.z = static_cast<float>(fbxdouble4[2]);
    xmfloat4.w = static_cast<float>(fbxdouble4[3]);
    return xmfloat4;
}

SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbxFilename, float samplingRate, bool triangulate)
{
    //このコードでは与えられた変数fbx_filenameの拡張子を“cereal”に変え、そのファイルが存在する場合はシリアライズされたデータ
    //からロードする。また、存在しない場合は従来通りFBXファイルからロードする。 
    //skinnd_meshクラスのデータ構造に変更があった場合はシリアライズされたファイルは無効になるので削除する必要がある。 

    std::filesystem::path cerealFilename(fbxFilename);
    cerealFilename.replace_extension("cereal");
    if (std::filesystem::exists(cerealFilename.c_str()))
    {
        std::ifstream  ifs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(sceneView, meshes, materials, animationClips);
    }
    else
    {
        FbxManager* fbxManager{ FbxManager::Create() };
        FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

        FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
        bool importStatus{ false };
        importStatus = fbxImporter->Initialize(fbxFilename);
        _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

        importStatus = fbxImporter->Import(fbxScene);
        _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

        FbxGeometryConverter fbxConverter(fbxManager);
        if (triangulate)
        {
            fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
            fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);
        }

        std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbxNode)
        {
            SCENE::Node& node{sceneView.nodes.emplace_back()};
            node.attribute = fbxNode->GetNodeAttribute() ?
                fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
            node.name = fbxNode->GetName();
            node.uniqueId = fbxNode->GetUniqueID();
            node.parentIndex = sceneView.indexof(fbxNode->GetParent() ?
                fbxNode->GetParent()->GetUniqueID() : 0);
            for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
            {
                traverse(fbxNode->GetChild(childIndex));
            }
        } };
        traverse(fbxScene->GetRootNode());
#if 0
        for (const Scene::Node& node : sceneView.nodes)
        {
            FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
            //Display node data in the output window as debug
            std::string nodeName = fbxNode->GetName();
            uint64_t uid = fbxNode->GetUniqueID();
            uint64_t parentUid = fbxNode->GetParent() ? fbxNode->GetParent()->GetUniqueID() : 0;
            int32_t type = fbxNode->GetNodeAttribute() ? fbxNode->GetNodeAttribute()->GetAttributeType() : 0;

            std::stringstream debugString;
            debugString << nodeName << ":" << uid << ":" << parentUid << ":" << type << "\n";
            OutputDebugStringA(debugString.str().c_str());
        }
#endif
        //三角化
        fbxConverter.Triangulate(fbxScene, true, false);
        FetchMeshes(fbxScene, meshes);
        FetchMaterials(fbxScene, materials);
        FetchAnimations(fbxScene, animationClips, samplingRate);
        fbxManager->Destroy();


        std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(sceneView, meshes, materials, animationClips);
    }
    CreateComObject(device, fbxFilename);
}

SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbx_filename, std::vector<std::string>& animation_filenames, bool triangulate, float sampling_rate)
{
    std::filesystem::path cereal_filename(fbx_filename);
    cereal_filename.replace_extension("cereal");
    if (std::filesystem::exists(cereal_filename.c_str()))
    {
        std::ifstream ifs(cereal_filename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(sceneView, meshes, materials, animationClips);
    }
    else
    {
        // UNIT.30
        FetchScene(fbx_filename, triangulate, sampling_rate);

        // UNIT.30
        for (const std::string animation_filename : animation_filenames)
        {
            AppendAnimations(animation_filename.c_str(), sampling_rate);
        }

        // UNIT.30
        std::ofstream ofs(cereal_filename.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(sceneView, meshes, materials, animationClips);
    }
    // UNIT.18
    CreateComObject(device, fbx_filename);
}

// UNIT.30
void SkinnedMesh::FetchScene(const char* fbx_filename, bool triangulate, float sampling_rate)
{
    FbxManager* fbx_manager{ FbxManager::Create() };
    FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };
    FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
    bool import_status{ false };
    import_status = fbx_importer->Initialize(fbx_filename);
    _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());
    import_status = fbx_importer->Import(fbx_scene);
    _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

    FbxGeometryConverter fbx_converter(fbx_manager);
    if (triangulate)
    {
        fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
        fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
    }

    // Serialize an entire scene graph into sequence container
    std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbx_node) {
#if 0
        if (fbx_node->GetNodeAttribute())
        {
            switch (fbx_node->GetNodeAttribute()->GetAttributeType())
            {
            case FbxNodeAttribute::EType::eNull:
            case FbxNodeAttribute::EType::eMesh:
            case FbxNodeAttribute::EType::eSkeleton:
            case FbxNodeAttribute::EType::eUnknown:
            case FbxNodeAttribute::EType::eMarker:
            case FbxNodeAttribute::EType::eCamera:
            case FbxNodeAttribute::EType::eLight:
                scene::node& node{ scene_view.nodes.emplace_back() };
                node.attribute = fbx_node->GetNodeAttribute()->GetAttributeType();
                node.name = fbx_node->GetName();
                node.unique_id = fbx_node->GetUniqueID();
                node.parent_index = scene_view.indexof(fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0);
                break;
            }
        }
#else
        SCENE::Node& node{ sceneView.nodes.emplace_back() };
        node.attribute = fbx_node->GetNodeAttribute() ? fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
        node.name = fbx_node->GetName();
        node.uniqueId = fbx_node->GetUniqueID();
        node.parentIndex = sceneView.indexof(fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0);
#endif
        for (int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
        {
            traverse(fbx_node->GetChild(child_index));
        }
    } };
    traverse(fbx_scene->GetRootNode());

    // UNIT.18
    FetchMeshes(fbx_scene, meshes);

    // UNIT.19
    FetchMaterials(fbx_scene, materials);

    // UNIT.25
#if 0
    float sampling_rate{ 0 };
#endif
    FetchAnimations(fbx_scene, animationClips, sampling_rate);

    // UNIT.17
    fbx_manager->Destroy();
}


void SkinnedMesh::FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes)
{
    for (const SCENE::Node& node : sceneView.nodes)
    {
        if (node.attribute != FbxNodeAttribute::EType::eMesh)
        {
            continue;
        }
        FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
        FbxMesh* fbxMesh{ fbxNode->GetMesh() };

        Mesh& mesh{ meshes.emplace_back() };
        mesh.uniqueId = fbxNode->GetUniqueID();
        mesh.name = fbxNode->GetName();
        mesh.nodeIndex = sceneView.indexof(mesh.uniqueId);
        mesh.defaultGlobalTransform = ToXmfloat4x4(fbxNode->EvaluateGlobalTransform());

        std::vector< BoneInfluencePerControlPoint> boneInfluences;
        FetchBoneInfluences(fbxMesh, boneInfluences);

        FetchSkelton(fbxMesh, mesh.bindPose);

        std::vector<Mesh::Subset>& subsets{ mesh.subsets };
        const int materialCount{ fbxMesh->GetNode()->GetMaterialCount() };
        subsets.resize(materialCount > 0 ? materialCount : 1);
        for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
        {
            const FbxSurfaceMaterial* fbxMaterual{ fbxMesh->GetNode()->GetMaterial(materialIndex) };
            subsets.at(materialIndex).materialName = fbxMaterual->GetName();
            subsets.at(materialIndex).materialUniqueId = fbxMaterual->GetUniqueID();
        }
        if (materialCount > 0)
        {
            const int polygonCount{ fbxMesh->GetPolygonCount() };
            for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
            {
                const int materialIndex
                { fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) };
                subsets.at(materialIndex).indexCount += 3;
            }
        }
        uint32_t offset{ 0 };
        for (Mesh::Subset& subset : subsets)
        {
            subset.startIndexLocation = offset;
            offset += subset.indexCount;
            //This will be used as counter in the following procedures,reset to zero
            subset.indexCount = 0;
        }

        const int polygonCount{ fbxMesh->GetPolygonCount() };
        mesh.vertices.resize(polygonCount * 3LL);
        mesh.indices.resize(polygonCount * 3LL);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };

        //境界線の情報を入れる
        for (const Vertex& v : mesh.vertices)
        {
            mesh.boundingBox[0].x = std::min<float>(mesh.boundingBox[0].x, v.position.x);
            mesh.boundingBox[0].y = std::min<float>(mesh.boundingBox[0].y, v.position.y);
            mesh.boundingBox[0].z = std::min<float>(mesh.boundingBox[0].z, v.position.z);
            mesh.boundingBox[1].x = std::max<float>(mesh.boundingBox[1].x, v.position.x);
            mesh.boundingBox[1].y = std::max<float>(mesh.boundingBox[1].y, v.position.y);
            mesh.boundingBox[1].z = std::max<float>(mesh.boundingBox[1].z, v.position.z);
        }


        for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
        {
            const int materialIndex
            { materialCount > 0 ? fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0 };
            Mesh::Subset& subset{ subsets.at(materialIndex) };
            const uint32_t offset{ subset.startIndexLocation + subset.indexCount };

            for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
            {
                const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

                Vertex vertex;
                const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex,positionInPolygon) };
                vertex.position.x = static_cast<float>(controlPoints[polygonVertex][0]);
                vertex.position.y = static_cast<float>(controlPoints[polygonVertex][1]);
                vertex.position.z = static_cast<float>(controlPoints[polygonVertex][2]);

                const BoneInfluencePerControlPoint& influencesPerControlPoint{ boneInfluences.at(polygonVertex) };
                for (size_t influenceIndex = 0; influenceIndex < influencesPerControlPoint.size(); influenceIndex++)
                {
                    if (influenceIndex < MAX_BONE_INFLUENCES)
                    {
                        vertex.boneWeights[influenceIndex] =
                            influencesPerControlPoint.at(influenceIndex).boneWeight;
                        vertex.boneIndices[influenceIndex] =
                            influencesPerControlPoint.at(influenceIndex).boneIndex;
                    }
                    else
                    {
                        //TODO:01pdf22 5.下記コードは影響を受けるボーン数が４つを超える場合は、それ以降の影響度を無視している（改善しなさい）
                        _ASSERT_EXPR(FALSE, L"(ToT)");

                    }
                }
                //法線ベクトルの取得
                if (fbxMesh->GetElementNormalCount() > 0)
                {
                    FbxVector4 normal;
                    fbxMesh->GetPolygonVertexNormal(polygonIndex, positionInPolygon, normal);
                    vertex.normal.x = static_cast<float>(normal[0]);
                    vertex.normal.y = static_cast<float>(normal[1]);
                    vertex.normal.z = static_cast<float>(normal[2]);
                }
                //テクスチャ座標を取得
                if (fbxMesh->GetElementUVCount() > 0)
                {
                    FbxVector2 uv;
                    bool unmappedUv;
                    fbxMesh->GetPolygonVertexUV(polygonIndex, positionInPolygon, uvNames[0], uv, unmappedUv);
                    vertex.texcoord.x = static_cast<float>(uv[0]);
                    vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
                }

                if (fbxMesh->GenerateTangentsData(0, false))
                {
                    const FbxGeometryElementTangent* tangent = fbxMesh->GetElementTangent(0);
                    vertex.tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[0]);
                    vertex.tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[1]);
                    vertex.tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[2]);
                    vertex.tangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[3]);
                }
                mesh.vertices.at(vertexIndex) = std::move(vertex);
                mesh.indices.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
                subset.indexCount++;
            }
        }
    }
}

void SkinnedMesh::UpdateAnimation(Animation::Keyframe& keyframe)
{
    size_t nodeCount{ keyframe.nodes.size() };
    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        Animation::Keyframe::Node& node{ keyframe.nodes.at(nodeIndex) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scaling.x,node.scaling.y,node.scaling.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation)) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation.x,node.translation.y,node.translation.z) };

        int64_t parentIndex{ sceneView.nodes.at(nodeIndex).parentIndex };
        DirectX::XMMATRIX P{ parentIndex < 0 ? DirectX::XMMatrixIdentity() :
        DirectX::XMLoadFloat4x4(&keyframe.nodes.at(parentIndex).globalTransform) };

        DirectX::XMStoreFloat4x4(&node.globalTransform, S * R * T * P);
    }
}

bool SkinnedMesh::AppendAnimations(const char* animationFilename, float samplingRate)
{
    FbxManager* fbxManager{ FbxManager::Create() };
    FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

    FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
    bool importStatus{ false };
    importStatus = fbxImporter->Initialize(animationFilename);
    _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());
    importStatus = fbxImporter->Import(fbxScene);
    _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

    FetchAnimations(fbxScene, animationClips, samplingRate);

    fbxManager->Destroy();

    return true;
}

void SkinnedMesh::BlendAnimations(const Animation::Keyframe* keyframes[2], float factor,
    Animation::Keyframe& keyframe)
{
    size_t nodeCount{ keyframes[0]->nodes.size() };
    keyframe.nodes.resize(nodeCount);
    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        DirectX::XMVECTOR S[2]
        {
            DirectX::XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).scaling),
            DirectX::XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).scaling)
        };
        DirectX::XMStoreFloat3(&keyframe.nodes.at(nodeIndex).scaling, DirectX::XMVectorLerp(S[0], S[1], factor));

        DirectX::XMVECTOR R[2]
        {
            DirectX::XMLoadFloat4(&keyframes[0]->nodes.at(nodeIndex).rotation),
            DirectX::XMLoadFloat4(&keyframes[1]->nodes.at(nodeIndex).rotation)
        };
        DirectX::XMStoreFloat4(&keyframe.nodes.at(nodeIndex).rotation, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

        DirectX::XMVECTOR T[2]
        {
            DirectX::XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).translation),
            DirectX::XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).translation)
        };
        DirectX::XMStoreFloat3(&keyframe.nodes.at(nodeIndex).translation, DirectX::XMVectorLerp(T[0], T[1], factor));
    }
}

void SkinnedMesh::FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials)
{

    const size_t nodeCount{ sceneView.nodes.size() };
    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        const SCENE::Node& node{ sceneView.nodes.at(nodeIndex) };
        const FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };

        const int materialCount{ fbxNode->GetMaterialCount() };
        for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
        {
            const FbxSurfaceMaterial* fbxMaterial{ fbxNode->GetMaterial(materialIndex) };

            Material material;
            material.name = fbxMaterial->GetName();
            material.uniqueId = fbxMaterial->GetUniqueID();
            FbxProperty fbxProperty;
            fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
            if (fbxProperty.IsValid())
            {
                const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
                material.Kd.x = static_cast<float>(color[0]);
                material.Kd.y = static_cast<float>(color[1]);
                material.Kd.z = static_cast<float>(color[2]);
                material.Kd.w = 1.0f;

                const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                material.textureFilenames[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
                material.textureFilenames[1] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
            }

            materials.emplace(material.uniqueId, std::move(material));

        }
    }


    Material dummy_material;
    materials.emplace(0, dummy_material);

}

void SkinnedMesh::FetchSkelton(FbxMesh* fbxMesh, Skelton& bindPose)
{
    const int deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
    {
        FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
        const int clusterCount = skin->GetClusterCount();
        bindPose.bones.resize(clusterCount);
        for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
        {
            FbxCluster* cluster = skin->GetCluster(clusterIndex);

            Skelton::Bone& bone{ bindPose.bones.at(clusterIndex) };
            bone.name = cluster->GetLink()->GetName();
            bone.uniqueId = cluster->GetLink()->GetUniqueID();
            bone.parentIndex = bindPose.indexOf(cluster->GetLink()->GetParent()->GetUniqueID());
            bone.nodeIndex = sceneView.indexof(bone.uniqueId);

            //'reference_global_init_position' is used to convert from local space of model(mesh) to
            // global space of scene.
            FbxAMatrix referenceGlobalInitPosition;
            cluster->GetTransformMatrix(referenceGlobalInitPosition);

            // 'cluster_global_init_position' is used to convert from local space of bone to 
            // global space of scene. 
            FbxAMatrix clusterGlobalInitPosition;
            cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

            // Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation 
            // (translation, rotation and scale), the last row of the matrix represents the translation part of 
            // the transformation.
            // Compose 'bone.offset_transform' matrix that transforms position from mesh space to bone space.
            // This matrix is called the offset matrix. 

            bone.offsetTransform =
                ToXmfloat4x4(clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition);
        }
    }
}

//FBXシーンからアニメーションの情報を抽出する
void SkinnedMesh::FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
    float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/)
{
    FbxArray<FbxString*> animationStackNames;
    fbxScene->FillAnimStackNameArray(animationStackNames);
    const int animationStackCount{ animationStackNames.GetCount() };
    for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
    {
        Animation& animationClip{ animationClips.emplace_back() };
        animationClip.name = animationStackNames[animationStackIndex]->Buffer();

        FbxAnimStack* animationStack{ fbxScene->FindMember<FbxAnimStack>(animationClip.name.c_str()) };
        fbxScene->SetCurrentAnimationStack(animationStack);

        const FbxTime::EMode timeMode{ fbxScene->GetGlobalSettings().GetTimeMode() };
        FbxTime oneSecond;
        oneSecond.SetTime(0, 0, 1, 0, 0, timeMode);
        animationClip.samplingRate = samplingRate > 0 ?
            samplingRate : static_cast<float>(oneSecond.GetFrameRate(timeMode));
        const FbxTime samplingInterval
        { static_cast<FbxLongLong>(oneSecond.Get() / animationClip.samplingRate) };
        const FbxTakeInfo* takeInfo{ fbxScene->GetTakeInfo(animationClip.name.c_str()) };
        const FbxTime startTime{ takeInfo->mLocalTimeSpan.GetStart() };
        const FbxTime stopTime{ takeInfo->mLocalTimeSpan.GetStop() };
        for (FbxTime time = startTime; time < stopTime; time += samplingInterval)
        {
            Animation::Keyframe& keyframe{ animationClip.sequence.emplace_back() };

            const size_t nodeCount{ sceneView.nodes.size() };
            keyframe.nodes.resize(nodeCount);
            for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
            {
                FbxNode* fbxNode{ fbxScene->FindNodeByName(sceneView.nodes.at(nodeIndex).name.c_str()) };
                if (fbxNode)
                {
                    Animation::Keyframe::Node& node{ keyframe.nodes.at(nodeIndex) };
                    // 'global_transform' is a transformation matrix of a node with respect to 
                    // the scene's global coordinate system. 
                    node.globalTransform = ToXmfloat4x4(fbxNode->EvaluateGlobalTransform(time));

                    // 'local_transform' is a transformation matrix of a node with respect to
                    // its parent's local coordinate system. 
                    const FbxAMatrix& localTransform{ fbxNode->EvaluateLocalTransform(time) };
                    node.scaling = ToXmfloat3(localTransform.GetS());
                    node.rotation = ToXmfloat4(localTransform.GetQ());
                    node.translation = ToXmfloat3(localTransform.GetT());
                }
            }
        }
    }
    for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
    {
        delete animationStackNames[animationStackIndex];
    }
}


void SkinnedMesh::CreateComObject(ID3D11Device* device, const char* fbxFilename)
{
    for (Mesh& mesh : meshes)
    {
        HRESULT hr{ S_OK };
        D3D11_BUFFER_DESC bufferDesc{};
        D3D11_SUBRESOURCE_DATA subresourceData{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        subresourceData.pSysMem = mesh.vertices.data();
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        subresourceData.pSysMem = mesh.indices.data();
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#if 1
        mesh.vertices.clear();
        mesh.indices.clear();
#endif
    }

    for (std::unordered_map<uint64_t, Material>::iterator iterator = materials.begin(); iterator != materials.end(); ++iterator)
    {
        for (size_t textureIndex = 0; textureIndex < 2; ++textureIndex)
        {
            if (iterator->second.textureFilenames[textureIndex].size() > 0)
            {
                std::filesystem::path path(fbxFilename);
                path.replace_filename(iterator->second.textureFilenames[textureIndex]);
                D3D11_TEXTURE2D_DESC texture2dDesc;
                LoadTextureFromFile(device, path.c_str(), iterator->second.shaderResourceViews[textureIndex].GetAddressOf(), &texture2dDesc);
            }
            else
            {
                MakeDummyTexture(device, iterator->second.shaderResourceViews[textureIndex].GetAddressOf(),
                    textureIndex == 1 ? 0xFFFF7F7F : 0xFFFFFFFF, 16);
            }
        }
    }

    HRESULT hr = S_OK;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT},
        {"TANGENT",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
        {"WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
        {"BONES",0,DXGI_FORMAT_R32G32B32A32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT},
    };

    hr = CreateVsFromCSO(device, "./Shader/SkinnedMeshVS.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    hr = CreatePsFromCSO(device, "./Shader/SkinnedMeshPS.cso", pixelShader.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Constants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void SkinnedMesh::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor, const Animation::Keyframe* keyframe)
{
    using namespace DirectX;
    for (Mesh& mesh : meshes)
    {
        uint32_t stride{ sizeof(Vertex) };
        uint32_t offset{ 0 };

        immediateContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
        immediateContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

        Constants data;

        if (keyframe && keyframe->nodes.size() > 0)
        {
            const Animation::Keyframe::Node& meshNode{ keyframe->nodes.at(mesh.nodeIndex) };
            DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&meshNode.globalTransform) * DirectX::XMLoadFloat4x4(&world));

            const size_t boneCount{ mesh.bindPose.bones.size() };
            _ASSERT_EXPR(boneCount < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");

            for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
            {
                const Skelton::Bone& bone{ mesh.bindPose.bones.at(boneIndex) };
                const Animation::Keyframe::Node& boneNode{ keyframe->nodes.at(bone.nodeIndex) };
                DirectX::XMStoreFloat4x4(&data.boneTransforms[boneIndex],
                    DirectX::XMLoadFloat4x4(&bone.offsetTransform) *
                    DirectX::XMLoadFloat4x4(&boneNode.globalTransform) *
                    DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform)));
            }
        }
        else
        {
            DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform) * DirectX::XMLoadFloat4x4(&world));
            for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
            {
                data.boneTransforms[boneIndex] =
                {
                    1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1,
                };
            }
        }
        for (const Mesh::Subset& subset : mesh.subsets)
        {
            const Material& material{ materials.at(subset.materialUniqueId) };

            DirectX::XMStoreFloat4(&data.materialColor, DirectX::XMLoadFloat4(&materialColor) * DirectX::XMLoadFloat4(&material.Kd));
            //data.materialColor = materialColor;
            immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
            //シェーダーリソースビューをピクセルシェーダーにバインド
            //immediateContext->PSSetShaderResources(0, 1, materials.cbegin()->second.shaderResourceViews[0].GetAddressOf());
            immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());

            //法線マップのシェーダーリソースビューをバインド
            if (!material.textureFilenames[1].empty())
            {//TODO:01 pdf29⑦skinned_meshクラスのrenderメンバ関数で法線マップのシェーダーリソースビューをバインドする 
                immediateContext->PSSetShaderResources(1, 1, material.shaderResourceViews[1].GetAddressOf());
            }

            immediateContext->DrawIndexed(subset.indexCount, subset.startIndexLocation, 0);
        }

    }
}