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

	Elite::BehaviorState ChangeToSeekAgent(Elite::Blackboard* pBlackboard)
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

	Elite::BehaviorState ChangeToFleeAgent(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		Elite::Vector2 targetPos;

		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToFlee(targetPos);

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	constexpr float gFleeRadius{ 25.f };
	constexpr float gSeekRadius{ 50.f };

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

		const float searchRadius{ pAgent->GetRadius() + gSeekRadius };

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

	bool IsBiggerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };
		std::vector<AgarioAgent*>* pAgentsVec;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || !pAgentsVec)
			return false;

		if (pAgentsVec->empty())
			return false;

		const float searchRadius{ pAgent->GetRadius() + gFleeRadius };

		// Debug draw search radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), searchRadius, { 1.f, .0f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		float closestDistSqr{ searchRadius * searchRadius };
		AgarioAgent* pClosestAgent{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pOtherAgent : *pAgentsVec)
		{
			if (pOtherAgent == pAgent)
				continue;

			const float distSqr{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distSqr < closestDistSqr && pOtherAgent->GetRadius() > pAgent->GetRadius())
			{
				closestDistSqr = distSqr;
				pClosestAgent = pOtherAgent;
			}
		}

		if (!pClosestAgent)
			return false;

		pBlackboard->ChangeData("Target", pClosestAgent->GetPosition());

		// Debug draw a circle around the target
		DEBUGRENDERER2D->DrawCircle(pClosestAgent->GetPosition(), pClosestAgent->GetRadius() + 1.f, { 1.f, .0f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		return true;
	}

	bool IsSmallerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };
		std::vector<AgarioAgent*>* pAgentsVec;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || !pAgentsVec)
			return false;

		if (pAgentsVec->empty())
			return false;

		const float searchRadius{ pAgent->GetRadius() + gSeekRadius };

		// Debug draw search radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), searchRadius, { .0f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		float closestDistSqr{ searchRadius * searchRadius };
		AgarioAgent* pClosestAgent{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pOtherAgent : *pAgentsVec)
		{
			if (pOtherAgent == pAgent)
				continue;

			const float distSqr{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distSqr < closestDistSqr && pOtherAgent->GetRadius() + 1.f < pAgent->GetRadius())
			{
				closestDistSqr = distSqr;
				pClosestAgent = pOtherAgent;
			}
		}

		if (!pClosestAgent)
			return false;

		pBlackboard->ChangeData("Target", pClosestAgent->GetPosition());

		// Debug draw a circle around the target
		DEBUGRENDERER2D->DrawCircle(pClosestAgent->GetPosition(), pClosestAgent->GetRadius() + 1.f, { .0f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
		return true;
	}

	bool IsTargetNotNearBiggerAgent(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent{ nullptr };
		std::vector<AgarioAgent*>* pAgentsVec;
		Elite::Vector2 targetPos{};

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return true;

		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || !pAgentsVec)
			return true;

		if (!pBlackboard->GetData("Target", targetPos))
			return true;

		if (pAgentsVec->empty())
			return true;

		const float searchRadius{ pAgent->GetRadius() + gFleeRadius };

		// Debug draw search radius
		DEBUGRENDERER2D->DrawCircle(targetPos, searchRadius, { 1.f, .0f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		float closestDistSqr{ searchRadius * searchRadius };
		AgarioAgent* pClosestAgent{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pOtherAgent : *pAgentsVec)
		{
			if (pOtherAgent == pAgent)
				continue;

			const float distSqr{ pOtherAgent->GetPosition().DistanceSquared(targetPos) };

			if (distSqr < closestDistSqr && pOtherAgent->GetRadius() > pAgent->GetRadius())
			{
				closestDistSqr = distSqr;
				pClosestAgent = pOtherAgent;
			}
		}

		if (!pClosestAgent)
			return true;

		// Debug draw a circle around the target
		DEBUGRENDERER2D->DrawCircle(pClosestAgent->GetPosition(), pClosestAgent->GetRadius() + 1.f, { 1.f, .5f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		return false;
	}
}
#endif