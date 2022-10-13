#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Move towards the neighborhood center (the average position of all neighboring agents)
	return { m_pFlock->GetAverageNeighborPos().GetNormalized() * pAgent->GetMaxLinearSpeed() };
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Move away from neighbors with a magnitude that’s inversely proportional to the distance to
	// that neighbor.The closer a neighbor is, the more impact it should have on the separation.
	return { (pAgent->GetPosition() - m_pFlock->GetAverageNeighborPos()).GetNormalized() * pAgent->GetMaxLinearSpeed() };
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Match the average velocity of nearby agents
	return { m_pFlock->GetAverageNeighborVelocity().GetNormalized() * pAgent->GetMaxLinearSpeed() };
}