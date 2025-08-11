#include "BehaviorTree.h"
#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "RiderEnemy.h"
#include "BehaviorData.h"

//�f�X�g���N�^
BehaviorTree::~BehaviorTree()
{
	NodeAllClear(root);
}

void BehaviorTree::AddNode(std::string parentName, std::string entryName, int priority, SelectRule selectRule, JudgmentBase* judgment, ActionBase* action)
{
	if (parentName != "")
	{
		NodeBase* parentNode = root->SearchNode(parentName);

		if (parentNode != nullptr)
		{
			NodeBase* addNode = new NodeBase(entryName, parentNode, priority, selectRule, judgment, action);
			parentNode->AddChild(addNode);
		}
	}
	else {
		if (root == nullptr)
		{
			root = new NodeBase(entryName, nullptr, priority, selectRule, judgment, action);
		}
	}
}

//���s�m�[�h�𐄘_����
NodeBase* BehaviorTree::ActiveNodeInference(BehaviorData* data)
{
	// �f�[�^�����Z�b�g���ĊJ�n
	data->Init();
	return root->Inference(data);
}

//�V�[�P���X�m�[�h���琄�_�J�n
NodeBase* BehaviorTree::SequenceBack(NodeBase* sequenceNode, BehaviorData* data)
{
	return sequenceNode->Inference(data);
}

//�m�[�h���s
NodeBase* BehaviorTree::Run(NodeBase* actionNode, BehaviorData* data, float elapsedTime)
{
	// �m�[�h���s
	ActionBase::State state = actionNode->Run(elapsedTime);

	// ����I��
	if (state == ActionBase::State::Complete)
	{
		// �V�[�P���X�̓r�����𔻒f
		NodeBase* sequenceNode = data->PopSequenceNode();

		// �r������Ȃ��Ȃ�I��
		if (sequenceNode == nullptr)
		{
			return nullptr;
		}
		else {
			// �r���Ȃ炻������n�߂�
			return SequenceBack(sequenceNode, data);
		}
	}
	else if (state == ActionBase::State::Failed) {
		// ���s�͏I��
		return nullptr;
	}

	// ����ێ�
	return actionNode;
}

//�o�^���ꂽ�m�[�h�����ׂč폜����
void BehaviorTree::NodeAllClear(NodeBase* delNode)
{
	size_t count = delNode->children.size();
	if (count > 0)
	{
		for (NodeBase* node : delNode->children)
		{
			NodeAllClear(node);
		}
		delete delNode;
	}
	else
	{
		delete delNode;
	}
}
