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
#include "Behaviors.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
	{
		return false;
	}

	//TODO: Check for food closeby and set target accordingly
	const float closeToFoodRange{ 20.f };

	// finding food
	auto foodIt = std::find_if(foodVec->begin(), foodVec->end(), [&pAgent, &closeToFoodRange](AgarioFood* food)
		{
			return DistanceSquared(pAgent->GetPosition(), food->GetPosition()) < (closeToFoodRange * closeToFoodRange);
		});

	// setting the target
	if (foodIt != foodVec->end())
	{
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}

	return false;
}

bool IsBiggerEnemyClose(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* AgentsVec = nullptr;
	const float DangerRadius{ 15.f };

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", AgentsVec);

	auto enemyIt = std::find_if(AgentsVec->begin(), AgentsVec->end(), [&pAgent, &DangerRadius](AgarioAgent* enemy)
		{
			return DistanceSquared(pAgent->GetPosition(), enemy->GetPosition()) < (DangerRadius * DangerRadius) && enemy->GetRadius() > pAgent->GetRadius();
		});

	if (enemyIt != AgentsVec->end())
	{
		pBlackboard->ChangeData("Target", (*enemyIt)->GetPosition());
		return true;
	}

	return false;
}

bool IsSmallerEnemyClose(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* AgentsVec = nullptr;
	const float CloseRadius{ 15.f };

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", AgentsVec);

	auto enemyIt = std::find_if(AgentsVec->begin(), AgentsVec->end(), [&pAgent, &CloseRadius](AgarioAgent* enemy)
		{
			return DistanceSquared(pAgent->GetPosition(), enemy->GetPosition()) < (CloseRadius * CloseRadius) && enemy->GetRadius() < pAgent->GetRadius();
		});

	if (enemyIt != AgentsVec->end())
	{
		pBlackboard->ChangeData("Target", (*enemyIt)->GetPosition());
		return true;
	}

	return false;
}

Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
	{
		return Elite::BehaviorState::Failure;
	}

	pAgent->SetToWander();

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (!pAgent)
	{
		return Elite::BehaviorState::Failure;
	}

	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(seekTarget);

	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 fleeTarget{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", fleeTarget);
	if (!dataAvailable)
	{
		return Elite::BehaviorState::Failure;
	}

	if (!pAgent)
	{
		return Elite::BehaviorState::Failure;
	}

	pAgent->SetToFlee(fleeTarget);

	return Elite::BehaviorState::Success;
}

#endif