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
#include "SteeringHelpers.h"
class SteeringAgent;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetEvadeRadius(float evadeRadius) { m_EvadeRadius = evadeRadius; };

private:
	float m_EvadeRadius = 20.f;
};
#endif


