//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

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

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), { 1.f, 0.22f, 1.f });

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), { 0.f, 0.682f, 0.937f});

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), { 0.f, 0.941f, 0.f });

	}

	return steering;
}

//FLEE
//****
//Moving away from a target position or agent
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position; // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), { 1.f, 0.22f, 1.f });

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), { 0.f, 0.682f, 0.937f });

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), { 0.f, 0.941f, 0.f });

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

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), { 1.f, 0.22f, 1.f });

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), { 0.f, 0.682f, 0.937f });

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), { 0.f, 0.941f, 0.f });

		// Render slow radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), slowRadius, { 1.f, 0.949f, 0.22f }, 0.f);

		// Render target radius
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), targetRadius, { 1.f, 0.329f, 0.329f }, 0.f);

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

//WANDER
//****
//Randomized movement pattern that resembles wandering around
//A random point on a circle in front of the agent is chosen as a new target. This results in a somewhat believable random movement pattern.
//Adding a random offset to the wander target and projecting it back onto the circle.
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	static constexpr float circleRadius{ 4.f };
	const Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * Elite::Vector2{pAgent->GetMaxLinearSpeed(), pAgent->GetMaxLinearSpeed()} };

	// Random offset
	const float randomOffset{ static_cast<float>(rand() % 360) };
	const Elite::Vector2 randomOffsetVector{ cosf(randomOffset), sinf(randomOffset) };

	// Project random offset onto circle
	const Elite::Vector2 wanderTarget{ circleCenter + randomOffsetVector * circleRadius };

	// Seek the wander target
	steering.LinearVelocity = wanderTarget - pAgent->GetPosition(); // Desired velocity
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Scale desired velocity to max speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		const Elite::Vector2 currentVelocity{ pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredCurrentVelocity{ steering.LinearVelocity - pAgent->GetLinearVelocity() };
		const Elite::Vector2 desiredVelocity{ steering.LinearVelocity };

		// Render current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), currentVelocity, currentVelocity.Magnitude(), { 1.f, 0.22f, 1.f });

		// Render desired velocity - current velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredCurrentVelocity, desiredCurrentVelocity.Magnitude(), { 0.f, 0.682f, 0.937f });

		// Render desired velocity
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, desiredVelocity.Magnitude(), { 0.f, 0.941f, 0.f });

		// Render circle
		DEBUGRENDERER2D->DrawCircle(circleCenter, circleRadius, { 1.f, 0.949f, 0.22f }, 0.f);

		// Render offset
		DEBUGRENDERER2D->DrawCircle(wanderTarget, 0.1f, { 1.f, 0.329f, 0.329f }, 0.f);

	}

	return steering;

}