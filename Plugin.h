#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h"
#include "EDecisionMaking.h"
#include "SteeringBehaviors.h"

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
	IExamInterface* m_pInterface = nullptr;

	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
	std::vector<HouseInfo> m_VHouseInfo;
	std::vector<EntityInfo> m_VEntityInfo;

	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	// agent stuff
	Elite::Vector2 m_Target = {};
	AgentInfo* m_pAgentInfo = nullptr;

	// behavior stuf
	Elite::IDecisionMaking* m_pDesitionMaking = nullptr;
	
	// steering stuf
	Seek* m_pSeek = nullptr;
	Wander* m_pWander = nullptr;
	Flee* m_pFlee = nullptr;
	Arrive* m_pArrive = nullptr;
	Face* m_pFace = nullptr;
	Evade* m_pEvade = nullptr;
	Pursuit* m_pPursuit = nullptr;

	ISteeringBehavior* m_pSteeringBehaviour = nullptr;
	ISteeringBehavior* m_pAngularBehaviour = nullptr;
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