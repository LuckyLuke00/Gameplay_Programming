/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class EvadeBiggerAgentsState : public Elite::FSMState
	{
	public:
		EvadeBiggerAgentsState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};
}

//-----------------
//---TRANSITIONS---
//-----------------
namespace FSMConditions
{
	class FoodNearbyCondition : public Elite::FSMCondition
	{
	public:
		FoodNearbyCondition() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NearestBigBoyCondition : public Elite::FSMCondition
	{
	public:
		NearestBigBoyCondition() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}
#endif