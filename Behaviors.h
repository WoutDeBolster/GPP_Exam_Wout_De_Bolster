/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "Behaviors.h"
#include "EBehaviorTree.h"
#include "SteeringBehaviors.h"
#include "EBlackboard.h"
#include "IExamInterface.h"
#include "EliteMath/EMath.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsHouseInsideFOV(Elite::Blackboard* pBlackboard)
{
	vector<HouseInfo>* pVHouseInfo{};
	IExamInterface* pInterface{};
	Elite::Vector2 target{};
	AgentInfo* pAgent{};
	HouseInfo* currentHouse{};
	auto dataAvailable = pBlackboard->GetData("Houses", pVHouseInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Target", target) &&
		pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable)
		return false;

	if (pAgent->IsInHouse)
		return false;

	// looking for the closed house
	float distance = FLT_MAX;
	for (HouseInfo& info : *pVHouseInfo)
	{
		float houseDistance = Distance(pAgent->Position, info.Center);

		if (houseDistance < distance)
		{
			distance = houseDistance;
			target = info.Center;
			currentHouse = &info;
		}
	}

	// if there is a house around set it to the target value
	if (distance != FLT_MAX)
	{
		pBlackboard->ChangeData("Target", target);
		pBlackboard->ChangeData("ClosestHouse", currentHouse);
		return true;
	}
	return false;
}

bool EntitieInsiteFOV(Elite::Blackboard* pBlackboard)
{
	vector<EntityInfo>* pVEntetyInfo{};
	IExamInterface* pInterface{};
	Elite::Vector2 target{};
	AgentInfo* pAgent{};
	EntityInfo currentEntity{};
	auto dataAvailable = pBlackboard->GetData("Entities", pVEntetyInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Target", target) &&
		pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable)
		return false;

	if (pAgent->IsInHouse)
		return false;

	// looking for the closed house
	float distance = FLT_MAX;
	for (const EntityInfo& info : *pVEntetyInfo)
	{
		float EntityDistance = Distance(pAgent->Position, info.Location);

		if (EntityDistance < distance)
		{
			distance = EntityDistance;
			target = info.Location;
			currentEntity = info;
		}
	}

	// if there is a house around set it to the target value
	if (distance != FLT_MAX)
	{
		PurgeZoneInfo ClosestPurgeZone{};
		EnemyInfo ClosestEnemy{};
		ItemInfo ClosestItem{};

		switch (currentEntity.Type)
		{
		case eEntityType::PURGEZONE:
			pInterface->PurgeZone_GetInfo(currentEntity, ClosestPurgeZone);
			pBlackboard->ChangeData("ClosestPurgeZone", static_cast<PurgeZoneInfo>(ClosestPurgeZone));
			break;
		case eEntityType::ENEMY:
			pInterface->Enemy_GetInfo(currentEntity, ClosestEnemy);
			pBlackboard->ChangeData("ClosestEnemy", static_cast<EnemyInfo>(ClosestEnemy));
			break;
		case eEntityType::ITEM:
			pInterface->Item_GetInfo(currentEntity, ClosestItem);
			pBlackboard->ChangeData("ClosestEnemy", static_cast<ItemInfo>(ClosestItem));
			break;
		default:
			std::cout << "Unown entity" << std::endl;
			break;
		}
		pBlackboard->ChangeData("Target", target);
		return true;
	}
	return false;
}

bool IsEnemyClose(Elite::Blackboard* pBlackboard)
{
	Elite::Vector2 target{};
	AgentInfo* pAgent{};
	EnemyInfo closestEnemy{};
	auto dataAvailable = pBlackboard->GetData("ClosestEnemy", closestEnemy) &&
		pBlackboard->GetData("Target", target) &&
		pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable)
		return false;

	if (pAgent->IsInHouse)
		return false;

	const float DangerRadius{ 10.f };
	if (DistanceSquared(pAgent->Position, closestEnemy.Location) < (DangerRadius * DangerRadius) &&
		(closestEnemy.Size / 2.f) > (pAgent->AgentSize / 2.f))
	{
		pBlackboard->ChangeData("Target", closestEnemy.Location);
		return true;
	}

	return false;
}

bool IsItemClose(Elite::Blackboard* pBlackboard)
{
	Elite::Vector2 target{};
	AgentInfo* pAgent{};
	ItemInfo closestItem{};
	auto dataAvailable = pBlackboard->GetData("ClosestItem", closestItem) &&
		pBlackboard->GetData("Target", target) &&
		pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable)
		return false;

	if (pAgent->IsInHouse)
		return false;

	const float DangerRadius{ 10.f };
	if (DistanceSquared(pAgent->Position, closestItem.Location) < (DangerRadius * DangerRadius))
	{
		pBlackboard->ChangeData("Target", closestItem.Location);
		return true;
	}

	return false;
}

bool IsPurgeZoneClose(Elite::Blackboard* pBlackboard)
{
	Elite::Vector2 target{};
	AgentInfo* pAgent{};
	PurgeZoneInfo closestZone{};
	auto dataAvailable = pBlackboard->GetData("ClosestPurgeZone", closestZone) &&
		pBlackboard->GetData("Target", target) &&
		pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable)
		return false;

	if (pAgent->IsInHouse)
		return false;

	const float DangerRadius{ 10.f };
	if (DistanceSquared(pAgent->Position, closestZone.Center) < (DangerRadius * DangerRadius) &&
		closestZone.Radius > (pAgent->AgentSize / 2.f))
	{
		pBlackboard->ChangeData("Target", closestZone.Center);
		return true;
	}

	return false;
}

//bool IsBiggerEnemyClose(Elite::Blackboard* pBlackboard)
//{
//	AgarioAgent* pAgent = nullptr;
//	std::vector<AgarioAgent*>* AgentsVec = nullptr;
//	const float DangerRadius{ 15.f };
//
//	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
//		pBlackboard->GetData("AgentsVec", AgentsVec);
//
//	auto enemyIt = std::find_if(AgentsVec->begin(), AgentsVec->end(), [&pAgent, &DangerRadius](AgarioAgent* enemy)
//		{
//			return DistanceSquared(pAgent->GetPosition(), enemy->GetPosition()) < (DangerRadius * DangerRadius) && enemy->GetRadius() > pAgent->GetRadius();
//		});
//
//	if (enemyIt != AgentsVec->end())
//	{
//		pBlackboard->ChangeData("Target", (*enemyIt)->GetPosition());
//		return true;
//	}
//
//	return false;
//}

Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pSeek = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Seek", pSeek) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pSeek->SetTargetPos(seekTarget);
	*ppSteering = pSeek;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pWander = nullptr;
	ISteeringBehavior* pScouting = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	ISteeringBehavior** ppAngular = nullptr;
	auto dataAvailable = pBlackboard->GetData("Wander", pWander) &&
		pBlackboard->GetData("Scouting", pScouting) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Angular", ppAngular);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	*ppSteering = pWander;
	*ppAngular = pScouting;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pFlee = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 FleeTarget{};
	auto dataAvailable = pBlackboard->GetData("Flee", pFlee) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Target", FleeTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pFlee->SetTargetPos(FleeTarget);
	*ppSteering = pFlee;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToArrive(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pArrive = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 ArriveTarget{};
	auto dataAvailable = pBlackboard->GetData("Arrive", pArrive) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Target", ArriveTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pArrive->SetTargetPos(ArriveTarget);
	*ppSteering = pArrive;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToFace(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pFace = nullptr;
	ISteeringBehavior** ppAngular = nullptr;
	Elite::Vector2 FaceTarget{};
	auto dataAvailable = pBlackboard->GetData("Face", pFace) &&
		pBlackboard->GetData("Angular", ppAngular) &&
		pBlackboard->GetData("Target", FaceTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pFace->SetTargetPos(FaceTarget);
	*ppAngular = pFace;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToEvade(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pEvade = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 EvadeTarget{};
	auto dataAvailable = pBlackboard->GetData("Evade", pEvade) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Target", EvadeTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pEvade->SetTargetPos(EvadeTarget);
	*ppSteering = pEvade;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToPursuit(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pPursuit = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 PursuitTarget{};
	auto dataAvailable = pBlackboard->GetData("Pursuit", pPursuit) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("Target", PursuitTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pPursuit->SetTargetPos(PursuitTarget);
	*ppSteering = pPursuit;

	return Elite::BehaviorState::Success;
}
#endif