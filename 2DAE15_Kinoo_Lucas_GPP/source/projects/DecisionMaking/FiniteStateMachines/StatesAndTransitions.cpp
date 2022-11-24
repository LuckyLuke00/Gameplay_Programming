#include "stdafx.h"
#include "StatesAndTransitions.h"

void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	//Set the steering behavior
	pAgent->SetToWander();
}

void  FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || !nearestFood) return;

	pAgent->SetToSeek(nearestFood->GetPosition());
}

void FSMStates::EvadeBiggerAgentsState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioAgent* nearestBigBoy;
	if (!pBlackboard->GetData("NearestFood", nearestBigBoy) || !nearestBigBoy) return;

	pAgent->SetToSeek(nearestBigBoy->GetPosition());
}

bool FSMConditions::FoodNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;
	if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pAgent) return false;

	const float radius{ 10.f };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser{ [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
		{
			const float dist1{ Elite::DistanceSquared(pFood1->GetPosition(), agentPos) };
			const float dist2{ Elite::DistanceSquared(pFood2->GetPosition(), agentPos) };

			return dist1 < dist2;
		}
	};

	const auto closestElementIt{ std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser) };

	if (closestElementIt == pFoodVec->end()) return false;

	AgarioFood* pClosestFood{ *closestElementIt };

	if (Elite::DistanceSquared(pClosestFood->GetPosition(), agentPos) < radius * radius)
	{
		pBlackboard->ChangeData("NearestFood", pClosestFood);
		return true;
	}

	return false;
}

bool FSMConditions::NearestBigBoyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pAgentVec;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;
	if (!pBlackboard->GetData("AgentVec", pAgentVec) || !pAgent) return false;

	const float radius{ 10.f };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser{ [agentPos](AgarioAgent const* pFood1, AgarioAgent const* pFood2)
		{
			const float dist1{ Elite::DistanceSquared(pFood1->GetPosition(), agentPos) };
			const float dist2{ Elite::DistanceSquared(pFood2->GetPosition(), agentPos) };

			return dist1 < dist2;
		}
	};

	const auto closestElementIt{ std::min_element(pAgentVec->begin(), pAgentVec->end(), isCloser) };

	if (closestElementIt == pAgentVec->end()) return false;

	AgarioAgent* pClosestFood{ *closestElementIt };

	if (Elite::DistanceSquared(pClosestFood->GetPosition(), agentPos) < radius * radius)
	{
		pBlackboard->ChangeData("NearestFood", pClosestFood);
		return true;
	}

	return false;
}