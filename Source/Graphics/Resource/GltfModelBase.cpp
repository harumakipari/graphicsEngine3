#include "GltfModelBase.h"
#include <stack>
#include <functional>
#include <filesystem>

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/Shader.h"
#include "Texture.h"

// �m�[�h�̕ϊ��i�X�P�[���A��]�A�ʒu�j��ݐς��Ă����֐�
void GltfModelBase::CumulateTransforms(std::vector<Node>& nodes)
{
    // �e�m�[�h�̃O���[�o���ϊ��s����Ǘ�����X�^�b�N
    std::stack<DirectX::XMFLOAT4X4>  parentGlobalTransforms;

    // �m�[�h�c���[���ċA�I�ɏ��񂵂āA�ϊ���ݐς��郉���_�֐�
    std::function<void(int)> traverse{ [&](int nodeIndex)->void
        {
            // ���݂̃m�[�h���擾
            Node& node{nodes.at(nodeIndex)};

            // �m�[�h�̃X�P�[���A��]�A�ʒu���s��ɕϊ�
            DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scale.x,node.scale.y,node.scale.z) };
            DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation.x,node.rotation.y,node.rotation.z,node.rotation.w)) };
            DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation.x,node.translation.y,node.translation.z) };

            // �e�m�[�h�̕ϊ����|�����킹�ăm�[�h�̃O���[�o���ϊ��s����v�Z
            DirectX::XMStoreFloat4x4(&node.globalTransform, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
            
            // �q�m�[�h������΍ċA�I�ɏ���
            for (int childIndex : node.children)
            {
                // �q�m�[�h����������O�ɐe�m�[�h�̃O���[�o���ϊ���ۑ�
                parentGlobalTransforms.push(node.globalTransform);
                traverse(childIndex);// �q�m�[�h������
                parentGlobalTransforms.pop(); // �q�m�[�h������ɃX�^�b�N����폜
            }
        } };

    // �V�[���̍ŏ��̃m�[�h���珈�����J�n
    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        // �����̐e�m�[�h�̃O���[�o���ϊ��s��i�P�ʍs��j���X�^�b�N�Ƀv�b�V��
        parentGlobalTransforms.push(
            {
                1,0,0,0,// X��
                0,1,0,0,// Y��
                0,0,1,0,// Z��
                0,0,0,1 //���s�ړ��Ȃ�
            });

        // �m�[�h�c���[�����񂵂ĕϊ���ݐ�
        traverse(nodeIndex);

        // �e�m�[�h�̃O���[�o���ϊ����X�^�b�N����|�b�v
        parentGlobalTransforms.pop();
    }
}
