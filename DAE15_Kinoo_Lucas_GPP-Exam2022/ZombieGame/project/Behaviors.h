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
#include "InventoryManager.h"
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

		return Elite::BehaviorState::Failure; //no pistol was found and thus none was grabbed
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

		return Elite::BehaviorState::Failure; //no pistol was found and thus none was grabbed
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
			const float distance{ (item.Location - agentInfo.Position).Magnitude() };
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

			if (Elite::DistanceSquared(itemInfo.Location, pExamInterface->Agent_GetInfo().Position) < pExamInterface->Agent_GetInfo().GrabRange * pExamInterface->Agent_GetInfo().GrabRange)
			{
				std::cout << "Item in grab-range\n";
				return true;
			}
		}
		return false;
	}

	bool ShouldPickupPistol(Elite::Blackboard* pBlackboard)
	{
		// Check if the agent has a pistol
		IExamInterface* pExamInterface{};

		if (!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		bool hasPistol{ false };

		ItemInfo itemInfo;
		// Check if the	pistol slot is empty
		if (!pExamInterface->Inventory_GetItem(PISTOL_SLOT, itemInfo))
		{
			if (itemInfo.Type == eItemType::PISTOL)
			{
				hasPistol = true;
			}
		}

		// If he has a pistol, check if the pistol in the inventory is better than the one in the FOV
		if (hasPistol)
		{
			std::vector<EntityInfo>* pItemsInFov{};
			if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov))
			{
				return false;
			}

			// Loop through all the items in the FOV
			for (const auto& item : *pItemsInFov)
			{
				pExamInterface->Item_GetInfo(item, itemInfo);

				// If the item is a pistol, check if it's better than the one in the inventory
				if (itemInfo.Type == eItemType::PISTOL)
				{
					ItemInfo pistolInInventory{};
					pExamInterface->Inventory_GetItem(PISTOL_SLOT, pistolInInventory);

					// If the pistol in the FOV is better, return true
					if (pExamInterface->Weapon_GetAmmo(pistolInInventory) < pExamInterface->Weapon_GetAmmo(itemInfo))
					{
						std::cout << "Pistol in FOV is better\n";
						return true;
					}
				}
			}
			// If the pistol in the inventory is better, return false
			return false;
		}
		return true;
	}

	bool ShouldPickupShotgun(Elite::Blackboard* pBlackboard)
	{
		// Check if the agent has a shotgun
		IExamInterface* pExamInterface{};

		if (!pBlackboard->GetData(EXAM_ITERFACE, pExamInterface))
		{
			return false;
		}

		bool hasShotgun{ false };

		ItemInfo itemInfo;
		// Check if the	shotgun slot is empty
		if (!pExamInterface->Inventory_GetItem(SHOTGUN_SLOT, itemInfo))
		{
			if (itemInfo.Type == eItemType::SHOTGUN)
			{
				hasShotgun = true;
			}
		}

		// If he has a shotgun, check if the shotgun in the inventory is better than the one in the FOV
		if (hasShotgun)
		{
			std::vector<EntityInfo>* pItemsInFov{};
			if (!pBlackboard->GetData(ITEMS_IN_FOV, pItemsInFov))
			{
				return false;
			}

			// Loop through all the items in the FOV
			for (const auto& item : *pItemsInFov)
			{
				pExamInterface->Item_GetInfo(item, itemInfo);

				// If the item is a shotgun, check if it's better than the one in the inventory
				if (itemInfo.Type == eItemType::SHOTGUN)
				{
					ItemInfo shotgunInInventory{};
					pExamInterface->Inventory_GetItem(SHOTGUN_SLOT, shotgunInInventory);

					// If the shotgun in the FOV is better, return true
					if (pExamInterface->Weapon_GetAmmo(shotgunInInventory) < pExamInterface->Weapon_GetAmmo(itemInfo))
					{
						std::cout << "Shotgun in FOV is better\n";
						return true;
					}
				}
			}
			// If the shotgun in the inventory is better, return false
			return false;
		}
		return true;
	}
}
#endif
