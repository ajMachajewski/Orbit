#pragma once
#include "Engine/Renderer/Camera.hpp"


//----------------------------------------------------------------------------------------------------------
class GameCamera : public Camera
{
public:
	void Update();
	void Reset();

public:
	Vec2 m_targetPosition = Vec2::ZERO;

private:
	Vec2 m_velocity = Vec2::ZERO;
	float m_accelerationPerDist = .15f;
	float m_accelerationPerDistSq = .3f;
	float m_yBias = 2.f;
	float m_dragPerSpeed = -5.f;
}; 