#pragma once

class RiderEnemy;

//���s����
class JudgmentBase
{
public:
	JudgmentBase(RiderEnemy* enemy) :owner(enemy) {}
	virtual bool Judgment() = 0;
protected:
	RiderEnemy* owner;
};
