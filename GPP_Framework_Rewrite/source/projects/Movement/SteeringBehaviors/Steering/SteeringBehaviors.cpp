//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework/EliteMath/EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (distance < m_OutterRadius)
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance - m_InnerRadius) / m_OutterRadius;
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_InnerRadius, { 1.f, 0.25f, 0.f, 0.5f }, 0.40f);

		// Draw outter circle
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_OutterRadius, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);
	}

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);

	const Elite::Vector2 direction{ m_Target.Position - pAgent->GetPosition() };
	const float angle{ atan2f(direction.y, direction.x) };

	SteeringOutput steering{};

	steering.AngularVelocity = angle - pAgent->GetRotation();

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * m_OffsetDistance };

	m_WanderAngle += (Elite::randomFloat(0, 1) * m_MaxAngleChange - m_MaxAngleChange * .5f);
	Elite::ClampRef(m_WanderAngle, Elite::ToRadians(-90), Elite::ToRadians(90));

	m_Target.Position = (circleCenter + Elite::Vector2{ cosf(m_WanderAngle), sinf(m_WanderAngle) } *m_Radius);

	if (pAgent->CanRenderBehavior())
	{
		// Draw desired velocity again to display it above the line to circle center
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), pAgent->GetLinearVelocity().Magnitude(), {1.f, 0.f, 1.f, 0.5f}, 0.40f);

		// Draw the circle
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);

		// Draw target
		DEBUGRENDERER2D->DrawSolidCircle(m_Target.Position, 0.1f, {}, { 0.f, 1.f, 0.f }, 0.40f);

		// Draw line to circle center
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), m_OffsetDistance, {0.f, 0.f, 1.f, 0.5f}, 0.40f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// The code below works super well! But it's not the same as in the provided solution
	//m_Target.Position = m_Target.Position + m_Target.LinearVelocity * m_Target.GetDirection() * (pAgent->GetPosition() - m_Target.Position).Magnitude() / pAgent->GetMaxLinearSpeed();

	if (m_Target.LinearVelocity.Magnitude() > 0.f)
	{
		m_Target.Position += m_Target.GetDirection() * (m_Target.Position - pAgent->GetPosition()).Magnitude() / pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawSolidCircle(m_Target.Position, 0.25f, {}, { 1.f, 0.f, 1.f }, 0.40f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{ Pursuit::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}