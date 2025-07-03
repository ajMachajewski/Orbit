#pragma once
#include "Game/GameCommon.hpp"
#include "Game/TapManager.hpp"
#include "Game/GameCamera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"
#include "Engine/Math/AABB2.hpp"


//----------------------------------------------------------------------------------------------------------
class Clock;
class Path;
class PlayerPlanets;
class Conductor;
class Menu;


//----------------------------------------------------------------------------------------------------------
enum class GameState
{
	ATTRACT,
	LEVEL_SELECT,
	GAMEPLAY,
};


//----------------------------------------------------------------------------------------------------------
class Game 
{
public:
	static bool RecieveButtonPressEvent( EventArgs& args );

public:
	Game();
	~Game();

	void Update();
	void Render() const;

	Clock* GetClock();
	void GoToState( GameState state );

private:
	void InitializeCameras();

	void UpdateDevCheats();
	void UpdateCameras();

	void Update_Attract();
	void Update_LevelSelect();
	void Update_Gameplay();

	void Render_Attract() const;
	void Render_LevelSelect() const;
	void Render_Gameplay() const;

	void OnExit_Attract();
	void OnExit_LevelSelect();
	void OnExit_Gameplay();

	void OnEnter_Attract();
	void OnEnter_LevelSelect();
	void OnEnter_Gameplay();

private:
	Conductor* m_conductor;
	PlayerPlanets* m_player;
	Path* m_testPath;

	SoundPlaybackID m_music;
	GameState m_currentState;
	GameCamera m_gameCamera;
	Camera m_screenCamera;
	RandomNumberGenerator* m_rng;
	Clock* m_gameClock;

	Menu* m_attractMenu;
	Menu* m_levelSelectMenu;

	bool m_inAttractMode = true;
};
