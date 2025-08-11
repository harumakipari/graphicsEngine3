#pragma once

#include "JudgmentBase.h"
#include "RiderEnemy.h"

//TODO:Behavior s“®”»’è’Ç‰Á

// StartPerfNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class StartPerfJudgment :public JudgmentBase
{
public:
	StartPerfJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// DamageNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class DamageJudgment:public JudgmentBase
{
public:
	DamageJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// ChangeNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class ChangeJudgment :public JudgmentBase
{
public:
	ChangeJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// BattleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class BattleJudgment :public JudgmentBase
{
public:
	BattleJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// SpecialNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class SpecialJudgment :public JudgmentBase
{
public:
	SpecialJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// TerrainNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class TerrainJudgment :public JudgmentBase
{
public:
	TerrainJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// PursuitNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class PursuitJudgment :public JudgmentBase
{
public:
	PursuitJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// AttackNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class AttackJudgment :public JudgmentBase
{
public:
	AttackJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// NormalNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class NormalJudgment :public JudgmentBase
{
public:
	NormalJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// ChargeNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class ChargeJudgment :public JudgmentBase
{
public:
	ChargeJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

// SummonNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class SummonJudgment :public JudgmentBase
{
public:
	SummonJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

class CoolIdleJudgment :public JudgmentBase
{
public:
	CoolIdleJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};

class CoolPursuitJudgment :public JudgmentBase
{
public:
	CoolPursuitJudgment(RiderEnemy* enemy) :JudgmentBase(enemy) {};
	//”»’è
	bool Judgment();
};
