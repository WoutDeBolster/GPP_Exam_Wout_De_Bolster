//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	// Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0, 1, 0, 0.5 });

	return steering;
}

//FLEE (base> SEEK)
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = { Seek::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity *= -1;

	return steering;
}

//ARRIVE (base> SEEK)
//******
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = { Seek::CalculateSteering(deltaT, pAgent) };
	const float distance = (m_Target.Position - pAgent->GetPosition()).Magnitude();

	steering.LinearVelocity *= distance / m_SlowdownRadius; //Rescale to Max Speed slowing down the closer you come in the radius

	// Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0, 1, 0, 0.5 });

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetAutoOrient(false);

	// calc the angle to face the target
	const Elite::Vector2 targetVector = m_Target.Position - pAgent->GetPosition();
	float angle = atan2(targetVector.y, targetVector.x) - pAgent->GetOrientation();
	angle = Elite::ToDegrees(angle);
	angle += 90.f;

	steering.AngularVelocity = Elite::Clamp(angle, -pAgent->GetMaxAngularSpeed(), pAgent->GetMaxAngularSpeed());

	// Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0, 1, 0, 0.5 });

	return steering;
}

//WANDER
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	m_WanderingCirPos = { pAgent->GetPosition() + (pAgent->GetDirection() * (m_WanderCirRad + 1.f)) };

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

	steering.LinearVelocity = m_FocusPoint - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0,1,0,0.5f }, 0.4f);
		//DEBUGRENDERER2D->DrawCircle( m_WanderingCirPos, m_WanderCirRad, { 0,0,1,0.5f }, 0.4f );
		//DEBUGRENDERER2D->DrawCircle( m_FocusPoint, 0.5f, { 1,0,0,0.8f }, 0.4f );
	}

	return steering;
}

//PURSUIT
//*******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const Elite::Vector2 distance = m_Target.Position - pAgent->GetPosition();
	// The longer the distance the higher the pursuitRate
	const float pursuitRate{ distance.Magnitude() / pAgent->GetMaxLinearSpeed() }; 

	// calculating the future position using the speed
	const Elite::Vector2 futurePos = (m_Target).Position + (m_Target).LinearVelocity * pursuitRate; 

	steering.LinearVelocity = futurePos - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//// Debug rendering
	//if (pAgent->CanRenderBehavior())
	//	DEBUGRENDERER2D->DrawPoint(futurePos, 7.f, { 0.5f, 0.f, 0.5f, 0.7f });
	//	DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0, 1, 0, 0.5 });

	return steering;
}

//EVADE
//*****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = Elite::Distance(pAgent->GetPosition(), m_Target.Position);
	if (distanceToTarget > m_EvadeRadius)
		return SteeringOutput(Elite::ZeroVector2, 0.f, false);

	SteeringOutput steering = { Pursuit::CalculateSteering(deltaT, pAgent)};

	steering.LinearVelocity *= -1;

	return steering;
}