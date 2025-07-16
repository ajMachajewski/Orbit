#pragma once
#include "Game/LevelMetrics.hpp"
#include <vector>


//----------------------------------------------------------------------------------------------------------
class Conductor;
class PlayerPlanets;
class Path;
class Prop;
class GameCamera;
class TapManager;
class Timer;
struct PlanetSettings;
struct AABB2;
struct Vec2;


//----------------------------------------------------------------------------------------------------------
enum class LevelState
{
	COUNTDOWN,
	PLAYING,
	FAIL,
	WIN,
	INACTIVE,
};


//----------------------------------------------------------------------------------------------------------
struct LevelInfo
{
	std::string m_name;
	std::string m_source;
	float m_difficulty = 0;
};


//----------------------------------------------------------------------------------------------------------
class Level
{

public:
	Level();
	Level( const char* xmlFilePath ); 
	~Level();

	void LoadFromXML( const char* xmlFilePath );

	void Startup();
	void Update();
	void Render() const;
	void Shutdown();

	void GoToState( LevelState newState );
	void ResetCheckpoints();

	void RenderHUD( AABB2 const& screenBounds ) const;
	void RenderInfo( AABB2 const& bounds ) const;

	void SetPlayerSettings( PlanetSettings const& settings );
	void ReportTimingJudgement( Vec2 position, TimingJudgement judgement );
	void ReportCheckpoint( unsigned int checkpointNodeIndex );

	TapManager& GetTapManager();
	Path const* GetPath() const;
	bool IsPlaying() const;

private:
	void OnEnter_Countdown();
	void OnEnter_Playing();
	void OnEnter_Fail();
	void OnEnter_Win();
	void OnEnter_Inactive();

	void OnExit_Countdown();
	void OnExit_Playing();
	void OnExit_Fail();
	void OnExit_Win();
	void OnExit_Inactive();

	void Update_Countdown();
	void Update_Playing();
	void Update_Fail();
	void Update_Win();
	void Update_Inactive();

	void RenderHUD_Countdown( AABB2 const& screenBounds ) const;
	void RenderHUD_Playing( AABB2 const& screenBounds ) const;
	void RenderHUD_Fail( AABB2 const& screenBounds ) const;
	void RenderHUD_Win( AABB2 const& screenBounds ) const;
	void RenderHUD_Inactive( AABB2 const& screenBounds ) const;

	void AddProp( std::vector<Prop*>& propList, Prop* newProp );
	void ClearGarbageProps( std::vector<Prop*>& propList );

private:
	GameCamera*		m_camera			= nullptr;
	Conductor*		m_conductor			= nullptr;
	PlayerPlanets*	m_player			= nullptr;
	Path*			m_path				= nullptr;
	TapManager*		m_tapInput			= nullptr;
	Timer*			m_inputLockTimer	= nullptr;

	std::vector<Prop*> m_judgementProps;

	LevelMetrics	m_currentMetrics;
	LevelMetrics	m_lastCheckpointMetrics;

	LevelInfo		m_info;
	LevelState		m_state = LevelState::INACTIVE;
	int				m_countdownLength = 4;
	int				m_beatsUntilStart = -1;		// Used for countdown
	double			m_startTimeBeats  = 0.0;	// Used for countdown
	unsigned int	m_checkpointNodeIndex = 0;
};