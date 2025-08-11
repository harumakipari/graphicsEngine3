#include "JudgmentDerived.h"
#include "EnemyMath.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Managers/GameManager.h"

//TODO:Behavior �s������ǉ�

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

// StartPerfNode�ɑJ�ڂł��邩����
bool StartPerfJudgment::Judgment()
{
	if (!GameManager::GetGameTimerStart())
	{
		return true;
	}
	return false;
}

// DamageNode�ɑJ�ڂł��邩����
bool DamageJudgment::Judgment()
{
	if (owner->GetIsStaggered())
	{
		return true;
	}
	return false;
}

// ChangeNode�ɑJ�ڂł��邩����
bool ChangeJudgment::Judgment()
{
	if (!owner->GetIsChange())
	{
		//HP��50%�ȉ��̎�
		if (owner->GetHP() < (owner->GetMaxHP() * 0.5f))
		{
			return true;
		}
	}
	return false;
}

// BattleNode�ɑJ�ڂł��邩����
bool BattleJudgment::Judgment()
{
	//�N�[���^�C��������Ȃ�������
	if (!owner->GetIsCoolTime())
	{
		return true;
	}
	return false;
}

// SpecialNode�ɑJ�ڂł��邩����
bool SpecialJudgment::Judgment()
{
	//�K�E�Q�[�W��100%�ɂȂ�����
	if (owner->GetCurrentSpecialGauge() >= (owner->GetMaxSpecialGauge()))
	{
		return true;
	}
	return false;
}

// TerrainNode�ɑJ�ڂł��邩����
bool TerrainJudgment::Judgment()
{
	//�n�`�ϓ��̃N�[���^�C���ő�l�𒴂�����
	if (owner->GetTerrainTime() > owner->GetMaxTerrainTime())
	{
		owner->SetTerrainTime(0.0f);
		owner->SetCanTerrain(true);
		return true;
	}
	return false;
}

// PursuitNode�ɑJ�ڂł��邩����
bool PursuitJudgment::Judgment()
{
	//�U���͈͊O��������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist > owner->GetAttackRange())
	{
		return true;
	}
	return false;
}

// AttackNode�ɑJ�ڂł��邩����
bool AttackJudgment::Judgment()
{
	//�U���͈͂ɓ�������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < owner->GetAttackRange())
	{
		return true;
	}
	return false;
}

// NormalNode�ɑJ�ڂł��邩����
bool NormalJudgment::Judgment()
{
	//�U���͈͓��{������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < owner->GetDistNear())
	{
		return true;
	}
	return false;
}

// ChargeNode�ɑJ�ڂł��邩����
bool ChargeJudgment::Judgment()
{
	//�U���͈͓��{������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < owner->GetDistFar())
	{
		return true;
	}
	return false;
}

// SummonNode�ɑJ�ڂł��邩����
bool SummonJudgment::Judgment()
{
	//�����N�[���^�C�����ő�l�𒴂�����
	if (owner->GetSummonTime() > owner->GetMaxSummonTime())
	{
		owner->SetCanSummon(true);
		return true;
	}
	return false;
}

// CoolIdleNode�ɑJ�ڂł��邩����
bool CoolIdleJudgment::Judgment()
{
	//�������߂�������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < owner->GetDistMid())
	{
		return true;
	}
	return false;
}

// CoolPursuitNode�ɑJ�ڂł��邩����
bool CoolPursuitJudgment::Judgment()
{
	//����������������
	DirectX::XMFLOAT3 position = owner->GetPosition();
	auto player = std::dynamic_pointer_cast<Player>(owner->GetOwnerScene()->GetActorManager()->GetActorByName("actor"));
	DirectX::XMFLOAT3 targetPosition = player->GetPosition();

	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist > owner->GetDistMid())
	{
		return true;
	}
	return false;
}
