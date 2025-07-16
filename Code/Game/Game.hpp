#pragma once
#include "Game/GameCommon.hpp"
#include "Game/TapManager.hpp"
#include "Game/GameCamera.hpp"
#include "Game/Level.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/TaggedString.hpp"


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
	CREDITS,
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
	Level& GetCurrentLevel();
	Level const& GetCurrentLevel() const;

	void LoadLevelData();
	void InitializeCameras();
	void InitializeMenus();
	void InitializeAttractMenu();
	void InitalizeLevelSelectMenu();

	void SelectNextLevel();
	void SelectPrevLevel();

	void UpdateDevCheats();

	void Update_Attract();
	void Update_LevelSelect();
	void Update_Gameplay();
	void Update_Credits();

	void Render_Attract() const;
	void Render_LevelSelect() const;
	void Render_Gameplay() const;
	void Render_Credits() const;

	void OnExit_Attract();
	void OnExit_LevelSelect();
	void OnExit_Gameplay();
	void OnExit_Credits();

	void OnEnter_Attract();
	void OnEnter_LevelSelect();
	void OnEnter_Gameplay();
	void OnEnter_Credits();

private:
	Level* m_levels = nullptr;
	unsigned int m_levelCount;
	unsigned int m_currentLevelIndex = 0;

	TaggedString m_credits;
	SoundPlaybackID m_music;
	GameState m_currentState = GameState::ATTRACT;
	Camera m_screenCamera;
	RandomNumberGenerator* m_rng;
	Clock* m_gameClock;

	Menu* m_attractMenu;
	Menu* m_levelSelectMenu;

	bool m_inAttractMode = true;
};
