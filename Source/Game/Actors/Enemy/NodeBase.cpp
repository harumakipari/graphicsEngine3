#include "JudgmentBase.h"
#include "NodeBase.h"
#include "RiderEnemy.h"
#include "BehaviorData.h"
#include "ActionBase.h"
#include "EnemyMath.h"

// �f�X�g���N�^
NodeBase::~NodeBase()
{
	delete judgment;
	delete action;
}

// �m�[�h����
NodeBase* NodeBase::SearchNode(std::string searchName)
{
	// ���O����v
	if (name == searchName)
	{
		return this;
	}
	else {
		// �q�m�[�h�Ō���
		for (auto itr = children.begin(); itr != children.end(); itr++)
		{
			NodeBase* ret = (*itr)->SearchNode(searchName);

			if (ret != nullptr)
			{
				return ret;
			}
		}
	}

	return nullptr;
}

// �m�[�h���_
NodeBase* NodeBase::Inference(BehaviorData* data)
{
	std::vector<NodeBase*> list;
	NodeBase* result = nullptr;

	// children�̐��������[�v���s��
	for (int i = 0; i < children.size(); i++)
	{
		// children.at(i)->judgment��nullptr�łȂ����
		if (children.at(i)->judgment != nullptr)
		{
			// children.at(i)->judgment->Judgment()�֐������s���Ature�ł����
			// list��children.at(i)��ǉ����Ă���
			if (children.at(i)->judgment->Judgment())
			{
				list.emplace_back(children.at(i));
			}
		}
		else {
			// ����N���X���Ȃ���Ζ������ɒǉ�
			list.emplace_back(children.at(i));
		}
	}

	// �I�����[���Ńm�[�h����
	switch (selectRule)
	{
		// �D�揇��
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(&list);
		break;
		// �����_��
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(&list);
		break;
		// �V�[�P���X
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(&list, data);
		break;
	}

	if (result != nullptr)
	{
		// �s��������ΏI��
		if (result->HasAction() == true)
		{
			return result;
		}
		else {
			// ���܂����m�[�h�Ő��_�J�n
			result = result->Inference(data);
		}
	}

	return result;
}

// �D�揇�ʂŃm�[�h�I��
NodeBase* NodeBase::SelectPriority(std::vector<NodeBase*>* list)
{
	NodeBase* selectNode = nullptr;
	int priority = INT_MAX;

	// ��ԗD�揇�ʂ������m�[�h��T����selectNode�Ɋi�[
	for (auto node : *list)
	{
		int nodePriority = node->GetPriority();

		if (nodePriority < priority)
		{
			priority = nodePriority;
			selectNode = node;
		}
	}

	return selectNode;
}


// �����_���Ńm�[�h�I��
NodeBase* NodeBase::SelectRandom(std::vector<NodeBase*>* list)
{
	int selectNo = 0;
	// list�̃T�C�Y�ŗ������擾����selectNo�Ɋi�[

	// Mathf::RandomRange ���g���� 0 �` list->size() �͈̔͂Ń����_���� float ���擾
	float randomFloat = Mathf::RandomRange(0.0f, static_cast<float>(list->size()));

	// �����_��؂�̂ĂăC���f�b�N�X�ɕϊ��i��F3.7 �� 3�j
	selectNo = static_cast<int>(randomFloat);

	// �C���f�b�N�X���͈͊O�ɂȂ�Ȃ��悤�ɒ����i�O�̂��߁j
	if (selectNo >= list->size()) selectNo = static_cast<int>(list->size()) - 1;

	// list��selectNo�Ԗڂ̎��Ԃ����^�[��
	return (*list).at(selectNo);
}

// �V�[�P���X�E�V�[�P���V�������[�s���O�Ńm�[�h�I��
NodeBase* NodeBase::SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data)
{
	int step = 0;

	// �w�肳��Ă��钆�ԃm�[�h�̃V�[�P���X���ǂ��܂Ŏ��s���ꂽ���擾����
	step = data->GetSequenceStep(name);

	// ���ԃm�[�h�ɓo�^����Ă���m�[�h���ȏ�̏ꍇ�A
	if (step >= children.size())
	{
		// ���[���ɂ���ď�����؂�ւ���
		// ���[����BehaviorTree::SelectRule::SequentialLooping�̂Ƃ��͍ŏ�������s���邽�߁Astep��0����
		// ���[����BehaviorTree::SelectRule::Sequence�̂Ƃ��͎��Ɏ��s�ł���m�[�h���Ȃ����߁Anullptr�����^�[��
		if (step == static_cast<int>(BehaviorTree::SelectRule::SequentialLooping))
		{
			step = 0;
		}
		else if (step == static_cast<int>(BehaviorTree::SelectRule::Sequence))
		{
			return nullptr;
		}
	}
	// ���s�\���X�g�ɓo�^����Ă���f�[�^�̐��������[�v���s��
	for (auto itr = list->begin(); itr != list->end(); itr++)
	{
		// �q�m�[�h�����s�\���X�g�Ɋ܂܂�Ă��邩
		if (children.at(step)->GetName() == (*itr)->GetName())
		{
			// ���݂̎��s�m�[�h�̕ۑ��A���Ɏ��s����X�e�b�v�̕ۑ����s������A
			// ���݂̃X�e�b�v�ԍ��̃m�[�h�����^�[������

			// �@�X�^�b�N�ɂ�data->PushSequenceNode�֐����g�p����B�ۑ�����f�[�^�͎��s���̒��ԃm�[�h
			data->PushSequenceNode(this);

			// �A�܂��A���Ɏ��s���钆�ԃm�[�h�ƃX�e�b�v����ۑ�����
			// �@�ۑ��ɂ�data->SetSequenceStep�֐����g�p�B
			// �@�ۑ��f�[�^�͒��ԃm�[�h�̖��O�Ǝ��̃X�e�b�v���ł�(step + 1)
			data->SetSequenceStep(name, step + 1);

			// �B�X�e�b�v�ԍ��ڂ̎q�m�[�h�����s�m�[�h�Ƃ��ă��^�[������
			return children.at(step);
		}
	}
	// �w�肳�ꂽ���ԃm�[�h�Ɏ��s�\�m�[�h���Ȃ��̂�nullptr�����^�[������
	return nullptr;
}

// ����
bool NodeBase::Judgment()
{
	// judgment�����邩���f�B����΃����o�֐�Judgment()���s�������ʂ����^�[��
	if (judgment)
	{
		return judgment->Judgment();
	}
	else
	{
		return true;
	}
}

// �m�[�h���s
ActionBase::State NodeBase::Run(float elapsedTime)
{
	// action�����邩���f�B����΃����o�֐�Run()���s�������ʂ����^�[��
	if (action)
	{
		return action->Run(elapsedTime);
	}
	else
	{
		return ActionBase::State::Failed;
	}
}
