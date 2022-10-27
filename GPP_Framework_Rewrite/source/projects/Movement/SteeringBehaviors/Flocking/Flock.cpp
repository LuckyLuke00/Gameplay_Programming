#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,

	bool trimWorld /*= false*/
)
	: m_FlockSize{ flockSize }
	//, m_pCellSpace{ new CellSpace{worldSize, worldSize, 15, 15, flockSize } }
	, m_TrimWorld{ trimWorld }
	, m_WorldSize{ worldSize }
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{ 0 }
	, m_pAgentToEvade{ pAgentToEvade }
{
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize);

	m_pCohesionBehavior = new Cohesion(this);
	m_pEvadeBehavior = new Evade();
	m_pSeekBehavior = new Seek();
	m_pEvadingAgentSeekBehavior = new Seek();
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pWanderBehavior = new Wander();

	m_pBlendedSteering = new BlendedSteering({ { m_pCohesionBehavior,0.25f }, { m_pSeparationBehavior,0.25f }, { m_pVelMatchBehavior,0.25f },{m_pWanderBehavior,0.25f },{m_pSeekBehavior,0.25f} });
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });

	m_pCellSpace = new CellSpace{ worldSize, worldSize, static_cast<int>(m_QueryRadius) * 2, static_cast<int>(m_QueryRadius) * 2, flockSize };

	for (int idx{ 0 }; idx < m_FlockSize; ++idx)
	{
		m_Agents[idx] = new SteeringAgent();
		m_Agents[idx]->SetAutoOrient(true);
		m_Agents[idx]->SetMaxLinearSpeed(50.0f);
		m_Agents[idx]->SetMass(0.f);
		m_Agents[idx]->SetPosition({ static_cast<float>(rand() % static_cast<int>(m_WorldSize)), static_cast<float>(rand() % static_cast<int>(m_WorldSize)) });
		m_Agents[idx]->SetSteeringBehavior(m_pPrioritySteering);

		m_pCellSpace->AddAgent(m_Agents[idx]);
		m_OldPos.push_back(m_Agents[idx]->GetPosition());
	}

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetBodyColor({ 1, 0, 0 });
	m_pAgentToEvade->SetMaxLinearSpeed(50.0f);
	m_pAgentToEvade->SetMass(0.f);
	m_pAgentToEvade->SetPosition({ static_cast<float>(rand() % static_cast<int>(m_WorldSize)), static_cast<float>(rand() % static_cast<int>(m_WorldSize)) });
	m_pAgentToEvade->SetSteeringBehavior(m_pEvadingAgentSeekBehavior);
}

Flock::~Flock()
{
	SAFE_DELETE(m_pAgentToEvade)
		SAFE_DELETE(m_pBlendedSteering)
		SAFE_DELETE(m_pCellSpace)
		SAFE_DELETE(m_pCohesionBehavior)
		SAFE_DELETE(m_pEvadeBehavior)
		SAFE_DELETE(m_pEvadingAgentSeekBehavior)
		SAFE_DELETE(m_pPrioritySteering)
		SAFE_DELETE(m_pSeekBehavior)
		SAFE_DELETE(m_pSeparationBehavior)
		SAFE_DELETE(m_pVelMatchBehavior)
		SAFE_DELETE(m_pWanderBehavior)

		for (auto pAgent : m_Agents)
		{
			SAFE_DELETE(pAgent)
		}
	m_Agents.clear();
}
void Flock::Update(float deltaT)
{
	m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());

	int oldPosIdx{ 0 };

	for (SteeringAgent* pAgent : m_Agents)
	{
		if (pAgent == nullptr) continue;

		if (m_TrimWorld)
		{
			pAgent->TrimToWorld(m_WorldSize);
		}

		pAgent->Update(deltaT);

		if (m_SpatialPartitioning)
		{
			m_pCellSpace->RegisterNeighbors(pAgent, m_QueryRadius);
			m_pCellSpace->UpdateAgentCell(pAgent, m_OldPos[oldPosIdx]);
			m_OldPos[oldPosIdx++] = pAgent->GetPosition();
		}
		else
		{
			RegisterNeighbors(pAgent);
		}

		pAgent->SetRenderBehavior(m_DebugRenderSteering);

		if (pAgent != m_Agents.back()) continue;

		// Only execute the code below if the agent is the last one in the vector
		pAgent->SetRenderBehavior(m_DebugRenderNeighborhood);

		if (!m_DebugRenderNeighborhood) continue;

		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, { 1.f, 1.f, 1.f, 0.5f }, 0.f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), this->GetAverageNeighborPos(), 7, { 1.f, 0.f, 0.f });

		for (const auto& neighbor : m_Neighbors)
		{
			if (neighbor == nullptr) continue;
			neighbor->SetBodyColor({ 0.f, 1.f, 0.f });
		}
	}

	if (m_pAgentToEvade)
	{
		m_pAgentToEvade->Update(deltaT);
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}
}
void Flock::Render(float deltaT)
{
	m_pAgentToEvade->Render(deltaT);
	if (m_DebugRenderPartitions) m_pCellSpace->RenderCells();

	if (m_FlockSize > 250) return;
	for (SteeringAgent* pAgent : m_Agents)
	{
		if (pAgent != nullptr)
		{
			pAgent->Render(deltaT);
		}
	}
}
void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::Checkbox("Spatial Partitioning", &m_SpatialPartitioning);
	ImGui::Checkbox("Debug Render partitions", &m_DebugRenderPartitions);
	ImGui::Checkbox("Debug Render neighborhood", &m_DebugRenderNeighborhood);
	ImGui::Checkbox("Debug Render steering", &m_DebugRenderSteering);

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Vel Match", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pOtherAgent : m_Agents)
	{
		pOtherAgent->SetBodyColor({ 1.f,1.f,0.f });

		if (pAgent == pOtherAgent) continue;

		const float distance{ (pOtherAgent->GetPosition() - pAgent->GetPosition()).Magnitude() };
		if (distance < m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors++] = pOtherAgent;
		}
	}

	// Overwrite left over agents with nullptr
	for (size_t i{ m_NrOfNeighbors }; i < m_Agents.size(); ++i)
	{
		m_Neighbors[i] = nullptr;
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 avgPos{};
	if (m_SpatialPartitioning)
	{
		for (int idx{ 0 }; idx < m_pCellSpace->GetNeighbors().size(); ++idx)
		{
			if (m_pCellSpace->GetNeighbors()[idx] == nullptr) continue;

			avgPos += m_pCellSpace->GetNeighbors()[idx]->GetPosition();
		}
		return avgPos / static_cast<float>(m_pCellSpace->GetNeighbors().size());
	}

	for (int idx{ 0 }; idx < m_NrOfNeighbors; ++idx)
	{
		avgPos += m_Neighbors[idx]->GetPosition();
	}
	return avgPos / static_cast<float>(m_NrOfNeighbors);
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 avgVel{};

	if (m_SpatialPartitioning)
	{
		for (int idx{ 0 }; idx < m_pCellSpace->GetNeighbors().size(); ++idx)
		{
			if (m_pCellSpace->GetNeighbors()[idx] == nullptr) continue;

			avgVel += m_pCellSpace->GetNeighbors()[idx]->GetLinearVelocity();
		}
		return avgVel / static_cast<float>(m_pCellSpace->GetNeighbors().size());
	}

	for (int idx{ 0 }; idx < m_NrOfNeighbors; ++idx)
	{
		avgVel += m_Neighbors[idx]->GetLinearVelocity();
	}
	return avgVel / static_cast<float>(m_NrOfNeighbors);
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}