#pragma once
#include "Game/Path.hpp"
#include "Game/TimingJudgement.hpp"
#include "Engine/Core/Rgba8.hpp"


//----------------------------------------------------------------------------------------------------------
class TapManager;
class Conductor;


#define MAX_PLANETS 2	// In case I want to add three or four planet modes later
//----------------------------------------------------------------------------------------------------------
struct PlanetSettings
{
	Rgba8 m_planetColors[MAX_PLANETS];
	float m_planetRadius = .2f;
};


//----------------------------------------------------------------------------------------------------------
class PlayerPlanets
{
public:
	PlayerPlanets( Path const& path, Conductor const& conductor, PlanetSettings const& settings );
	~PlayerPlanets();

	void Update();
	void Render() const;

	void HandleTap( TimingJudgement judgement );
	void GoToNextNode();

	Vec2 const& GetPosition() const;
	Vec2 const& GetPositionAhead( int nodeLookahead = 1 ) const;

private:
	void Overload();
	void Die();

	PathNode const* GetPreviousNode() const;
	PathNode const* GetCurrentNode() const;
	PathNode const* GetNextNode() const;

private:
	Path const& m_path;
	Conductor const& m_conductor;
	TapManager* m_input = nullptr;
	PlanetSettings m_settings;
	Vec2 m_position = Vec2::ZERO;
	int m_planetCount = 2;		// Could theoretically change to 3 or 4-planet modes later on
	int m_currentPlanet = 0;
	int m_currentNodeIndex = -1;
	int m_overloadCount = 0;
	int m_judgementCounts[(int)TimingJudgement::COUNT];

	float m_angle = 0;	// The angle from the stationary planet to the next orbiting planet against Vec2::RIGHT
	bool m_clockwise = true;
	bool m_active = true;
	bool m_isDead = false;
};
