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

//FBX�̍s���DirectX��XMFLOAT4x4�ɕϊ�����֐�
inline XMFLOAT4X4 ToXmfloat4x4(const FbxAMatrix& fbxamatrix)
{
    XMFLOAT4X4 xmfloat4x4;
    // 4x4�s��̗v�f��ϊ����ăR�s�[
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
        }
    }
    return xmfloat4x4;
}

// glTF�̉摜�ǂݍ��݊֐��B�摜�̓ǂݍ��݂��X�L�b�v����ݒ�
static bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
    return true;
}

// �R���X�g���N�^: FBX�܂���glTF�t�@�C����ǂݍ���
CollisionMesh::CollisionMesh(ID3D11Device* device, const std::string& filename, bool triangulate/*ignored*/)
{
    // FBX��glTF��I�����ēǂݍ���
    if (filename.find(".fbx") != std::string::npos)
    {// FBX�t�@�C���̏ꍇ
        FbxManager* fbxManager = FbxManager::Create();
        FbxScene* fbxScene = FbxScene::Create(fbxManager, "");
        FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
        bool importStatus = false;
        importStatus = fbxImporter->Initialize(filename.c_str());
        assert(importStatus && "Failed to call FbxImporter::Initialize");
        importStatus = fbxImporter->Import(fbxScene);
        assert(importStatus && "Failed to call FbxImporter::Import");

        FbxGeometryConverter fbxConverter(fbxManager);
        // �K�v�ɉ����ă��b�V�����O�p�`��
        if (triangulate)
        {
            fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
            fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);
        }

        // �m�[�h���ċA�I�ɏ���
        std::function<void(FbxNode*)> traverse = [&](FbxNode* fbxNode) {
            if (fbxNode->GetNodeAttribute() && fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
            {
                // ���b�V���f�[�^�̊i�[
                decltype(meshes)::reference mesh = meshes.emplace_back();
                mesh.name = fbxNode->GetName();

                // �m�[�h�̃O���[�o���g�����X�t�H�[��
                DirectX::XMFLOAT4X4 nodeTransform = ToXmfloat4x4(fbxNode->EvaluateGlobalTransform());
                XMMATRIX globalTransform = XMLoadFloat4x4(&nodeTransform);

                FbxMesh* fbxMesh = fbxNode->GetMesh();
                const int materialCount = fbxMesh->GetNode()->GetMaterialCount();

                // �T�u�Z�b�g�̃T�C�Y��ݒ�
                mesh.subsets.resize(materialCount > 0 ? materialCount : 1);
                // �}�e���A�������擾
                for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
                {
                    const FbxSurfaceMaterial* fbxMaterial = fbxMesh->GetNode()->GetMaterial(materialIndex);
                    mesh.subsets.at(materialIndex).materialName = fbxMaterial->GetName();
                }

                const FbxVector4* controlPoints = fbxMesh->GetControlPoints();
                const int polygonCount = fbxMesh->GetPolygonCount();
                // �|���S���f�[�^������
                for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
                {
                    const int materialIndex = materialCount > 0 ? fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0;
                    decltype(mesh.subsets)::reference subset = mesh.subsets.at(materialIndex);

                    for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
                    {
                        DirectX::XMFLOAT3 position;
                        // �e�|���S���̒��_���W���擾
                        const int polygonVertex = fbxMesh->GetPolygonVertex(polygonIndex, positionInPolygon);
                        position.x = static_cast<float>(controlPoints[polygonVertex][0]);
                        position.y = static_cast<float>(controlPoints[polygonVertex][1]);
                        position.z = static_cast<float>(controlPoints[polygonVertex][2]);
                        // �O���[�o���ϊ���K�p
                        XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), globalTransform));
                        subset.positions.emplace_back(position);
                    }
                }
                // �o�E���f�B���O�{�b�N�X�̍X�V
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
            // �q�m�[�h���ċA�I�ɏ���
            for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
            {
                traverse(fbxNode->GetChild(childIndex));
            }
            };
        // ���[�g�m�[�h���珈�����J�n
        traverse(fbxScene->GetRootNode());
        // FBX���\�[�X�����
        fbxManager->Destroy();
    }
    else
    {
        // glTF�t�@�C����ǂݍ��ޏ���
        tinygltf::TinyGLTF tinyGltf;
        tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

        tinygltf::Model gltfModel;
        std::string error, warning;
        bool succeeded{ false };
        // glTF�`���̃t�@�C����ǂݍ���
        if (filename.find(".glb") != std::string::npos)
        {
            succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
        }
        else if (filename.find(".gltf") != std::string::npos)
        {
            succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
        }
        assert(succeeded && "Failed to load glTF file");

        // glTF�m�[�h���ċA�I�ɏ���
        std::stack<DirectX::XMFLOAT4X4> parentGlobalTransforms;
        std::function<void(int)> traverse = [&](int nodeIndex)
            {
                // �m�[�h���̎擾
                decltype(gltfModel.nodes)::value_type gltfNode = gltfModel.nodes.at(nodeIndex);
                // �O���[�o���g�����X�t�H�[���̌v�Z
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
                    // �s��f�[�^��ϊ�
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
                    // �X�P�[���A��]�A���s�ړ��̓K�p
                    DirectX::XMMATRIX S = gltfNode.scale.size() > 0 ? DirectX::XMMatrixScaling(static_cast<float>(gltfNode.scale.at(0)), static_cast<float>(gltfNode.scale.at(1)), static_cast<float>(gltfNode.scale.at(2))) : DirectX::XMMatrixIdentity();
                    DirectX::XMMATRIX R = gltfNode.rotation.size() > 0 ? DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(static_cast<float>(gltfNode.rotation.at(0)), static_cast<float>(gltfNode.rotation.at(1)), static_cast<float>(gltfNode.rotation.at(2)), static_cast<float>(gltfNode.rotation.at(3)))) : DirectX::XMMatrixIdentity();
                    DirectX::XMMATRIX T = gltfNode.translation.size() > 0 ? DirectX::XMMatrixTranslation(static_cast<float>(gltfNode.translation.at(0)), static_cast<float>(gltfNode.translation.at(1)), static_cast<float>(gltfNode.translation.at(2))) : DirectX::XMMatrixIdentity();
                    DirectX::XMStoreFloat4x4(&globalTransform, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
                }
                // ���b�V��������΂��̃f�[�^���i�[
                if (gltfNode.mesh > -1)
                {
                    // glTF�̃��b�V�����擾
                    std::vector<tinygltf::Mesh>::const_reference gltfMesh = gltfModel.meshes.at(gltfNode.mesh);
                    decltype(meshes)::reference mesh = meshes.emplace_back();
                    mesh.name = gltfMesh.name;

                    // glTF�̃v���~�e�B�u������
                    for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
                    {
                        // �}�e���A�������擾
                        decltype(mesh.subsets)::reference subset = mesh.subsets.emplace_back();
                        if (gltfPrimitive.material > -1)
                        {
                            std::vector<tinygltf::Material>::const_reference gltfMaterial = gltfModel.materials.at(gltfPrimitive.material);
                            subset.materialName = gltfMaterial.name;
                        }

                        // ���_���W�̎擾
                        const tinygltf::Accessor& positionAccessor = gltfModel.accessors.at(gltfPrimitive.attributes.at("POSITION"));
                        const tinygltf::BufferView& positionBufferView = gltfModel.bufferViews.at(positionAccessor.bufferView);
                        DirectX::XMFLOAT3* positions = reinterpret_cast<DirectX::XMFLOAT3*>(gltfModel.buffers.at(positionBufferView.buffer).data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset);

                        // �C���f�b�N�X�o�b�t�@�̎擾
                        const tinygltf::Accessor& indexAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
                        const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews.at(indexAccessor.bufferView);


                        assert(indexAccessor.type == TINYGLTF_TYPE_SCALAR);
                        if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        {
                            unsigned int* indices = reinterpret_cast<unsigned int*>(gltfModel.buffers.at(indexBufferView.buffer).data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);

                            // ���_�f�[�^���i�[�i���[���h�ϊ���K�p�j
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

                            // ���_�f�[�^���i�[�i���[���h�ϊ���K�p�j
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

                // �q�m�[�h���ċA�I�ɏ���
                for (decltype(gltfNode.children)::value_type childIndex : gltfNode.children)
                {
                    parentGlobalTransforms.push(globalTransform);
                    traverse(childIndex);
                    parentGlobalTransforms.pop();
                }
            };

        // �V�[���̃��[�g�m�[�h������
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

    // AABB�i�o�E���f�B���O�{�b�N�X�j�̍Čv�Z
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

    // 3D��Ԃ̊e���iX, Y, Z�j���Ƃ�AABB�Ƃ̌���������s��
    for (size_t a = 0; a < 3; ++a)
    {
        float inv_d = 1.0f / d[a];// �t�����̃x�N�g�������߂�
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
    return true; // �������Ă���
}

// ���C�ƏՓ˂��郁�b�V���𔻒�// ���[���h���W�ɕϊ����Ă���n��
bool CollisionMesh::Raycast(_In_ XMFLOAT3 rayPosition, _In_ XMFLOAT3 rayDirection, _In_ const XMFLOAT4X4& transform, _Out_ XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
    _Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ float rayLengthLimit, _In_ bool skipIf) const
{
    // ���[���h���W���烂�f����Ԃ֕ϊ�
    XMMATRIX T = XMLoadFloat4x4(&transform);
    XMMATRIX _T = XMMatrixInverse(NULL, T);

    XMStoreFloat3(&rayPosition, XMVector3TransformCoord(XMLoadFloat3(&rayPosition), _T));
    XMStoreFloat3(&rayDirection, XMVector3TransformNormal(XMLoadFloat3(&rayDirection), _T));

    const XMVECTOR P = XMLoadFloat3(&rayPosition);// ���C�̊J�n�_
    const XMVECTOR D = XMVector3Normalize(XMLoadFloat3(&rayDirection));// ���C�̕����𐳋K��

    int intersectionCount = 0;
    float closestDistance = FLT_MAX;

    // �e���b�V�����`�F�b�N
    for (decltype(meshes)::const_reference mesh : meshes)
    {
#if 1
        // AABB�i���E�{�b�N�X�j�Ƃ̌�������
        const float* aabbMin = reinterpret_cast<const float*>(&mesh.boundingBox[0]);
        const float* aabbMax = reinterpret_cast<const float*>(&mesh.boundingBox[1]);
        if (!intersectRayAabb(reinterpret_cast<const float*>(&rayPosition), reinterpret_cast<const float*>(&rayDirection), aabbMin, aabbMax))
        {
            continue;// �������Ȃ��ꍇ�A���̃��b�V����
        }
#endif
        // ���b�V�����̊e�T�u�Z�b�g�i�|���S���Q�j���`�F�b�N
        for (decltype(mesh.subsets)::const_reference subset : mesh.subsets)
        {
            const XMFLOAT3* positions = subset.positions.data();
            const size_t triangleCount = subset.positions.size() / 3;

            for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
            {
                // �O�p�`�̊e���_���擾
                const XMVECTOR A = XMLoadFloat3(&positions[triangleIndex * 3 + 0]);
                const XMVECTOR B = XMLoadFloat3(&positions[triangleIndex * 3 + 1]);
                const XMVECTOR C = XMLoadFloat3(&positions[triangleIndex * 3 + 2]);

                // �O�p�`�̖@�����v�Z
                const XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A)));
                const float d = XMVectorGetByIndex(XMVector3Dot(N, A), 0);
                const float denominator{ XMVectorGetByIndex(XMVector3Dot(N, D), 0) };

                // ���C���O�p�`�̕��ʂƌ������邩����
                if (denominator < 0) // Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
                {
                    const float numerator = d - XMVectorGetByIndex(XMVector3Dot(N, P), 0);
                    const float t = numerator / denominator;

                    // ���C�̌������L���͈͓����`�F�b�N
                    if (t > 0 && t < rayLengthLimit) // Forward and Length limit of Ray
                    {
                        XMVECTOR Q = XMVectorAdd(P, XMVectorScale(D, t));// �����_

                        // �O�p�`���ɓ_�����邩�`�F�b�N
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

                        // �ł��߂������_���X�V
                        if (t < closestDistance)
                        {
                            closestDistance = t;

                            XMStoreFloat3(&intersectionPosition, XMVector3TransformCoord(Q, T));
                            XMStoreFloat3(&intersectionNormal, XMVector3Normalize(XMVector3TransformNormal(N, T)));
                            intersectionMesh = mesh.name;
                            intersectionMaterial = subset.materialName;

                            if (skipIf)// �ŏ��̌����_�ŏI��
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return intersectionCount > 0;// ��ȏ����������� true
}

