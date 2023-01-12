#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

namespace Elite
{
	class BehaviorTree;
	class Blackboard;
}

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface{ nullptr };
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target{};
	bool m_CanRun{ false }; //Demo purpose
	bool m_GrabItem{ false }; //Demo purpose
	bool m_UseItem{ false }; //Demo purpose
	bool m_RemoveItem{ false }; //Demo purpose
	float m_AngSpeed{ .0f }; //Demo purpose

	UINT m_InventorySlot{ 0 };

	Elite::Vector2 m_WorldDimensions{};

	Elite::BehaviorTree* m_pBehaviorTree;
	Elite::Blackboard* m_pBlackboard;

	std::vector<EnemyInfo> m_EnemiesInFOV{};
	std::vector<EntityInfo> m_ItemsInFOV{};
	std::vector<HouseInfo> m_HousesInFOV{};

	void InitBlackboardData();

	// Function that updates the vectors of enemies, entities and houses in the field of view
	void UpdateFOVItems();
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}
