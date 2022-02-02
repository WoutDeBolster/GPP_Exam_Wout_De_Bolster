/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------

#include "Exam_HelperStructs.h"

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	//Seek Functions
	void SetTargetPos(const Elite::Vector2& target) { m_TargetPos = target; }
	void SetTargetLinVel(Elite::Vector2 target) { m_TargetLinVel = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	Elite::Vector2 m_TargetPos;
	Elite::Vector2 m_TargetLinVel;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

///////////////////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

///////////////////////////////////////
//ARRIVE
//******
class Arrive final : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetSlowRadius(float slowRadius) { m_SlowdownRadius = slowRadius; };
private:
	float m_SlowdownRadius = 3.0f;
};

///////////////////////////////////////
//FACE
//****
class Face final : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

///////////////////////////////////////
//WANDER
//******
class Wander final : public ISteeringBehavior
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
private:
	float m_ChangeTime = 1.f;
	float m_PassedTime = 0.f;
	float m_WanderCirRad = 5.f;
	float m_WanderAngle = Elite::ToRadians(180); // WanderRadius
	Elite::Vector2 m_WanderingCirPos = { 0.f, 0.f }; // Internal angle
	float m_FocusPointAngle = 0.f;
	float m_LastFocusPointAngle = 0.f;
	Elite::Vector2 m_FocusPoint = { 0.f, 0.f };
};

///////////////////////////////////////
//PURSUIT
//*******
class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetSlowRadius(float slowRadius) { m_SlowdownRadius = slowRadius; };
private:
	float m_SlowdownRadius = 3.0f;
};

///////////////////////////////////////
//EVADE
//*****
class Evade : public Pursuit
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetEvadeRadius(float evadeRadius) { m_EvadeRadius = evadeRadius; };

private:
	float m_EvadeRadius = 20.f;
};

///////////////////////////////////////
//SCOUT
//*****
class Scout : public Pursuit
{
public:
	Scout() = default;
	virtual ~Scout() = default;

	//Seek Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetEvadeRadius(float evadeRadius) { m_EvadeRadius = evadeRadius; };

private:
	float m_EvadeRadius = 20.f;
};
#endif


