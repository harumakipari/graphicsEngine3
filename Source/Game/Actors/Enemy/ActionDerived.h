#pragma once

#include "ActionBase.h"
#include "RiderEnemy.h"
#include "Utils/EasingHandler.h"

//TODO:Behavior アクション追加

//  Root(p) 　 ー StartPerf（末端）
//			　 ー Damaged
//			　 ー Die	（末端）
//			　 ー Flinch（末端）（一旦やってない）
//			ー Change（末端）
//          ー Battle(p)
//             ー Special （末端）
//             ー Terrain （末端）
//             ー Pursuit （末端）
//             ー Attack(r)
//                ー Normal （末端）
//                ー Charge(s)
//					 ー Dash		（末端）
//					 ー Normal	（末端）
//                ー Bombing（末端）
//                ー Summon （末端）
//          ー Scout(p)
//             ー Idle  （末端）

// 最初の演出
class StartPerfAction :public ActionBase
{
public:
	StartPerfAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// ひるみ
class DamageAction:public ActionBase
{
public:
	DamageAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 状態変化
class ChangeAction :public ActionBase
{
public:
	ChangeAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 必殺技
class SpecialAction :public ActionBase
{
	EasingHandler handler;
	EasingHandler yEasing;
	float easeY = 0.0f;
public:
	SpecialAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

//地形変動
class TerrainAction :public ActionBase
{
public:
	TerrainAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 追跡行動
class PursuitAction :public ActionBase
{
public:
	PursuitAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 通常攻撃
class NormalAction :public ActionBase
{
public:
	NormalAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 突進行動
class DashAction :public ActionBase
{
public:
	DashAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 爆撃行動
class BombingAction :public ActionBase
{
public:
	BombingAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 召喚行動
class SummonAction :public ActionBase
{
public:
	SummonAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 待機行動
class IdleAction :public ActionBase
{
public:
	IdleAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// クールタイム中の追跡行動
class CoolPursuitAction :public ActionBase
{
public:
	CoolPursuitAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};
