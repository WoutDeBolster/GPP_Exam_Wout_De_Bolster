//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_TargetPos - pAgent->Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->MaxLinearSpeed; //Rescale to Max Speed

	return steering;
}

//FLEE (base> SEEK)
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = { Seek::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity *= -1;

	return steering;
}

//ARRIVE (base> SEEK)
//******
SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = { Seek::CalculateSteering(deltaT, pAgent) };
	const float distance = (m_TargetPos - pAgent->Position).Magnitude();

	steering.LinearVelocity *= distance / m_SlowdownRadius; //Rescale to Max Speed slowing down the closer you come in the radius

	return steering;
}

//FACE
//****
SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	// calc the angle to face the target
	const Elite::Vector2 targetVector = m_TargetPos - pAgent->Position;
	float angle = atan2(targetVector.y, targetVector.x) - pAgent->Orientation;
	angle = Elite::ToDegrees(angle);
	angle += 90.f;

	steering.AngularVelocity = Elite::Clamp(angle, -pAgent->MaxAngularSpeed, pAgent->MaxAngularSpeed);

	return steering;
}

//WANDER
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	Elite::Vector2 Agentdirection{ cos(pAgent->Orientation - b2_pi / 2.f), sin(pAgent->Orientation - b2_pi / 2.f) };
	m_WanderingCirPos = { pAgent->Position + (Agentdirection * (m_WanderCirRad + 1.f)) };

	if (m_ChangeTime <= m_PassedTime)
	{
		m_LastFocusPointAngle = m_FocusPointAngle;
		m_FocusPointAngle = ((Elite::randomFloat(m_WanderAngle)) - (m_WanderAngle / 2.f)) + m_LastFocusPointAngle;

		m_FocusPoint = { (m_WanderCirRad * cos(m_FocusPointAngle)) + m_WanderingCirPos.x ,
						(m_WanderCirRad * sin(m_FocusPointAngle)) + m_WanderingCirPos.y };
		m_PassedTime = 0;
	}
	else
	{
		m_PassedTime += deltaT;
	}

	steering.LinearVelocity = m_FocusPoint - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//PURSUIT
//*******
SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	const Elite::Vector2 distance = m_TargetPos - pAgent->Position;
	// The longer the distance the higher the pursuitRate
	const float pursuitRate{ distance.Magnitude() / pAgent->MaxLinearSpeed };

	// calculating the future position using the speed
	const Elite::Vector2 futurePos = m_TargetPos + m_TargetLinVel * pursuitRate;

	steering.LinearVelocity = futurePos - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//EVADE
//*****
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto distanceToTarget = Elite::Distance(pAgent->Position, m_TargetPos);
	if (distanceToTarget > m_EvadeRadius)
		return SteeringPlugin_Output();

	SteeringPlugin_Output steering = { Pursuit::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity *= -1;

	return steering;
}

//SCOUT
//*****
SteeringPlugin_Output Scout::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.AngularVelocity = pAgent->MaxAngularSpeed;

	return steering;
}
