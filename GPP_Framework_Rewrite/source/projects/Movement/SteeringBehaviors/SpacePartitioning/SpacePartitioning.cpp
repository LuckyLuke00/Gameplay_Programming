#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = Elite::Vector2{ left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellWidth = width / cols;
	m_CellHeight = height / rows;

	// Create cells
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			float left = col * m_CellWidth;
			float bottom = row * m_CellHeight;

			m_Cells.emplace_back(left, bottom, m_CellWidth, m_CellHeight);
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	// Adds an agent to the appropriate Cell
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	// 1. Get the index of the cell the agent was in before the update
	int oldCellIdx = PositionToIndex(oldPos);

	// 2. Get the index of the cell the agent is currently in
	int currentCellIdx = PositionToIndex(agent->GetPosition());

	// 3. If the agent is in a different cell than before, move it to the new one
	if (oldCellIdx != currentCellIdx)
	{
		m_Cells[oldCellIdx].agents.remove(agent);
		m_Cells[currentCellIdx].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	// Find out which Cells are in the agent's neighborhood
	// Get indeces of the cells that are in the agent's neighborhood
	auto indices = GetNearbyCells(agent->GetPosition(), queryRadius);

	// Register the agents of the nearby cells
	m_NrOfNeighbors = 0;
	for (int idx : indices)
	{
		for (SteeringAgent* pAgent : m_Cells[idx].agents)
		{
			if (pAgent != agent)
			{
				m_Neighbors[m_NrOfNeighbors] = pAgent;
				++m_NrOfNeighbors;
			}
		}
	}
}

std::vector<int> CellSpace::GetNearbyCells(const Elite::Vector2 pos, float queryRadius) const
{
	// Find out which Cells are in the agent's neighborhood
	// Get indexes of the cells that are in the agent's neighborhood
	int left{ static_cast<int>(floor((pos.x - queryRadius) / m_CellWidth)) };
	int right{ static_cast<int>(ceil((pos.x + queryRadius) / m_CellWidth)) };
	int top{ static_cast<int>(floor((pos.y - queryRadius) / m_CellHeight)) };
	int bottom{ static_cast<int>(ceil((pos.y + queryRadius) / m_CellHeight)) };

	// Clamp indexes so they don't go out of bounds
	left =Elite::Clamp(left, 0, m_NrOfCols - 1);
	right = Elite::Clamp(right, 0, m_NrOfCols - 1);
	top = Elite::Clamp(top, 0, m_NrOfRows - 1);
	bottom = Elite::Clamp(bottom, 0, m_NrOfRows - 1);

	// Get the cell indexes in the neighborhood
	std::vector<int> cellIndexes;
	cellIndexes.reserve(m_NrOfRows * m_NrOfCols);
	for (int row = top; row <= bottom; ++row)
	{
		for (int col = left; col <= right; ++col)
		{
			cellIndexes.push_back(row * m_NrOfCols + col);
		}
	}

	return cellIndexes;
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (const auto& c : m_Cells)
	{
		auto rectPoints = c.GetRectPoints();
		DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), { .5f, 0.f, 0.f, 0.5f }, 0.f);

		DEBUGRENDERER2D->DrawString(c.GetRectPoints()[1], std::to_string(c.agents.size()).c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	// Returns the index of the Cell the position is in
	int row{ static_cast<int>(pos.y / m_CellHeight) };
	int col{ static_cast<int>(pos.x / m_CellWidth) };

	Elite::ClampRef(row, 0, m_NrOfRows - 1);
	Elite::ClampRef(col, 0, m_NrOfRows - 1);

	return abs(row * m_NrOfCols + col);
}