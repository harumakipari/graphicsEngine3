#include "GltfModelBase.h"
#include <stack>
#include <functional>
#include <filesystem>

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/Shader.h"
#include "Texture.h"

// ノードの変換（スケール、回転、位置）を累積していく関数
void GltfModelBase::CumulateTransforms(std::vector<Node>& nodes)
{
    // 親ノードのグローバル変換行列を管理するスタック
    std::stack<DirectX::XMFLOAT4X4>  parentGlobalTransforms;

    // ノードツリーを再帰的に巡回して、変換を累積するラムダ関数
    std::function<void(int)> traverse{ [&](int nodeIndex)->void
        {
            // 現在のノードを取得
            Node& node{nodes.at(nodeIndex)};

            // ノードのスケール、回転、位置を行列に変換
            DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scale.x,node.scale.y,node.scale.z) };
            DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation.x,node.rotation.y,node.rotation.z,node.rotation.w)) };
            DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation.x,node.translation.y,node.translation.z) };

            // 親ノードの変換を掛け合わせてノードのグローバル変換行列を計算
            DirectX::XMStoreFloat4x4(&node.globalTransform, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
            
            // 子ノードがいれば再帰的に処理
            for (int childIndex : node.children)
            {
                // 子ノードを処理する前に親ノードのグローバル変換を保存
                parentGlobalTransforms.push(node.globalTransform);
                traverse(childIndex);// 子ノードを処理
                parentGlobalTransforms.pop(); // 子ノード処理後にスタックから削除
            }
        } };

    // シーンの最初のノードから処理を開始
    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        // 初期の親ノードのグローバル変換行列（単位行列）をスタックにプッシュ
        parentGlobalTransforms.push(
            {
                1,0,0,0,// X軸
                0,1,0,0,// Y軸
                0,0,1,0,// Z軸
                0,0,0,1 //平行移動なし
            });

        // ノードツリーを巡回して変換を累積
        traverse(nodeIndex);

        // 親ノードのグローバル変換をスタックからポップ
        parentGlobalTransforms.pop();
    }
}
