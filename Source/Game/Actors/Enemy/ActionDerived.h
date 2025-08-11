#pragma once

#include "ActionBase.h"
#include "RiderEnemy.h"
#include "Utils/EasingHandler.h"

//TODO:Behavior �A�N�V�����ǉ�

//  Root(p) �@ �[ StartPerf�i���[�j
//			�@ �[ Damaged
//			�@ �[ Die	�i���[�j
//			�@ �[ Flinch�i���[�j�i��U����ĂȂ��j
//			�[ Change�i���[�j
//          �[ Battle(p)
//             �[ Special �i���[�j
//             �[ Terrain �i���[�j
//             �[ Pursuit �i���[�j
//             �[ Attack(r)
//                �[ Normal �i���[�j
//                �[ Charge(s)
//					 �[ Dash		�i���[�j
//					 �[ Normal	�i���[�j
//                �[ Bombing�i���[�j
//                �[ Summon �i���[�j
//          �[ Scout(p)
//             �[ Idle  �i���[�j

// �ŏ��̉��o
class StartPerfAction :public ActionBase
{
public:
	StartPerfAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �Ђ��
class DamageAction:public ActionBase
{
public:
	DamageAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// ��ԕω�
class ChangeAction :public ActionBase
{
public:
	ChangeAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �K�E�Z
class SpecialAction :public ActionBase
{
	EasingHandler handler;
	EasingHandler yEasing;
	float easeY = 0.0f;
public:
	SpecialAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

//�n�`�ϓ�
class TerrainAction :public ActionBase
{
public:
	TerrainAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ǐՍs��
class PursuitAction :public ActionBase
{
public:
	PursuitAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ʏ�U��
class NormalAction :public ActionBase
{
public:
	NormalAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ːi�s��
class DashAction :public ActionBase
{
public:
	DashAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �����s��
class BombingAction :public ActionBase
{
public:
	BombingAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �����s��
class SummonAction :public ActionBase
{
public:
	SummonAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ҋ@�s��
class IdleAction :public ActionBase
{
public:
	IdleAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �N�[���^�C�����̒ǐՍs��
class CoolPursuitAction :public ActionBase
{
public:
	CoolPursuitAction(RiderEnemy* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};
