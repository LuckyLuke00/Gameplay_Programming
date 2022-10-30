#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		std::vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree
		auto nodes = m_pGraph->GetAllNodes();
		int oddCount = 0;

		for (const auto& n : nodes)
		{
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() & 1) //eindigt het op een 1 (binair)
			{
				++oddCount;
			}
		}
		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if (oddCount == 2 && nodes.size() != 2)
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		auto path = std::vector<T_NodeType*>();
		int nrOfNodes = graphCopy->GetNrOfNodes();

		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		// Else we need to find a valid starting index for the algorithm

		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		auto path = std::vector<T_NodeType>();
		int nrOfNodes = graphCopy->GetNrOfNodes();

		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		// Else we need to find a valid starting index for the algorithm
		int currentNodeIndex = invalid_node_index;

		auto nodes = m_pGraph->GetAllNodes();

		if (eulerianity == Eulerianity::eulerian)
		{
			for (const auto& n : nodes)
			{
				currentNodeIndex = n->GetIndex();
			}
		}
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			for (const auto& n : nodes)
			{
				auto connections = m_pGraph->GetNodeConnections(n);

				if (connections.size() & 1) //heeft oneven aantal connecties
				{
					currentNodeIndex = n->GetIndex();
					break;
				}
			}
		}
		else
		{
			return path;
		}

		// Start algorithm loop
		std::stack<int> nodeStack;

		while ((graphCopy->GetNodeConnections(currentNodeIndex).size() != 0) || (nodeStack.size() != 0))
		{
			auto connections = graphCopy->GetNodeConnections(currentNodeIndex);
			if (connections.size() != 0)
			{
				nodeStack.push(currentNodeIndex);

				currentNodeIndex = connections.back()->GetTo();
				graphCopy->RemoveConnection(nodeStack.top(), currentNodeIndex);
			}
			else
			{
				path.push_back(m_pGraph->GetNode(currentNodeIndex));
				currentNodeIndex = nodeStack.top();
				nodeStack.pop();
			}
		}

		path.push_back(m_pGraph->GetNode(currentNodeIndex));

		std::reverse(path.begin(), path.end()); // reverses order of the path
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const
	{
		//TODO: Test
		// mark the visited node
		visited[startIdx] = true;

		// visit all unvisited neighbours
		auto connections = m_pGraph->GetNodeConnections(startIdx);
		for (const auto& c : connections)
		{
			if (!visited[c->GetTo()])
			{
				VisitAllNodesDFS(c->GetTo(), visited);
			}
		}

		// recursively visit any valid connected nodes that were not visited before
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(m_pGraph->GetNrOfNodes(), false);

		// find a valid starting node that has connections
		// if no valid node could be found, return false
		// start a depth-first-search traversal from the node that has at least one connection
		// if a node was never visited, this graph is not connected

		auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(m_pGraph->GetNrOfNodes(), false);

		// find a valid starting node that has connections
		int connectedIndex = invalid_node_index;

		for (const auto& n : nodes)
		{
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() != 0)
			{
				connectedIndex = n->GetIndex();
				break;
			}
		}

		// if no valid node could be found, return false
		if (connectedIndex == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from the node that has at least one connection
		VisitAllNodesDFS(connectedIndex, visited);

		// if a node was never visited, this graph is not connected
		for (const auto& n : nodes)
		{
			if (visited[n->GetIndex()] == false)
			{
				return false;
			}
		}

		return true;
	}
}