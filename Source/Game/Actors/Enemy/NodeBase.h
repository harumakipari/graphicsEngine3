#pragma once

#include <vector>
#include <string>
#include "BehaviorTree.h"
#include "ActionBase.h"

class JudgmentBase;
class BehaviorData;

//���������[�N�����p
#define debug_new new(_NORMAL_BLOCK,__FILE__,__LINE__)

//�m�[�h
class NodeBase
{
public:
	//�R���X�g���N�^
	NodeBase(std::string name, NodeBase* parent, int priority,
		BehaviorTree::SelectRule selectRule, JudgmentBase* judgment, ActionBase* action) :
		name(name), parent(parent), priority(priority),
		selectRule(selectRule), judgment(judgment), action(action), children(NULL)
	{
	}
	//�f�X�g���N�^
	~NodeBase();
	// ���O�Q�b�^�[
	std::string GetName() { return name; }
	// �D�揇�ʃQ�b�^�[
	int GetPriority() { return priority; }
	// �q�m�[�h�ǉ�
	void AddChild(NodeBase* child) { children.push_back(child); }
	// �s���f�[�^�������Ă��邩
	bool HasAction() { return action != nullptr ? true : false; }
	// ���s�۔���
	bool Judgment();
	// �D�揇�ʑI��
	NodeBase* SelectPriority(std::vector<NodeBase*>* list);
	// �����_���I��
	NodeBase* SelectRandom(std::vector<NodeBase*>* list);
	// �V�[�P���X�I��
	NodeBase* SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data);
	// �m�[�h����
	NodeBase* SearchNode(std::string searchName);
	// �m�[�h���_
	NodeBase* Inference(BehaviorData* data);
	// ���s
	ActionBase::State Run(float elapsedTime);
	std::vector<NodeBase*>		children;		// �q�m�[�h
protected:
	std::string					name;		//���O
	BehaviorTree::SelectRule	selectRule;	//�I�����[��
	JudgmentBase* judgment;	//����N���X
	ActionBase* action;		//���s�N���X
	unsigned int				priority;	//�D�揇��
	NodeBase* parent;		//�e�m�[�h
};
