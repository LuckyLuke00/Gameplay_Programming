#ifndef BEHAVIORS_H
#define BEHAVIORS_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "BlackboardData.h"
#include "EBehaviorTree.h"
#include "EliteMath/EMath.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState Explore(Elite::Blackboard* blackboard)
	{
		Elite::Vector2 destination{};

		if (!blackboard->GetData(DESTINATION, destination))
		{
			return Elite::BehaviorState::Failure;
		}

		blackboard->ChangeData(TARGET_INFO, destination);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState Seek(Elite::Blackboard* blackboard)
	{
		Elite::Vector2 destination{};
		AgentInfo agentInfo{};
		IExamInterface* examInterface{};
		SteeringPlugin_Output steering{};

		if (!blackboard->GetData(DESTINATION, destination) ||
			!blackboard->GetData(AGENT_INFO, agentInfo) ||
			!blackboard->GetData(EXAM_ITERFACE, examInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		destination = examInterface->NavMesh_GetClosestPathPoint(destination);

		// Draw a point at the destination
		examInterface->Draw_Point(destination, 5.f, { 1.f, .0f, .0f });

		// Prevent the agent from running, as to not deplete the stamina
		steering.RunMode = false;

		// Seek the destination
		steering.LinearVelocity = (destination - agentInfo.Position).GetNormalized() * agentInfo.MaxLinearSpeed;

		blackboard->ChangeData(STEERING_OUTPUT, steering);
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
}
#endif
