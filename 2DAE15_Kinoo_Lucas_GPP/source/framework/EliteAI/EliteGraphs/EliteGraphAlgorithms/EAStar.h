#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path{}; // Final Path
		std::vector<NodeRecord> openList{}; // Conncections to be checked
		std::vector<NodeRecord> closedList{}; // Connections already checked
		NodeRecord currentRecord{}; // Holds the current to be evaluated connection

		openList.emplace_back(NodeRecord{ pStartNode, nullptr, 0.f, GetHeuristicCost(pStartNode, pGoalNode) }); // Add the start node to the open list

		while (!openList.empty())
		{
			// We will continue searching for a connection that leads to the end node
			// A Get connection with lowest F score
			// Set the currentRecord to the best record from the openList (the one with the lowest Fcost)
			currentRecord = *std::min_element(openList.begin(), openList.end());
			if (currentRecord.pNode == pGoalNode) break;

			// Get all the connections from the currentRecord and loop over them
			for (const auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				// Calculate the total cost so far (G-cost) (we will need it further on)
				float totalCostSoFar{ currentRecord.costSoFar + connection->GetCost() };

				// Check if any of those connections lead to a node already on the closed list
				// Check the closed list (pNode) and if a connection to the connections endNode already exist in the closed list
				NodeRecord existingRecord{};

				for (const auto& record : closedList)
				{
					if (record.pNode == m_pGraph->GetNode(connection->GetTo()))
					{
						existingRecord = record;
						break;
					}
				}

				// Check if the already existing connection is cheaper (tip: use calculated G-Cost)
				// If so, continue to the next connection
				// Check if existingrecord is not empty
				if (existingRecord.pNode != nullptr && existingRecord.costSoFar <= totalCostSoFar)
				{
					continue;
				}
				else
				{
					closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord), closedList.end());
				}

				if (existingRecord.pNode == nullptr)
				{
					for (const auto& record : openList)
					{
						if (record.pNode == m_pGraph->GetNode(connection->GetTo()))
						{
							existingRecord = record;
							break;
						}
					}
				}

				if (existingRecord.pNode != nullptr && existingRecord.costSoFar <= totalCostSoFar)
				{
					continue;
				}
				else
				{
					openList.erase(std::remove(openList.begin(), openList.end(), existingRecord), openList.end());
				}

				// At this point any expensive connection should be removed (if it existed). We create a new nodeRecordand add it to the openList
				openList.emplace_back(NodeRecord{ m_pGraph->GetNode(connection->GetTo()), connection, totalCostSoFar, totalCostSoFar + GetHeuristicCost(m_pGraph->GetNode(connection->GetTo()), pGoalNode) });
			}
			// Remove NodeRecord from the openList and add it to the closedList
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.emplace_back(currentRecord);
		}

		// Reconstruct path from last connection to start node
		// Track back from the currentRecord until the node of the record is the startnode of the overall path
		while (currentRecord.pNode != pStartNode)
		{
			// Add each time the node of the currentRecord to the path
			path.emplace_back(currentRecord.pNode);
			// Look in the closedList for a record where pNode == the currentRecords’ connections’ startNode(tip: the fromIndex)
			for (const auto& record : closedList)
			{
				if (record.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
				{
					currentRecord = record;
					break;
				}
			}
		}
		// Add the startnode’s position to the vPath
		path.emplace_back(pStartNode);
		// Reverse vPath and return it
		std::reverse(path.begin(), path.end());
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}
