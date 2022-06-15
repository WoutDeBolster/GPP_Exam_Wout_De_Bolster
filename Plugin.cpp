#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behaviors.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "BotNameTEST";
	info.Student_FirstName = "Foo";
	info.Student_LastName = "Bar";
	info.Student_Class = "2DAEx";

	Elite::Blackboard* pB = new Elite::Blackboard();

	//Add data to blackboard
	pB->AddData("Target", m_Target);

	//Steering Behaviors
	pB->AddData("Seek", m_pSeek);
	pB->AddData("Wander", m_pWander);
	pB->AddData("Flee", m_pFlee);
	pB->AddData("Arrive", m_pArrive);
	pB->AddData("Face", m_pFace);
	pB->AddData("Evade", m_pEvade);
	pB->AddData("Pursuit", m_pPursuit);
	pB->AddData("Scout", m_pScout);

	pB->AddData("Steering", static_cast<ISteeringBehavior**>(&m_pSteeringBehaviour));
	pB->AddData("Angular", static_cast<ISteeringBehavior**>(&m_pAngularBehaviour));

	// other things
	pB->AddData("Agent", static_cast<AgentInfo*>(m_pAgentInfo));

	pB->AddData("Houses", static_cast<vector<HouseInfo>*>(&m_VHouseInfo));
	pB->AddData("Entities", static_cast<vector<EntityInfo>*>(&m_VEntityInfo));

	pB->AddData("Interface", m_pInterface);

	// empty stuff
	pB->AddData("ClosestHouse", static_cast<HouseInfo*>(nullptr));
	pB->AddData("ClosestEnemy", static_cast<EnemyInfo*>(nullptr));
	pB->AddData("ClosestItem", static_cast<ItemInfo*>(nullptr));
	pB->AddData("ClosestPurgeZone", static_cast<PurgeZoneInfo*>(nullptr));
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	AgentInfo* pWally{ &m_pInterface->Agent_GetInfo() };
	m_pAgentInfo = pWally;

	// steering init
	m_pSeek = new Seek();
	m_pWander = new Wander();
	m_pFlee = new Flee();
	m_pArrive = new Arrive();
	m_pFace = new Face();
	m_pEvade = new Evade();
	m_pPursuit = new Pursuit();
	m_pEvade = new Evade();
	m_pScout = new Scout();

	m_pSteeringBehaviour = m_pWander;
	m_pAngularBehaviour = m_pFace;

	Elite::Blackboard* pB = new Elite::Blackboard();
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
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
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	auto nextTargetPos = m_Target; //To start you can use the mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
		}
	}


	//INVENTORY USAGE DEMO
	//********************

	if (m_GrabItem)
	{
		ItemInfo item;
		//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
		//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
		//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
		//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
		if (m_pInterface->Item_Grab({}, item))
		{
			//Once grabbed, you can add it to a specific inventory slot
			//Slot must be empty
			m_pInterface->Inventory_AddItem(0, item);
		}
	}

	if (m_UseItem)
	{
		//Use an item (make sure there is an item at the given inventory slot)
		m_pInterface->Inventory_UseItem(0);
	}

	if (m_RemoveItem)
	{
		//Remove an item from a inventory slot
		m_pInterface->Inventory_RemoveItem(0);
	}

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

								 //SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

								 //@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
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
