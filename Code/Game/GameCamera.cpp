#include "Game/GameCamera.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"


//----------------------------------------------------------------------------------------------------------
void GameCamera::Update()
{
	AABB2 bounds = GetBoundingBox();
	Vec2 displacement = m_targetPosition - Vec2::CopyVec3XY( m_position );
	displacement.y *= m_yBias;
	float distance = displacement.GetLength();
	Vec2 directionToTarget = distance > 0.0001f ? displacement / distance : Vec2::ZERO;

	float accelerationMagnitude = m_accelerationPerDist * distance + m_accelerationPerDistSq * distance * distance;
	Vec2 acceleration = directionToTarget * accelerationMagnitude;
	Vec2 drag = m_dragPerSpeed * m_velocity;
	Vec2 totalAcceleration = drag + acceleration;

	float deltaTime = static_cast<float>( GetGameClock()->GetDeltaSeconds() );
	m_velocity += totalAcceleration * deltaTime;
	m_position += m_velocity * deltaTime;
	SetOrthoView( bounds, m_orthographicNear, m_orthographicFar );
}


//----------------------------------------------------------------------------------------------------------
void GameCamera::Reset()
{
	m_velocity = Vec2::ZERO;
	m_position = Vec3( m_targetPosition, 0.f );
}

