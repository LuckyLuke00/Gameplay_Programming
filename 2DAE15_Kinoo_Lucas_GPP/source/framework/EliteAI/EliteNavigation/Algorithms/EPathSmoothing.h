#pragma once

#include <vector>
#include "framework/EliteGeometry/EGeometry2DTypes.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"

namespace Elite
{
	//Portal struct (only contains line info atm, you can expand this if needed)
	struct Portal
	{
		Portal() = default;
		explicit Portal(const Elite::Line& line) :
			Line(line)
		{
		}
		Elite::Line Line = {};
	};

	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<Portal> FindPortals(
			const std::vector<NavGraphNode*>& nodePath,
			Polygon const* navMeshPolygon)
		{
			//Container
			std::vector<Portal> vPortals = {};

			vPortals.emplace_back(Line(nodePath[0]->GetPosition(), nodePath[0]->GetPosition()));

			std::vector<Line*> lines = navMeshPolygon->GetLines();

			//For each node received, get its corresponding line
			for (size_t nodeIdx = 1; nodeIdx < nodePath.size() - 1; ++nodeIdx)
			{
				//Local variables
				NavGraphNode const* pNode = nodePath[nodeIdx]; //Store node, except last node, because this is our target node!
				Line const* pLine = lines[pNode->GetLineIndex()];

				//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
				Vector2 centerLine = (pLine->p1 + pLine->p2) / 2.0f;
				Vector2 previousPosition = nodeIdx == 0 ? nodePath[0]->GetPosition() : nodePath[nodeIdx - 1]->GetPosition();

				float cross = Cross((centerLine - previousPosition), (pLine->p1 - previousPosition));

				Line portalLine = {};
				if (cross > 0)//Left
					portalLine = Line(pLine->p2, pLine->p1);
				else //Right
					portalLine = Line(pLine->p1, pLine->p2);

				//Store portal
				vPortals.emplace_back(portalLine);
			}
			//Add degenerate portal to force end evaluation
			vPortals.emplace_back(Line(nodePath[nodePath.size() - 1]->GetPosition(), nodePath[nodePath.size() - 1]->GetPosition()));

			return vPortals;
		}

		static std::vector<Vector2> OptimizePortals(const std::vector<Portal>& portals)
		{
			//P1 == right point of portal, P2 == left point of portal
			std::vector<Vector2> vPath{};
			const unsigned int amtPortals{ portals.size() };

			int apexIdx{ 0 };
			int leftLegIdx{ 1 };
			int rightLegIdx{ 1 };

			Vector2 apexPos{ portals[apexIdx].Line.p1 };
			Vector2 rightLeg{ portals[rightLegIdx].Line.p1 - apexPos };
			Vector2 leftLeg{ portals[leftLegIdx].Line.p2 - apexPos };

			for (unsigned int portalIdx = 1; portalIdx < amtPortals; ++portalIdx)
			{
				//Local
				const auto& portal{ portals[portalIdx] };

				// Right Check
				// Create the new right leg = from the apex to the p1 point of the portal
				Vector2 newRightLeg{ portal.Line.p1 - apexPos };

				// Check if going inwards or not (Tip: Use Cross).
				// If not going inwards, do nothing and just go to the left check.
				if (Cross(rightLeg, newRightLeg) >= 0.0f)
				{
					if (Cross(newRightLeg, leftLeg) > 0.0f)
					{
						rightLeg = newRightLeg;
						rightLegIdx = portalIdx;
					}
					else
					{
						apexPos += leftLeg;
						apexIdx = leftLegIdx;
						portalIdx = leftLegIdx + 1;
						leftLegIdx = portalIdx;
						rightLegIdx = portalIdx;
						vPath.emplace_back(apexPos);

						if (portalIdx < amtPortals)
						{
							// Update the right leg
							rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
							leftLeg = portals[leftLegIdx].Line.p2 - apexPos;
							continue;
						}
					}
				}

				//--- LEFT CHECK ---
				Vector2 newLeftLeg{ portal.Line.p2 - apexPos };
				//1. See if moving funnel inwards - LEFT
				if (Cross(leftLeg, newLeftLeg) <= 0.0f)
				{
					if (Cross(newLeftLeg, rightLeg) < 0.0f)
					{
						leftLeg = newLeftLeg;
						leftLegIdx = portalIdx;
					}
					else
					{
						apexPos += rightLeg;
						apexIdx = rightLegIdx;
						portalIdx = rightLegIdx + 1;
						leftLegIdx = portalIdx;
						rightLegIdx = portalIdx;
						vPath.emplace_back(apexPos);

						if (portalIdx < amtPortals)
						{
							rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
							leftLeg = portals[leftLegIdx].Line.p2 - apexPos;
							continue;
						}
					}
				}
			}

			// Add last path point (You can use the last portal p1 or p2 points as both are equal to the endPoint of the path
			vPath.push_back(portals[amtPortals - 1].Line.p2);

			return vPath;
		}
	private:
		SSFA() = default;
		~SSFA() = default;
	};
}
