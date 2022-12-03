#include "stdafx.h"
#include "ENavGraph.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;

	// Cleanup the connections
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

// TODO: Test implementation
void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes

	// Loop over all the lines (Tip: use GetLines()) of the Polygon
	// For each line:
	for (const auto& line : m_pNavMeshPolygon->GetLines())
	{
		// Check if that line is connected to another triangle (Tip: GetTrianglesFromLineIndex)
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			// Create a NavGraphNode on the graph
				// Position it in the middle of the line
				// And give it the lineIdx
			AddNode(new NavGraphNode(GetNextFreeNodeIndex(), line->index, (line->p1 + line->p2) / 2.0f));
		}
	}

	// For each Triangle in the Navigation Mesh, find the nodes.
	for (const auto& triangle : m_pNavMeshPolygon->GetTriangles())
	{
		// Variable that stores the valid node indices reserve 3 indices
		std::vector<int> validNodeIndices;
		validNodeIndices.reserve(3);

		// Loop over the line indexes (Tip: Look at the metadata property of Triangle)
		for (const auto& lineIdx : triangle->metaData.IndexLines)
		{
			// Check if a valid NavGraphNode for that lineIdx exists (Tip: GetNodeIdxFromLineIdx)
			if (GetNodeIdxFromLineIdx(lineIdx) != invalid_node_index)
			{
				// If so, add it to the validNodeIndices
				validNodeIndices.push_back(GetNodeIdxFromLineIdx(lineIdx));
			}
		}

		// If there are 2 valid nodes, create one connection between them
		// Use the function AddConnection
		if (validNodeIndices.size() == 2)
		{
			AddConnection(new GraphConnection2D(validNodeIndices[0], validNodeIndices[1]));
		}

		if (validNodeIndices.size() == 3)
		{
			AddConnection(new GraphConnection2D(validNodeIndices[0], validNodeIndices[1]));
			AddConnection(new GraphConnection2D(validNodeIndices[1], validNodeIndices[2]));
			AddConnection(new GraphConnection2D(validNodeIndices[2], validNodeIndices[0]));
		}
	}

	// Set the connection’s costs to the distance (Tip:SetConnectionCostsToDistance)
	SetConnectionCostsToDistance();

	//2. Create connections now that every node is created

	//3. Set the connections cost to the actual distance
}
