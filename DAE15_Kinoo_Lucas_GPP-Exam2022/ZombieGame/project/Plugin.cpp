#include "stdafx.h"
#include "Plugin.h"
#include "EBehaviorTree.h"
#include "EBlackboard.h"
#include "IExamInterface.h"
#include "BlackboardData.h"
#include "Behaviors.h"
#include "Structs.h"

using namespace std;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	UpdateFOVItems();

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Openingsuren Bibliotheek";
	info.Student_FirstName = "Lucas";
	info.Student_LastName = "Kinoo";
	info.Student_Class = "2DAE15";

	m_pBlackboard = new Elite::Blackboard();
	InitBlackboardData();

	m_WorldDimensions = m_pInterface->World_GetInfo().Dimensions;

	m_pBehaviorTree = new Elite::BehaviorTree
	{
		m_pBlackboard,
		// --------------- Root ---------------
		new Elite::BehaviorSelector
		{{
				// ---------- Enemy Handling ----------
				new Elite::BehaviorSequence
				{{
					new Elite::BehaviorConditional{ BT_Conditions::IsEnemyInFOV },
					new Elite::BehaviorConditional{ BT_Conditions::IsFacingEnemy },
					new Elite::BehaviorSelector
					{{
							// ----------- Shoot Pistol -----------
							new Elite::BehaviorSequence
							{{
								new Elite::BehaviorConditional{ BT_Conditions::IsPistolFireReady },
								new Elite::BehaviorAction{ BT_Actions::ShootPistol },
							}},
							// ---------- Shoot Shotgun ----------
							new Elite::BehaviorSequence
							{{
								new Elite::BehaviorConditional{ BT_Conditions::IsShotgunFireReady },
								new Elite::BehaviorAction{ BT_Actions::ShootShotgun },
							}},
						}},
					}},
					new Elite::BehaviorSequence
					{{
							// ---------- Manage Vitals ----------
							new Elite::BehaviorSelector
							{{
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::CanHeal },
										new Elite::BehaviorAction{ BT_Actions::Heal },
									}},
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::CanEat },
										new Elite::BehaviorAction{ BT_Actions::Eat },
									}},
							}},
					}},
					new Elite::BehaviorSequence
					{{
							// ------ Find and Pickup Items ------
							new Elite::BehaviorSequence
							{{
								new Elite::BehaviorConditional{ BT_Conditions::IsItemInFOV },
								new Elite::BehaviorAction{ BT_Actions::SetItemAsTarget },
								new Elite::BehaviorAction{ BT_Actions::Seek },
							}},
							new Elite::BehaviorConditional{ BT_Conditions::IsItemInGrabRange },
							new Elite::BehaviorSelector
							{{
									// -------------- Garbage --------------
									new Elite::BehaviorAction{ BT_Actions::RemoveGarbage },
									// -------------- Pistol --------------
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::ShouldPickupPistol },
										new Elite::BehaviorAction{ BT_Actions::PickUpPistol },
									}},
									// -------------- Shotgun --------------
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::ShouldPickupShotgun },
										new Elite::BehaviorAction{ BT_Actions::PickUpShotgun },
									}},
									// -------------- Medkit --------------
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::ShouldPickupMedkit },
										new Elite::BehaviorAction{ BT_Actions::PickUpMedkit },
									}},
									// --------------- Food ---------------
									new Elite::BehaviorSequence
									{{
										new Elite::BehaviorConditional{ BT_Conditions::ShouldPickupFood },
										new Elite::BehaviorAction{ BT_Actions::PickUpFood },
									}},
								}}
							}},
		// -------------- Explore --------------
		new Elite::BehaviorSelector
		{{
				new Elite::BehaviorAction{ BT_Actions::VisitHouses },
				new Elite::BehaviorSequence
				{{
					new Elite::BehaviorConditional{ BT_Conditions::ReachedDestination },
					new Elite::BehaviorAction{ BT_Actions::Explore },
				}},
				new Elite::BehaviorAction{ BT_Actions::Seek },
			}}
		}}
	};
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 36;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	// Clear the vectors
	m_EnemiesInFOV.clear();
	m_ItemsInFOV.clear();
	m_HousesInFOV.clear();

	SteeringPlugin_Output steering{};

	UpdateFOVItems();
	UpdateDiscoveredHouses(dt);

	m_pBehaviorTree->Update(dt);

	m_pBlackboard->GetData(STEERING_OUTPUT, steering);
	m_pBlackboard->ChangeData(AGENT_INFO, m_pInterface->Agent_GetInfo());

	m_GrabItem = false;
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0, 0 }, { 1, 0, 0 });

	// Draw the map's boundaries
	m_pInterface->Draw_Segment({ m_WorldDimensions.x * -.5f, m_WorldDimensions.y * -.5f }, { m_WorldDimensions.x * .5f, m_WorldDimensions.y * -.5f }, { 1, 0, 0 });
	m_pInterface->Draw_Segment({ m_WorldDimensions.x * .5f, m_WorldDimensions.y * -.5f }, { m_WorldDimensions.x * .5f, m_WorldDimensions.y * .5f }, { 1, 0, 0 });
	m_pInterface->Draw_Segment({ m_WorldDimensions.x * .5f, m_WorldDimensions.y * .5f }, { m_WorldDimensions.x * -.5f, m_WorldDimensions.y * .5f }, { 1, 0, 0 });
	m_pInterface->Draw_Segment({ m_WorldDimensions.x * -.5f, m_WorldDimensions.y * .5f }, { m_WorldDimensions.x * -.5f, m_WorldDimensions.y * -.5f }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::InitBlackboardData()
{
	m_pBlackboard->AddData(AGENT_INFO, AgentInfo{});
	m_pBlackboard->AddData(DESTINATION, Elite::Vector2{});
	m_pBlackboard->AddData(DISCOVERED_HOUSES, &m_DiscoveredHouses);
	m_pBlackboard->AddData(ENEMIES_IN_FOV, &m_EnemiesInFOV);
	m_pBlackboard->AddData(EXAM_ITERFACE, m_pInterface);
	m_pBlackboard->AddData(HOUSES_IN_FOV, &m_HousesInFOV);
	m_pBlackboard->AddData(ITEMS_IN_FOV, &m_ItemsInFOV);
	m_pBlackboard->AddData(SPIN_ROUND, true);
	m_pBlackboard->AddData(STEERING_OUTPUT, SteeringPlugin_Output{});
	m_pBlackboard->AddData(TARGET_INFO, Elite::Vector2{});
}

void Plugin::UpdateFOVItems()
{
	const auto vEntitiesInFOV{ GetEntitiesInFOV() };
	const auto vHousesInFOV{ GetHousesInFOV() };

	// Update the items in the FOV
	for (const auto& entity : vEntitiesInFOV)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			m_ItemsInFOV.emplace_back(entity);
			continue;
		}

		if (entity.Type == eEntityType::ENEMY)
		{
			EnemyInfo enemyInfo{};
			m_pInterface->Enemy_GetInfo(entity, enemyInfo);
			m_EnemiesInFOV.emplace_back(enemyInfo);
			continue;
		}
	}

	// Update the houses in the FOV
	for (const auto& house : vHousesInFOV)
	{
		m_HousesInFOV.emplace_back(house);
	}

	AddDiscoveredHouses();
}

void Plugin::AddDiscoveredHouses()
{
	// Loop through all the houses in the FOV, and add them to the discovered houses if they are not already in there
	// Check if they are equal by comparing the center of the house
	for (const auto& house : m_HousesInFOV)
	{
		bool bFound{ false };
		for (const auto& discoveredHouse : m_DiscoveredHouses)
		{
			if (house.Center == discoveredHouse.m_HouseInfo.Center)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			m_DiscoveredHouses.emplace_back(DiscoveredHouse{ house, false });
		}
	}
}

void Plugin::UpdateDiscoveredHouses(float dt)
{
	// Loop through all the discovered houses, and update the time since last seen
	for (auto& discoveredHouse : m_DiscoveredHouses)
	{
		if (!discoveredHouse.m_IsVisited) continue;

		discoveredHouse.m_TimeSinceVisit += dt;

		if (discoveredHouse.m_TimeSinceVisit >= HOUSE_VISIT_COOLDOWN)
		{
			discoveredHouse.m_IsVisited = false;
			discoveredHouse.m_TimeSinceVisit = 0.f;
		}
	}
}
