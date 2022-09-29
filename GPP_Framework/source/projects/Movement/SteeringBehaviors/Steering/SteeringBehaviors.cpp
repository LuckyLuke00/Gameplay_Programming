//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework/EliteMath/EMatrix2x3.h"

//SEEK
//****
//Moving to a target position or agent
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);
	}

	return steering;
}

//FLEE
//****
//Moving away from a target position or agent
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 fromTarget{ pAgent->GetPosition() - m_Target.Position };
	const float distance{ fromTarget.Magnitude() };

	SteeringOutput steering = {};

	if (distance > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}


	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position; // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);
	}

	return steering;
}

//ARRIVE
//****
//Seek but with slowing down when nearing the target
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	static constexpr float slowRadius{ 20.f };
	static constexpr float targetRadius{ 3.f };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); // Desired velocity

	const float distance{ steering.LinearVelocity.Magnitude() };

	if (distance > slowRadius)
	{
		steering.LinearVelocity.Normalize(); // Normalize desired velocity
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed
	}
	else if (distance < targetRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}
	else
	{
		steering.LinearVelocity.Normalize(); // Normalize desired velocity
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance - targetRadius) / slowRadius; // Scale desired velocity to max speed
	}

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);

		// Render slow radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), slowRadius, m_DebugColor3, 0.f);

		// Render target radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), targetRadius, m_DebugColor6, 0.f);
	}

	return steering;
}

//FACE
//****
//Looking towards the target
//Use angular velocity, don’t use SetRotation()!
//Disable the agent's AutoOrient
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Disable the agent's AutoOrient
	pAgent->SetAutoOrient(false);

	Elite::Vector2 direction{ m_Target.Position - pAgent->GetPosition() };
	direction.Normalize();

	const float angle{ atan2f(direction.y, direction.x) };

	steering.AngularVelocity = angle - pAgent->GetRotation();

	return steering;
}

////WANDER
////****
////Randomized movement pattern that resembles wandering around
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Calculate circle center
	//const Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * Elite::Vector2{pAgent->GetMaxLinearSpeed(), pAgent->GetMaxLinearSpeed()} };
	const Elite::Vector2 circleCenter{ pAgent->GetPosition() + Elite::Vector2{m_OffsetDistance,m_OffsetDistance } };

	// Calculate target on circle
	const Elite::Vector2 targetOnCircle{ circleCenter + Elite::Vector2{ cosf(m_WanderAngle), sinf(m_WanderAngle) } * m_Radius };

	// Calculate new wander angle in radians
	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	// Seek the target
	steering.LinearVelocity = targetOnCircle - pAgent->GetPosition(); // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);

		// Render circle
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, m_DebugColor3, 0.f);

		// Render target
		DEBUGRENDERER2D->DrawSolidCircle(targetOnCircle, 0.2f, {}, m_DebugColor7);
	}

	return steering;
}

////PURSUIT
////****
////Trying to intercept an agent by moving towards where it will be 
////in the future.
//SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
//{
//	SteeringOutput steering = {};
//
//	// Predict target position
//	const Elite::Vector2 predictedPosition{ m_Target.Position + m_Target.GetDirection() + m_Target.LinearVelocity };
//
//	// Seek the target
//	steering.LinearVelocity = predictedPosition - pAgent->GetPosition(); // Desired velocity
//	steering.LinearVelocity.Normalize(); // Normalize desired velocity
//	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed
//
//	// Debug rendering
//	if (pAgent->CanRenderBehavior())
//	{
//		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
//		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
//		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };
//
//		// Render desired velocity
//		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);
//
//		// Render desired velocity - current velocity
//		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);
//
//		// Render current velocity
//		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);
//
//		// Render target position
//		DEBUGRENDERER2D->DrawSolidCircle(predictedPosition, 0.2f, {}, m_DebugColor6);
//	}
//
//	return steering;
//}

//PURSUIT
//****
//Trying to intercept an agent by moving towards where it will be 
//in the future.
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	const float time{ distance / pAgent->GetMaxLinearSpeed() };

	const Elite::Vector2 futurePosition{ m_Target.Position + m_Target.LinearVelocity * time };

	// Seek the future position of the target
	steering.LinearVelocity = futurePosition - pAgent->GetPosition(); // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);

		// Render future position
		DEBUGRENDERER2D->DrawSolidCircle(futurePosition, 0.2f, {}, m_DebugColor6);
	}

	return steering;
}

//EVADE
//****
//Trying to avoid an agents path
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	const float time{ distance / pAgent->GetMaxLinearSpeed() };

	const Elite::Vector2 futurePosition{ m_Target.Position + m_Target.LinearVelocity * time };

	// Flee from the future position of the target
	steering.LinearVelocity = pAgent->GetPosition() - futurePosition; // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), m_DebugColor1);

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), m_DebugColor3);

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), m_DebugColor5);

		// Render future position
		DEBUGRENDERER2D->DrawSolidCircle(futurePosition, 0.2f, {}, m_DebugColor6);
	}

	return steering;
}