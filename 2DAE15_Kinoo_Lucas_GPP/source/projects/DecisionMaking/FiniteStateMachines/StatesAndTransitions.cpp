#include "stdafx.h"
#include "StatesAndTransitions.h"

constexpr float gFleeRadius{ 20.f };
constexpr float gSeekRadius{ 30.f };

void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Wandering\n";

	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	//Set the steering behavior
	pAgent->SetToWander();
}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Seeking food\n";

	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || !nearestFood) return;

	pAgent->SetToSeek(nearestFood->GetPosition());
}

void FSMStates::SeekTargetState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Seeking target\n";

	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioAgent* nearestTarget;
	if (!pBlackboard->GetData("Target", nearestTarget) || !nearestTarget) return;

	pAgent->SetToSeek(nearestTarget->GetPosition());
}

void FSMStates::FleeTargetState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Fleeing target\n";

	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioAgent* pTarget;
	if (!pBlackboard->GetData("Target", pTarget) || !pTarget) return;

	pAgent->SetToFlee(pTarget->GetPosition());
}

void FSMStates::FleeBorderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Fleeing border\n";

	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return;

	// Calculate the closest border to the agent
	float closestBorder{ FLT_MAX };
	float distanceToBorder{ .0f };
	Elite::Vector2 closestBorderPosition = Elite::Vector2(.0f, .0f);

	// Left border
	distanceToBorder = pAgent->GetPosition().x;
	if (distanceToBorder < closestBorder)
	{
		closestBorder = distanceToBorder;
		closestBorderPosition = Elite::Vector2(.0f, pAgent->GetPosition().y);
	}

	// Right border
	distanceToBorder = worldSize - pAgent->GetPosition().x;
	if (distanceToBorder < closestBorder)
	{
		closestBorder = distanceToBorder;
		closestBorderPosition = Elite::Vector2(worldSize, pAgent->GetPosition().y);
	}

	// Top border
	distanceToBorder = worldSize - pAgent->GetPosition().y;
	if (distanceToBorder < closestBorder)
	{
		closestBorder = distanceToBorder;
		closestBorderPosition = Elite::Vector2(pAgent->GetPosition().x, worldSize);
	}

	// Bottom border
	distanceToBorder = pAgent->GetPosition().y;
	if (distanceToBorder < closestBorder)
	{
		closestBorder = distanceToBorder;
		closestBorderPosition = Elite::Vector2(pAgent->GetPosition().x, .0f);
	}

	pAgent->SetToFlee(closestBorderPosition);
}

bool FSMConditions::FoodNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioFood*>* pFoodVec;
	if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pAgent) return false;

	const float radius{ pAgent->GetRadius() + gSeekRadius };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ .0f, 1.f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

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

bool FSMConditions::NoFoodNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	// Needed for debug drawing
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	const float radius{ pAgent->GetRadius() + gSeekRadius };
	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), radius, Elite::Color{ 1.f, .0f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

	// Call the FoodNearbyCondition and return the opposite
	return !FoodNearbyCondition::Evaluate(pBlackboard);
}

bool FSMConditions::BiggerAgentNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVec", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + gFleeRadius };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 1.f, .0f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

	// Loop over all agents and check if they are bigger than the current agent
	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		// Check if the other agent is bigger than the current agent
		if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

		// Check if the other agent is within the radius
		if (Elite::DistanceSquared(pOtherAgent->GetPosition(), agentPos) > radius * radius) continue;

		// Check if the other agent is not too close to the current agent
		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.f) continue;

		// Set the target to the other agent
		pBlackboard->ChangeData("Target", pOtherAgent);

		// Draw circle on the other agent
		DEBUGRENDERER2D->DrawCircle(pOtherAgent->GetPosition(), pOtherAgent->GetRadius() + 1.f, Elite::Color{ 1.f, .0f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

		return true;
	}

	return false;
}

bool FSMConditions::NoBiggerAgentNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	// Needed for debug drawing
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	const float radius{ pAgent->GetRadius() + gSeekRadius };
	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), radius, Elite::Color{ 1.f, .5f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

	// Call the BiggerAgentNearbyCondition and return the opposite
	return !BiggerAgentNearbyCondition::Evaluate(pBlackboard);
}

bool FSMConditions::BorderNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	const float radius{ pAgent->GetRadius() + gFleeRadius };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return false;

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ .0f, .0f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());

	// Check if the agent is within the radius of the border
	return (agentPos.x < radius || agentPos.x > worldSize - radius || agentPos.y < radius || agentPos.y > worldSize - radius);
}

bool FSMConditions::SmallerAgentNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVec", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + gSeekRadius };

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 1.f, .0f, .0f }, DEBUGRENDERER2D->NextDepthSlice());

	// Loop over all agents and check if they are smaller than the current agent
	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		// Check if the other agent is smaller than the current agent
		if (pOtherAgent->GetRadius() > pAgent->GetRadius()) continue;

		// Check if the other agent is within the radius
		if (Elite::DistanceSquared(pOtherAgent->GetPosition(), agentPos) > radius * radius) continue;

		// Check if the other agent is not too close to the current agent
		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.f) continue;

		// Set the target to the other agent
		pBlackboard->ChangeData("Target", pOtherAgent);

		// Draw circle on the other agent
		DEBUGRENDERER2D->DrawCircle(pOtherAgent->GetPosition(), pOtherAgent->GetRadius() + 1.f, Elite::Color{ 1.f, .0f, .0f }, DEBUGRENDERER2D->NextDepthSlice());
		return true;
	}

	return false;
}