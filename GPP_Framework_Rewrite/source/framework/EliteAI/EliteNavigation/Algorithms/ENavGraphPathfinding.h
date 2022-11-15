#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteGraphs/ENavGraph.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const auto startTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			const auto endTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

			// Check if they exist
			if (startTriangle == nullptr || endTriangle == nullptr)
				return finalPath;

			// Check if they are not the same
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);

				return finalPath;
			}

			// Clone the graph (it is okay that it is of type IGraph)
			auto pClonedGraph = pNavGraph->Clone();
			auto lines{ pNavGraph->GetNavMeshPolygon()->GetLines() };
			auto startNode{ new NavGraphNode(pClonedGraph->GetNextFreeNodeIndex(), -1, startPos) };

			// Create and add the start node to the graph
			// The line index of these nodes can be -1 (they are not situated on a line)
			pClonedGraph->AddNode(startNode);

			for (int lineIdx : startTriangle->metaData.IndexLines)
			{
				const Vector2 middle{ (lines[lineIdx]->p1 + lines[lineIdx]->p2) / 2.f };
				const auto& node{ pClonedGraph->GetNodeAtWorldPos(middle) };

				if (!node) continue;

				GraphConnection2D* newConnection{ new GraphConnection2D(startNode->GetIndex(), node->GetIndex(), Distance(startPos, node->GetPosition())) };

				if (pClonedGraph->IsUniqueConnection(newConnection->GetFrom(), newConnection->GetTo()))
				{
					pClonedGraph->AddConnection(newConnection);
				}
				else delete newConnection;
			}

			const auto& endNode{ new NavGraphNode(pClonedGraph->GetNextFreeNodeIndex(), -1, endPos) };

			pClonedGraph->AddNode(endNode);

			for (int lineIdx : endTriangle->metaData.IndexLines)
			{
				const Vector2 middle{ (lines[lineIdx]->p1 + lines[lineIdx]->p2) / 2.f };
				const auto& node{ pClonedGraph->GetNodeAtWorldPos(middle) };

				if (!node || node == endNode) continue;

				GraphConnection2D* newConnection{ new GraphConnection2D(endNode->GetIndex(), node->GetIndex(), Distance(endPos, node->GetPosition())) };

				if (pClonedGraph->IsUniqueConnection(newConnection->GetFrom(), newConnection->GetTo()))
				{
					pClonedGraph->AddConnection(newConnection);
				}
				else delete newConnection;
			}
			auto pathFinder = AStar<NavGraphNode, GraphConnection2D>(pClonedGraph.get(), HeuristicFunctions::Chebyshev);
			const auto path = pathFinder.FindPath(startNode, endNode);

			for (const auto& node : path)
			{
				finalPath.push_back(node->GetPosition());
			}

			debugNodePositions = finalPath;

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			//m_Portals = SSFA::FindPortals(nodes, m_pNavGraph->GetNavMeshPolygon());
			//finalPath = SSFA::OptimizePortals(m_Portals);

			return finalPath;
		}
	};
}
