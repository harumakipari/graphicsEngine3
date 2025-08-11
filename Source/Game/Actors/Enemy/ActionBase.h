#pragma once

class RiderEnemy;

//�s���������N���X
class ActionBase
{
public:
	ActionBase(RiderEnemy* enemy) :owner(enemy) {}
	//���s���
	enum class State
	{
		Run,		//���s��
		Failed,		//���s���s
		Complete,	//���s����
	};

	//���s����(�������z�֐�)
	virtual ActionBase::State Run(float elapsedTime) = 0;
protected:
	RiderEnemy* owner;
	int step = 0;
};
