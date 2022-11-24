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
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		Elite::Vector2 targetPos;

		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };
		std::vector<AgarioFood*>* pfoodVec;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		if (!pBlackboard->GetData("FoodVec", pfoodVec) || !pfoodVec)
			return false;

		if (pfoodVec->empty())
			return false;

		const float searchRadius{ pAgent->GetRadius() + 20.f };

		float closestDistSqr{ searchRadius * searchRadius };
		AgarioFood* pClosestFood{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pFood : *pfoodVec)
		{
			const float distSqr{ pFood->GetPosition().DistanceSquared(agentPos) };

			if (distSqr < closestDistSqr)
			{
				closestDistSqr = distSqr;
				pClosestFood = pFood;
			}
		}

		if (!pClosestFood)
			return false;

		pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
		return true;
	}
}
#endif