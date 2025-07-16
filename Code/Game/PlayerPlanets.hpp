#pragma once
#include "Game/Level.hpp"
#include "Game/TimingJudgement.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"


//----------------------------------------------------------------------------------------------------------
class TapManager;
class Conductor;
class PathNode;


#define MAX_PLANETS 2	// In case I want to add three or four planet modes later
//----------------------------------------------------------------------------------------------------------
struct PlanetSettings
{
	Rgba8 m_planetColors[MAX_PLANETS];
	float m_planetRadius = .25f;
};


//----------------------------------------------------------------------------------------------------------
class PlayerPlanets
{
	friend class Level;

public:
	PlayerPlanets( Level& level, Conductor const& conductor, PlanetSettings const& settings, int startingIndex = 0 );
	~PlayerPlanets();

	void Update();
	void Render() const;

	void Enable();
	void Disable();

	void HandleTap( TimingJudgement judgement );
	void GoToNextNode();

	unsigned int GetNodeIndex() const;
	Vec2 const& GetPosition() const;
	Vec2 const& GetPositionAhead( int nodeLookahead = 1 ) const;
	Vec2 GetOrbitingPlanetPosition() const;

private:
	void Overload();
	void Die();

	PathNode const* GetPreviousNode() const;
	PathNode const* GetCurrentNode() const;
	PathNode const* GetNextNode() const;

public:
	PlanetSettings m_settings;

private:
	Level& m_level;
	int m_planetCount = 2;		// Could theoretically change to 3 or 4-planet modes later on
	Path const& m_path;
	Conductor const& m_conductor;
	Vec2 m_position = Vec2::ZERO;
	int m_currentPlanet = 0;
	int m_currentNodeIndex = -1;
	int m_overloadCount = 0;
	int m_judgementCounts[(int)TimingJudgement::COUNT];

	float m_angle = 0;	// The angle from the stationary planet to the next orbiting planet against Vec2::RIGHT
	bool m_clockwise = true;
	bool m_active = true;
	bool m_isDead = false;
};
