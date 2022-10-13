#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
    int flockSize /*/= 50 / */,
    float worldSize/* /= 100.f / */,
    SteeringAgent* pAgentToEvade /*/= nullptr /*/ ,
    bool trimWorld /*/= false /*/ )

    : m_WorldSize{ worldSize }
    , m_FlockSize{ flockSize }
    , m_TrimWorld{ trimWorld }
    , m_pAgentToEvade{ pAgentToEvade }
    , m_NeighborhoodRadius{ 15 }
    , m_NrOfNeighbors{ 0 }
{
    m_Agents.resize(m_FlockSize);
    m_Neighbors.resize(m_FlockSize);

    m_pSeekBehavior = new Seek();
    m_pWanderBehavior = new Wander();

    m_pEvadeBehavior = new Evade();

    m_pCohesionBehavior = new Cohesion(this);
    m_pSeparationBehavior = new Separation(this);
    m_pVelMatchBehavior = new VelocityMatch(this);

    m_pBlendedSteering = new BlendedSteering({ { m_pCohesionBehavior,0.5f }, { m_pSeparationBehavior,0.5f }, { m_pVelMatchBehavior,0.5f },{m_pWanderBehavior,0.5f },{m_pSeekBehavior,0.5f} });
    m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior,m_pBlendedSteering });

    for (int index{}; index < m_FlockSize; ++index)
    {
        m_Agents[index] = new SteeringAgent();
        m_Agents[index]->SetSteeringBehavior(m_pPrioritySteering);
        m_Agents[index]->SetMaxLinearSpeed(15.0f);
        m_Agents[index]->SetMass(0.f);
        m_Agents[index]->SetAutoOrient(true);
        m_Agents[index]->SetPosition(randomVector2(m_WorldSize));
        m_Agents[index]->SetLinearVelocity(randomVector2(m_Agents[index]->GetMaxLinearSpeed()));
    }

    m_pAgentToEvade = new SteeringAgent();
    m_pAgentToEvade->SetSteeringBehavior(m_pSeekBehavior);
    m_pAgentToEvade->SetMaxLinearSpeed(15.0f);
    m_pAgentToEvade->SetMass(0.f);
    m_pAgentToEvade->SetAutoOrient(true);
    m_pAgentToEvade->SetPosition(randomVector2(m_WorldSize));
    m_pAgentToEvade->SetLinearVelocity(randomVector2(m_pAgentToEvade->GetMaxLinearSpeed()));
    m_pAgentToEvade->SetBodyColor({ 1, 0, 0 });
}

Flock::~Flock()
{
    SAFE_DELETE(m_pEvadeBehavior);
    SAFE_DELETE(m_pSeekBehavior);
    SAFE_DELETE(m_pWanderBehavior);
    SAFE_DELETE(m_pCohesionBehavior);
    SAFE_DELETE(m_pSeparationBehavior);
    SAFE_DELETE(m_pBlendedSteering);
    SAFE_DELETE(m_pPrioritySteering);
    SAFE_DELETE(m_pVelMatchBehavior);
    SAFE_DELETE(m_pAgentToEvade);

    for (auto pAgent : m_Agents)
    {
        SAFE_DELETE(pAgent);
    }
    m_Agents.clear();
}
void Flock::Update(float deltaT)
{
    m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());

    for (SteeringAgent* pAgent : m_Agents)
    {
        if (pAgent != nullptr)
        {
            RegisterNeighbors(pAgent);
            pAgent->Update(deltaT);

            if (m_TrimWorld)
            {
                pAgent->TrimToWorld(m_WorldSize);
            }
			
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
    for (SteeringAgent* pAgent : m_Agents)
    {
        if (pAgent != nullptr)
        {
            pAgent->Render(deltaT);
        }
    }

    if (m_pAgentToEvade)
    {
        m_pAgentToEvade->Render(deltaT);
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

    // TODO: Implement checkboxes for debug rendering and weight sliders here
    ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
    ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
    ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
    ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
    ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

    //End
    ImGui::PopAllowKeyboardFocus();
    ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// Update the container and neighbor count
	// (resetting the neighbor count and overwriting the variables already in the container)

	m_NrOfNeighbors = 0;
	for (auto pOtherAgent : m_Agents)
	{
		if (pOtherAgent != pAgent)
		{
			const float dist{ Distance(pAgent->GetPosition(), pOtherAgent->GetPosition()) };

			if (dist <= m_NeighborhoodRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = pOtherAgent;
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 averagePos{};

	for (const auto& neighbor : m_Neighbors)
	{
		if (neighbor != nullptr)
		{
			averagePos += neighbor->GetPosition();
		}
	}

	return averagePos / static_cast<float>(m_NrOfNeighbors);
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 averageVelocity{};

	for (const auto& neighbor : m_Neighbors)
	{
		if (neighbor != nullptr)
		{
			averageVelocity += neighbor->GetLinearVelocity();
		}
	}

	averageVelocity /= static_cast<float>(m_NrOfNeighbors);

	return averageVelocity;
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