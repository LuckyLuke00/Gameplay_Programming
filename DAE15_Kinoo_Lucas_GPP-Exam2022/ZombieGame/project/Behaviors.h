#ifndef BEHAVIORS_H
#define BEHAVIORS_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "BlackboardData.h"
#include "Constants.h"
#include "EBehaviorTree.h"
#include "EliteMath/EMath.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState Seek(Elite::Blackboard* pBlackboard)
	{
		AgentInfo agentInfo{};
		Elite::Vector2 destination{};
		IExamInterface* pExamInterface{};
		SteeringPlugin_Output steering{};
		bool spinRound{ false };

		if (!pBlackboard->GetData(DESTINATION, destination) ||
			!pBlackboard->GetData(AGENT_INFO, agentInfo) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface) ||
			!pBlackboard->GetData(SPIN_ROUND, spinRound))
		{
			return Elite::BehaviorState::Failure;
		}

		destination = pExamInterface->NavMesh_GetClosestPathPoint(destination);

		// Draw a point at the destination
		pExamInterface->Draw_Point(destination, 5.f, { 1.f, .0f, .0f });

		// Prevent the agent from running, as to not deplete the stamina
		steering.RunMode = false;

		// Spin the agent to check for enemies
		if (spinRound)
		{
			steering.AutoOrient = false;
			steering.AngularVelocity = agentInfo.MaxAngularSpeed;
		}

		// Seek the destination
		steering.LinearVelocity = (destination - agentInfo.Position).GetNormalized() * agentInfo.MaxLinearSpeed;

		pBlackboard->ChangeData(STEERING_OUTPUT, steering);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
	{
		// Set a random destination on the navmesh
		AgentInfo agentInfo{};
		IExamInterface* pExamInterface{};
		SteeringPlugin_Output steering{};
		bool spinRound{ false };

		if (!pBlackboard->GetData(AGENT_INFO, agentInfo) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface) ||
			!pBlackboard->GetData(SPIN_ROUND, spinRound))
		{
			return Elite::BehaviorState::Failure;
		}

		// Prevent the agent from running, as to not deplete the stamina
		steering.RunMode = false;

		// Set the spinround bool to true
		pBlackboard->ChangeData(SPIN_ROUND, true);

		// Set a random destination on the navmesh
		const Elite::Vector2 randomDestination
		{
			Elite::Vector2
			{
				Elite::randomFloat(.0f, pExamInterface->World_GetInfo().Dimensions.x * .5f),
				Elite::randomFloat(.0f, pExamInterface->World_GetInfo().Dimensions.y * .5f)
			}
		};

		pBlackboard->ChangeData(DESTINATION, randomDestination);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState PickUpPistol(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};
		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		for (const auto& item : *pItemsInFov)
		{
			ItemInfo itemInfo;
			pExamInterface->Item_GetInfo(item, itemInfo);

			if (itemInfo.Type != eItemType::PISTOL) continue;

			pExamInterface->Item_Grab(item, itemInfo);
			pExamInterface->Inventory_AddItem(PISTOL_SLOT, itemInfo);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState PickUpShotgun(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};
		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		for (const auto& item : *pItemsInFov)
		{
			ItemInfo itemInfo;
			pExamInterface->Item_GetInfo(item, itemInfo);

			if (itemInfo.Type != eItemType::SHOTGUN) continue;

			pExamInterface->Item_Grab(item, itemInfo);
			pExamInterface->Inventory_AddItem(SHOTGUN_SLOT, itemInfo);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState PickUpMedkit(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};
		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		for (const auto& item : *pItemsInFov)
		{
			ItemInfo itemInfo;
			pExamInterface->Item_GetInfo(item, itemInfo);

			if (itemInfo.Type != eItemType::MEDKIT) continue;

			pExamInterface->Item_Grab(item, itemInfo);
			pExamInterface->Inventory_AddItem(MEDKIT_SLOT, itemInfo);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState PickUpFood(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};
		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		for (const auto& item : *pItemsInFov)
		{
			ItemInfo itemInfo;
			pExamInterface->Item_GetInfo(item, itemInfo);

			if (itemInfo.Type != eItemType::FOOD) continue;

			pExamInterface->Item_Grab(item, itemInfo);
			pExamInterface->Inventory_AddItem(FOOD_SLOT, itemInfo);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState SetItemAsTarget(Elite::Blackboard* pBlackboard)
	{
		// Find the item closest to the agent
		AgentInfo agentInfo{};
		Elite::Vector2 destination{};
		EntityInfo targetItem{};
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};

		if (!pBlackboard->GetData(DESTINATION, destination) ||
			!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(AGENT_INFO, agentInfo) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return Elite::BehaviorState::Failure;
		}

		// Find the closest item
		float closestDistance{ FLT_MAX };
		for (const EntityInfo& item : *pItemsInFov)
		{
			const float distance{ Elite::DistanceSquared(item.Location, agentInfo.Position) };
			if (distance < closestDistance)
			{
				closestDistance = distance;
				targetItem = item;
			}
		}

		// Set the destination to the item
		pBlackboard->ChangeData(DESTINATION, targetItem.Location);

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool ReachedDestination(Elite::Blackboard* pBlackboard)
	{
		AgentInfo agentInfo{};
		Elite::Vector2 destination{};
		if (!pBlackboard->GetData(DESTINATION, destination) ||
			!pBlackboard->GetData(AGENT_INFO, agentInfo))
		{
			return false;
		}
		return Elite::DistanceSquared(agentInfo.Position, destination) < 1.f;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};
		bool spinRound{ false };

		if (!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface) ||
			!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(SPIN_ROUND, spinRound))
		{
			return false;
		}

		if (pItemsInFov->empty()) return false;

		pBlackboard->ChangeData(SPIN_ROUND, false);

		//std::cout << "Item is in fov\n";

		return true;
	}

	bool IsItemInGrabRange(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pItemsInFov{};
		IExamInterface* pExamInterface{};

		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		ItemInfo itemInfo;
		for (const auto& item : *pItemsInFov)
		{
			if (!pExamInterface->Item_GetInfo(item, itemInfo)) continue;

			if (Elite::DistanceSquared(itemInfo.Location, pExamInterface->Agent_GetInfo().Position) <
				pExamInterface->Agent_GetInfo().GrabRange * pExamInterface->Agent_GetInfo().GrabRange)
			{
				return true;
			}
		}
		return false;
	}

	bool ShouldPickupPistol(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};

		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		ItemInfo itemInfo{};
		for (const auto& item : *pItemsInFov)
		{
			if (!pExamInterface->Item_GetInfo(item, itemInfo)) continue;

			// Continue to the next item if the current item is not a pistol
			if (itemInfo.Type != eItemType::PISTOL) continue;

			ItemInfo currentItemInfo{};
			if (!pExamInterface->Inventory_GetItem(PISTOL_SLOT, currentItemInfo)) return true;

			// Return true if the pistol in the inventory is worse than the pistol in the fov
			if (pExamInterface->Weapon_GetAmmo(itemInfo) > pExamInterface->Weapon_GetAmmo(currentItemInfo)) return true;

			// If the pistol in the inventory is better than the pistol in the fov, return false
			return false;
		}
		// If no pistol is in the fov, return false
		return false;
	}

	bool ShouldPickupShotgun(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};

		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		ItemInfo itemInfo{};
		for (const auto& item : *pItemsInFov)
		{
			if (!pExamInterface->Item_GetInfo(item, itemInfo)) continue;

			// Continue to the next item if the current item is not a shotgun
			if (itemInfo.Type != eItemType::SHOTGUN) continue;

			ItemInfo currentItemInfo{};
			if (!pExamInterface->Inventory_GetItem(SHOTGUN_SLOT, currentItemInfo)) return true;

			// Return true if the shotgun in the inventory is worse than the shotgun in the fov
			if (pExamInterface->Weapon_GetAmmo(itemInfo) > pExamInterface->Weapon_GetAmmo(currentItemInfo)) return true;

			// If the shotgun in the inventory is better than the shotgun in the fov, return false
			return false;
		}
		// If no shotgun is in the fov, return false
		return false;
	}

	bool ShouldPickupMedkit(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};

		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		ItemInfo itemInfo{};
		for (const auto& item : *pItemsInFov)
		{
			if (!pExamInterface->Item_GetInfo(item, itemInfo)) continue;

			// Continue to the next item if the current item is not a medkit
			if (itemInfo.Type != eItemType::MEDKIT) continue;

			ItemInfo currentItemInfo{};
			if (!pExamInterface->Inventory_GetItem(MEDKIT_SLOT, currentItemInfo)) return true;

			// Return true if the medkit in the inventory is worse than the medkit in the fov
			if (pExamInterface->Medkit_GetHealth(itemInfo) > pExamInterface->Medkit_GetHealth(currentItemInfo)) return true;

			// If the medkit in the inventory is better than the medkit in the fov, return false
			return false;
		}
		// If no medkit is in the fov, return false
		return false;
	}

	bool ShouldPickupFood(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface{};
		std::vector<EntityInfo>* pItemsInFov{};

		if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov) ||
			!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		ItemInfo itemInfo{};
		for (const auto& item : *pItemsInFov)
		{
			if (!pExamInterface->Item_GetInfo(item, itemInfo)) continue;

			// Continue to the next item if the current item is not a food
			if (itemInfo.Type != eItemType::FOOD) continue;

			ItemInfo currentItemInfo{};
			if (!pExamInterface->Inventory_GetItem(FOOD_SLOT, currentItemInfo)) return true;

			// Return true if the food in the inventory is worse than the food in the fov
			if (pExamInterface->Food_GetEnergy(itemInfo) > pExamInterface->Food_GetEnergy(currentItemInfo)) return true;

			// If the food in the inventory is better than the food in the fov, return false
			return false;
		}
		// If no food is in the fov, return false
		return false;
	}
}
#endif
