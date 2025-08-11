#pragma once

#include "JudgmentBase.h"
#include "RiderEnemy.h"

//TODO:Behavior �s������ǉ�

// StartPerfNode�ɑJ�ڂł��邩����
class StartPerfJudgment :public JudgmentBase
{
public:
	StartPerfJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// DamageNode�ɑJ�ڂł��邩����
class DamageJudgment:public JudgmentBase
{
public:
	DamageJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// ChangeNode�ɑJ�ڂł��邩����
class ChangeJudgment :public JudgmentBase
{
public:
	ChangeJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// BattleNode�ɑJ�ڂł��邩����
class BattleJudgment :public JudgmentBase
{
public:
	BattleJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// SpecialNode�ɑJ�ڂł��邩����
class SpecialJudgment :public JudgmentBase
{
public:
	SpecialJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// TerrainNode�ɑJ�ڂł��邩����
class TerrainJudgment :public JudgmentBase
{
public:
	TerrainJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// PursuitNode�ɑJ�ڂł��邩����
class PursuitJudgment :public JudgmentBase
{
public:
	PursuitJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// AttackNode�ɑJ�ڂł��邩����
class AttackJudgment :public JudgmentBase
{
public:
	AttackJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// NormalNode�ɑJ�ڂł��邩����
class NormalJudgment :public JudgmentBase
{
public:
	NormalJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// ChargeNode�ɑJ�ڂł��邩����
class ChargeJudgment :public JudgmentBase
{
public:
	ChargeJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

// SummonNode�ɑJ�ڂł��邩����
class SummonJudgment :public JudgmentBase
{
public:
	SummonJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

class CoolIdleJudgment :public JudgmentBase
{
public:
	CoolIdleJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};

class CoolPursuitJudgment :public JudgmentBase
{
public:
	CoolPursuitJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//����
	bool Judgment();
};
