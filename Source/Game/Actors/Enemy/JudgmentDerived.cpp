#include "JudgmentDerived.h"
#include "EnemyMath.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Managers/GameManager.h"

//TODO:Behavior 行動判定追加

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

// StartPerfNodeに遷移できるか判定
bool StartPerfJudgment::Judgment()
{
	if (!GameManager::GetGameTimerStart())
	{
		return true;
	}
	return false;
}

// DamageNodeに遷移できるか判定
bool DamageJudgment::Judgment()
{
	if (owner->GetIsStaggered())
	{
		return true;
	}
	return false;
}

// ChangeNodeに遷移できるか判定
bool ChangeJudgment::Judgment()
{
	if (!owner->GetIsChange())
	{
		//HPが50%以下の時
		if (owner->GetHP() < (owner->GetMaxHP() * 0.5f))
		{
			return true;
		}
	}
	return false;
}

// BattleNodeに遷移できるか判定
bool BattleJudgment::Judgment()
{
	//クールタイム中じゃなかったら
	if (!owner->GetIsCoolTime())
	{
		return true;
	}
	return false;
}

// SpecialNodeに遷移できるか判定
bool SpecialJudgment::Judgment()
{
	//必殺ゲージが100%になったら
	if (owner->GetCurrentSpecialGauge() >= (owner->GetMaxSpecialGauge()))
	{
		return true;
	}
	return false;
}

// TerrainNodeに遷移できるか判定
bool TerrainJudgment::Judgment()
{
	//地形変動のクールタイム最大値を超えたら
	if (owner->GetTerrainTime() > owner->GetMaxTerrainTime())
	{
		owner->SetTerrainTime(0.0f);
		owner->SetCanTerrain(true);
		return true;
	}
	return false;
}

// PursuitNodeに遷移できるか判定
bool PursuitJudgment::Judgment()
{
	//攻撃範囲外だったら
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

// AttackNodeに遷移できるか判定
bool AttackJudgment::Judgment()
{
	//攻撃範囲に入ったら
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

// NormalNodeに遷移できるか判定
bool NormalJudgment::Judgment()
{
	//攻撃範囲内＋距離小
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

// ChargeNodeに遷移できるか判定
bool ChargeJudgment::Judgment()
{
	//攻撃範囲内＋距離大
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

// SummonNodeに遷移できるか判定
bool SummonJudgment::Judgment()
{
	//召喚クールタイムが最大値を超えたら
	if (owner->GetSummonTime() > owner->GetMaxSummonTime())
	{
		owner->SetCanSummon(true);
		return true;
	}
	return false;
}

// CoolIdleNodeに遷移できるか判定
bool CoolIdleJudgment::Judgment()
{
	//距離が近かったら
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

// CoolPursuitNodeに遷移できるか判定
bool CoolPursuitJudgment::Judgment()
{
	//距離が遠かったら
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
