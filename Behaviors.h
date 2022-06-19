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

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

// CONDITIONALS
//-------------

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
			pBlackboard->ChangeData("ClosestItem", static_cast<ItemInfo>(ClosestItem));
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

// use items
bool shouldUseMedkit(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (pAgent->Health <= 5.f)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a medkit in inverntory
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::MEDKIT)
			{
				return Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

bool shouldUseFood(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (pAgent->Stamina <= 5.f)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a medkit in inverntory
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::FOOD)
			{
				return Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

// purgeZone
bool InPurgeZone(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};
	vector<EntityInfo>* pVEntetyInfo{};
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Entities", pVEntetyInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	PurgeZoneInfo zoneInfo;
	for (auto& e : *pVEntetyInfo)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			pInterface->PurgeZone_GetInfo(e, zoneInfo);
		}
	}

	const float DangerRadius{ zoneInfo.Radius };
	if (DistanceSquared(pAgent->Position, zoneInfo.Center) < (DangerRadius * DangerRadius))
	{
		pBlackboard->ChangeData("fleeTarget", zoneInfo.Center);
		return Elite::BehaviorState::Success;
	}

	return Elite::BehaviorState::Failure;
}

// enemy
bool AgentBittenHasStamina(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (!pAgent->Bitten)
	{
		return Elite::BehaviorState::Failure;
	}
	if (pAgent->Stamina < 0.1f)
	{
		return Elite::BehaviorState::Failure;
	}

	return Elite::BehaviorState::Success;
}

bool EnemyInFOV(Elite::Blackboard* pBlackboard)
{
	vector<EntityInfo>* pVEntetyInfo{};

	auto dataAvailable = pBlackboard->GetData("Entities", pVEntetyInfo);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	for (const EntityInfo& entity : *pVEntetyInfo)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			pBlackboard->ChangeData("EnemyTarget", entity);
			Elite::BehaviorState::Success;
		}
	}

	Elite::BehaviorState::Failure;
}

bool HasStamina(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (pAgent->Stamina < 0.1f)
	{
		return Elite::BehaviorState::Failure;
	}

	return Elite::BehaviorState::Success;
}

// not done
bool canHitEnemy(Elite::Blackboard* pBlackboard)
{
	vector<EntityInfo>* pVEntetyInfo{};
	IExamInterface* pInterface{};
	AgentInfo* pAgent{};

	auto dataAvailable = pBlackboard->GetData("Entities", pVEntetyInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a pistol in inverntory
	bool hasGun{};
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::PISTOL)
			{
				hasGun = true;
			}
		}
	}

	// facing enemy????

	return Elite::BehaviorState::Failure;
}

// get items
bool InGrabRange(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};
	vector<EntityInfo>* pVEntetyInfo{};
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Entities", pVEntetyInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	ItemInfo item;
	for (auto& e : *pVEntetyInfo)
	{
		if (e.Type == eEntityType::ITEM)
		{
			pInterface->Item_GetInfo(e, item);

			const float grabRange{ 1.f };
			if (DistanceSquared(pAgent->Position, item.Location) < (grabRange * grabRange))
			{
				return Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

// inside house
bool InsideHouse(Elite::Blackboard* pBlackboard)
{
	AgentInfo* pAgent{};
	vector<HouseInfo>* pVHouseInfo{};
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("houseTarget", pVHouseInfo) &&
		pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	//// square detection
	//PurgeZoneInfo zoneInfo;
	//for (auto& h : *pVHouseInfo)
	//{
	//	const Elite::Vector2 DangerRadius{ h.Size };
	//	const float distance{ DistanceSquared(pAgent->Position, h.Center) };
	//	if (DistanceSquared(pAgent->Position, h.Center) < (DangerRadius * DangerRadius))
	//	{
	//		pBlackboard->ChangeData("houseTarget", h);
	//		return Elite::BehaviorState::Success;
	//	}
	//}

	return Elite::BehaviorState::Failure;
}

// ACTIONS
//--------

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

Elite::BehaviorState ChangeToEvade(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pEvade = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 EvadeTarget{};
	auto dataAvailable = pBlackboard->GetData("fleeTarget", pEvade) &&
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

// use items
Elite::BehaviorState UseMedkit(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a pistol in inverntory
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::MEDKIT)
			{
				pInterface->Inventory_UseItem(i);
				return Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a pistol in inverntory
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::FOOD)
			{
				pInterface->Inventory_UseItem(i);
				return Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

// purgeZone
Elite::BehaviorState Flee(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pFlee = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	Elite::Vector2 FleeTarget{};
	auto dataAvailable = pBlackboard->GetData("Flee", pFlee) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("fleeTarget", FleeTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pFlee->SetTargetPos(FleeTarget);
	*ppSteering = pFlee;

	return Elite::BehaviorState::Success;
}

// enemy
Elite::BehaviorState RunFlee(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pFlee = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	AgentInfo* pAgent{};
	Elite::Vector2 FleeTarget{};

	auto dataAvailable = pBlackboard->GetData("Flee", pFlee) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("EnemyTarget", FleeTarget) &&
		pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	// flee
	pFlee->SetTargetPos(FleeTarget);
	*ppSteering = pFlee;

	// run
	pAgent->RunMode = true;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState FaceToClosestEnemy(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pFace = nullptr;
	ISteeringBehavior** ppAngular = nullptr;
	Elite::Vector2 FaceTarget{};
	auto dataAvailable = pBlackboard->GetData("Face", pFace) &&
		pBlackboard->GetData("Angular", ppAngular) &&
		pBlackboard->GetData("EnemyTarget", FaceTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pFace->SetTargetPos(FaceTarget);
	*ppAngular = pFace;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ShootClosestEnemy(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	// search for a pistol in inverntory
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item))
		{
			if (item.Type == eItemType::PISTOL)
			{
				pInterface->Inventory_UseItem(i);
				Elite::BehaviorState::Success;
			}
		}
	}

	return Elite::BehaviorState::Failure;
}

// get items
Elite::BehaviorState SeekItems(Elite::Blackboard* pBlackboard)
{
	ISteeringBehavior* pSeek = nullptr;
	ISteeringBehavior** ppSteering = nullptr;
	EntityInfo seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Seek", pSeek) &&
		pBlackboard->GetData("Steering", ppSteering) &&
		pBlackboard->GetData("ItemTarget", seekTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	pSeek->SetTargetPos(seekTarget.Location);
	*ppSteering = pSeek;

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState GrabItems(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	EntityInfo target{};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("ItemTarget", target);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	ItemInfo item{};
	if (pInterface->Item_Grab(target, item))
	{
		// for now first 3 slots
		switch (item.Type)
		{
		case eItemType::PISTOL:
			pInterface->Inventory_RemoveItem(0);
			pInterface->Inventory_AddItem(0, item);
			return Elite::BehaviorState::Success;
		case eItemType::FOOD:
			pInterface->Inventory_RemoveItem(1);
			return Elite::BehaviorState::Success;
			pInterface->Inventory_AddItem(1, item);
		case eItemType::MEDKIT:
			pInterface->Inventory_RemoveItem(2);
			pInterface->Inventory_AddItem(2, item);
			return Elite::BehaviorState::Success;
		default:
			return Elite::BehaviorState::Failure;
			break;
		}
	}

	return Elite::BehaviorState::Failure;
}
#endif