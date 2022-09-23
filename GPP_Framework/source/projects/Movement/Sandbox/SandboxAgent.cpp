#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent() : BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	Seek();

	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2(velocity.y, velocity.x) + E_PI_2);
	}

	SetRotation(GetRotation() + E_PI_2);
}

void SandboxAgent::Seek() const
{
	// This function makes the agent seek the target
	// Goes straight to that target

	// Constants
	static constexpr float maxSpeed{ 50.f };
	static constexpr float arrivalRadius{ 1.f };
	static constexpr float slowRadius{ 15.f };

	const Vector2 toTarget{ m_Target - GetPosition() };
	const float distance{ toTarget.Magnitude() };

	if (distance < arrivalRadius)
	{
		SetLinearVelocity({ 0.f,0.f });
		return;
	}

	Vector2 velocity{ toTarget.GetNormalized() };

	if (distance < slowRadius)
	{
		velocity *= maxSpeed * (distance / slowRadius);
	}
	else
	{
		velocity *= maxSpeed;
	}

	SetLinearVelocity(velocity);
}
