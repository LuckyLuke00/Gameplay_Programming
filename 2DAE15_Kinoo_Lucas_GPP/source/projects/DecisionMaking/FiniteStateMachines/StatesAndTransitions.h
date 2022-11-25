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

	class SeekTargetState : public Elite::FSMState
	{
	public:
		SeekTargetState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeTargetState : public Elite::FSMState
	{
	public:
		FleeTargetState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeBorderState : public Elite::FSMState
	{
	public:
		FleeBorderState() : FSMState() {};
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

	class NoFoodNearbyCondition : public FoodNearbyCondition
	{
	public:
		NoFoodNearbyCondition() : FoodNearbyCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BiggerAgentNearbyCondition : public Elite::FSMCondition
	{
	public:
		BiggerAgentNearbyCondition() : Elite::FSMCondition{} {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoBiggerAgentNearbyCondition : public BiggerAgentNearbyCondition
	{
	public:
		NoBiggerAgentNearbyCondition() : BiggerAgentNearbyCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BorderNearbyCondition : public Elite::FSMCondition
	{
	public:
		BorderNearbyCondition() : Elite::FSMCondition{} {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallerAgentNearbyCondition : public Elite::FSMCondition
	{
	public:
		SmallerAgentNearbyCondition() : Elite::FSMCondition{} {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}
#endif