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