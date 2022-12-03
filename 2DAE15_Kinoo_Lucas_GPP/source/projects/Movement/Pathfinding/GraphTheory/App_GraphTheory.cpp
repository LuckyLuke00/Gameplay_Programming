//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
//#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"
//#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAstar_PGAIBE.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2D)
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_pGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>(false);
	m_pGraph2D->AddNode(new GraphNode2D(0, { 20, 30 }));
	m_pGraph2D->AddNode(new GraphNode2D(1, { -10, -10 }));
	m_pGraph2D->AddConnection(new GraphConnection2D(0, 1));
}

void App_GraphTheory::Update(float deltaTime)
{
	m_GraphEditor.UpdateGraph(m_pGraph2D);
	m_pGraph2D->SetConnectionCostsToDistance();

	const auto eulerFinder{ EulerianPath<GraphNode2D, GraphConnection2D>(m_pGraph2D) };
	const Eulerianity eulerianity{ eulerFinder.IsEulerian() };

	m_pGraph = eulerFinder.GetGraph();

	// Variables for printing the message once
	static Eulerianity lastEulerianity{};
	std::string message{ "Graph is " };

	switch (eulerianity)
	{
	case Elite::Eulerianity::notEulerian:
		message += "not Eulerian\n";
		break;
	case Elite::Eulerianity::semiEulerian:
		message += "semi-Eulerian\n";
		break;
	case Elite::Eulerianity::eulerian:
		message += "Eulerian\n";
		break;
	default:
		break;
	}

	if (lastEulerianity != eulerianity)
	{
		// Print message
		std::cout << message;
		lastEulerianity = eulerianity;
	}

	UpdateNodeColors();

	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) - static_cast<float>(menuWidth) - 10, 10));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(menuWidth), static_cast<float>(height) - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GraphRenderer.RenderGraph(m_pGraph2D, true, true);
}

void App_GraphTheory::UpdateNodeColors()
{
	// For every node
	for (auto& node : m_pGraph2D->GetAllNodes())
	{
		// Rest the color to default so we can use the same color for multiple nodes
		node->SetColor(m_MinColors.front());

		for (const auto& color : m_MinColors)
		{
			if (!HasNeighborSameColor(node)) break;

			node->SetColor(color);
		}

		while (HasNeighborSameColor(node))
		{
			node->SetColor(Color::GenerateRandomColor());
		}

		// Check if the color is already in the vector
		if (std::find(m_MinColors.begin(), m_MinColors.end(), node->GetColor()) == m_MinColors.end())
		{
			m_MinColors.push_back(node->GetColor());
		}
	}
}

bool App_GraphTheory::HasNeighborSameColor(const Elite::GraphNode2D* pNode) const
{
	// Get the connections to the current node on the m_pGraph
	const auto& connections{ m_pGraph->GetNodeConnections(pNode->GetIndex()) };

	// For every connection
	for (const auto& connection : connections)
	{
		// Get the node on the other side of the connection
		const auto& node{ m_pGraph->GetNode(connection->GetTo()) };

		if (node->GetColor() == pNode->GetColor())
			return true;
	}

	return false;
}