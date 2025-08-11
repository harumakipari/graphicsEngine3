#pragma once

class RiderEnemy;

//Às”»’è
class JudgmentBase
{
public:
	JudgmentBase(RiderEnemy* enemy) :owner(enemy) {}
	virtual bool Judgment() = 0;
protected:
	RiderEnemy* owner;
};
