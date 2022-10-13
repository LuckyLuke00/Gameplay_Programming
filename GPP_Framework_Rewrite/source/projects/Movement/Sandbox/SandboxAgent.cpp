#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent() : BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
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