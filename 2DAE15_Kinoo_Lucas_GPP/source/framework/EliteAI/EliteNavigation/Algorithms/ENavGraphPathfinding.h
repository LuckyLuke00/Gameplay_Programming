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
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph const* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const auto startTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const auto endTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			// Check if they exist
			if (!startTriangle || !endTriangle)
				return finalPath;

			// Check if they are not the same
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			// Clone the graph (it is okay that it is of type IGraph)
			const auto pClonedGraph{ pNavGraph->Clone() };
			const auto lines{ pNavGraph->GetNavMeshPolygon()->GetLines() };
			const auto startNode{ new NavGraphNode(pClonedGraph->GetNextFreeNodeIndex(), -1, startPos) };

			// Create and add the start node to the graph
			// The line index of these nodes can be -1 (they are not situated on a line)
			pClonedGraph->AddNode(startNode);

			for (int lineIdx : startTriangle->metaData.IndexLines)
			{
				const int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(lineIdx) };

				if (nodeIdx == -1) continue;

				const auto node{ pNavGraph->GetNode(nodeIdx) };

				if (!node || node == startNode) continue;

				GraphConnection2D* newConnection{ new GraphConnection2D(startNode->GetIndex(), nodeIdx, Distance(startPos, node->GetPosition())) };

				if (pClonedGraph->IsUniqueConnection(newConnection->GetFrom(), newConnection->GetTo()))
				{
					pClonedGraph->AddConnection(newConnection);
				}
				else SAFE_DELETE(newConnection)
			}

			const auto& endNode{ new NavGraphNode(pClonedGraph->GetNextFreeNodeIndex(), -1, endPos) };

			pClonedGraph->AddNode(endNode);

			for (int lineIdx : endTriangle->metaData.IndexLines)
			{
				const int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(lineIdx) };

				if (nodeIdx == -1) continue;

				const auto& node{ pClonedGraph->GetNode(nodeIdx) };

				if (!node || node == endNode) continue;

				GraphConnection2D* newConnection{ new GraphConnection2D(endNode->GetIndex(), nodeIdx, Distance(endPos, node->GetPosition())) };

				if (pClonedGraph->IsUniqueConnection(newConnection->GetFrom(), newConnection->GetTo()))
				{
					pClonedGraph->AddConnection(newConnection);
				}
				else SAFE_DELETE(newConnection)
			}
			auto pathFinder{ AStar<NavGraphNode, GraphConnection2D>(pClonedGraph.get(), HeuristicFunctions::Manhattan) };
			const auto path{ pathFinder.FindPath(startNode, endNode) };

			for (const auto& node : path)
			{
				finalPath.push_back(node->GetPosition());
			}

			debugNodePositions = finalPath;

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto m_Portals{ SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon()) };
			finalPath = SSFA::OptimizePortals(m_Portals);

			return finalPath;
		}
	};
}
