#include "Game/Game.hpp"

#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Path.hpp"
#include "Game/PlayerPlanets.hpp"
#include "Game/Conductor.hpp"
#include "Game/Menu.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8Gradient.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"
#include "Engine/Window/Window.hpp"


//----------------------------------------------------------------------------------------------------------
/*static*/bool Game::RecieveButtonPressEvent( EventArgs& args )
{
	if ( g_theApp == nullptr )
		return false;

	Game* theGame = g_theApp->m_theGame;
	if ( theGame == nullptr )
		return false;

	std::string buttonEventName = args.GetValue( "buttonEventName", "" );
	if ( buttonEventName == "GOTO_LEVELSELECT" )
	{
		theGame->GoToState( GameState::LEVEL_SELECT );
		return true;
	}
	
	if ( buttonEventName == "GOTO_ATTRACT" )
	{
		theGame->GoToState( GameState::ATTRACT );
		return true;
	}

	g_theDevConsole->AddLine( DevConsole::WARNING, "Invalid Button Press Event!" );
	return false;
}


//--------------------------------------------------------------------------------------------------------------
Game::Game()
{
	SubscribeEventCallbackFunction( "OnButtonPress", RecieveButtonPressEvent );

	m_rng = new RandomNumberGenerator();
	m_gameClock = new Clock();

	// Menu Initialization
	AABB2 screenBounds = g_theWindow->GetClientBounds();
	Vec2 screenDimensions = g_theWindow->GetClientDimensions();

	// Attract Menu
	m_attractMenu = new Menu( "Attract" );
	m_attractMenu->m_buttons.reserve( 3 );

	AABB2 buttonRowBounds = screenBounds;		// The "button row" is a temporary container for the buttons
	Vec2 buttonRowDimensions = Vec2( screenDimensions.x * .8f, screenDimensions.y * .25f );
	buttonRowBounds.SetDimensions( buttonRowDimensions, Vec2( .5f, .25f ) );

	AABB2 startButtonBounds = buttonRowBounds;
	Vec2 startButtonDimensions = Vec2( buttonRowDimensions.x * .33f, buttonRowDimensions.y );
	startButtonBounds.SetDimensions( startButtonDimensions ); // Default alignment (.5, .5)
	Button& startButton = m_attractMenu->m_buttons.emplace_back( startButtonBounds, "GOTO_LEVELSELECT", "Start" );

	AABB2 settingsButtonBounds = buttonRowBounds;
	Vec2 secondaryButtonDimensions = Vec2( startButtonDimensions.x * .75f, startButtonDimensions.y * .9f );
	settingsButtonBounds.SetDimensions( secondaryButtonDimensions, Vec2( 0.f, .5f ) );
	Button& settingsButton = m_attractMenu->m_buttons.emplace_back( settingsButtonBounds, "GOTO_SETTINGS", "Settings" );
	
	AABB2 creditsButtonBounds = buttonRowBounds;
	creditsButtonBounds.SetDimensions( secondaryButtonDimensions, Vec2( 1.f, .5f ) );
	Button& creditsButton = m_attractMenu->m_buttons.emplace_back( creditsButtonBounds, "GOTO_CREDITS", "Credits" );

	startButton.m_neighbors[EAST] = &creditsButton;
	startButton.m_neighbors[WEST] = &settingsButton;
	settingsButton.m_neighbors[EAST] = &startButton;
	creditsButton.m_neighbors[WEST] = &startButton;

	// Level Select Menu
	m_levelSelectMenu = new Menu( "Level Select" );
	m_levelSelectMenu->m_buttons.reserve( 3 );


	OnEnter_Attract();
}

//--------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		OnExit_Attract();		break;
		case GameState::LEVEL_SELECT:	OnExit_LevelSelect();	break;
		case GameState::GAMEPLAY:		OnExit_Gameplay();		break;
	}

	delete m_levelSelectMenu;
	m_levelSelectMenu = nullptr;

	delete m_attractMenu;
	m_attractMenu = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_rng;
	m_rng = nullptr;

	UnsubscribeEventCallbackFunction( BUTTON_PRESS_EVENT_NAME, RecieveButtonPressEvent );
}


//--------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	UpdateDevCheats();
	UpdateCameras();

	switch ( m_currentState )
	{
		case GameState::ATTRACT:		Update_Attract();		break;
		case GameState::LEVEL_SELECT:	Update_LevelSelect();	break;
		case GameState::GAMEPLAY:		Update_Gameplay();		break;
	}
}

//--------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		Render_Attract();		break; 
		case GameState::LEVEL_SELECT:	Render_LevelSelect();	break; 
		case GameState::GAMEPLAY:		Render_Gameplay();		break; 
	}

	DebugRenderScreen( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
Clock* Game::GetClock()
{
	return m_gameClock;
}


//----------------------------------------------------------------------------------------------------------
void Game::GoToState( GameState state )
{
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		OnExit_Attract();		break;
		case GameState::LEVEL_SELECT:	OnExit_LevelSelect();	break;
		case GameState::GAMEPLAY:		OnExit_Gameplay();		break;
	}

	m_currentState = state;
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		OnEnter_Attract();		break;
		case GameState::LEVEL_SELECT:	OnEnter_LevelSelect();	break;
		case GameState::GAMEPLAY:		OnEnter_Gameplay();		break;
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	float aspect = g_theWindow->GetAspect();
	float gameSize = g_gameConfigBlackboard.GetValue( "gameSize", 10.f );
	Vec2 gameCameraDimensions;
	gameCameraDimensions.y = gameSize;
	gameCameraDimensions.x = gameSize * aspect;

	AABB2 gameCameraBounds = AABB2( Vec2::ZERO, gameCameraDimensions );
	gameCameraBounds.SetCenter( Vec2::ZERO );
	m_gameCamera.SetOrthoView( gameCameraBounds );
	m_gameCamera.Reset();

	AABB2 screenCameraBounds = g_theWindow->GetClientBounds();
	m_screenCamera.SetOrthoView( screenCameraBounds );
}


//----------------------------------------------------------------------------------------------------------
void Game::UpdateDevCheats()
{
	if ( g_theInput->GetKeyDown( 'P' ) )
	{
		m_gameClock->TogglePause();
	}

	if ( g_theInput->GetKeyDown( 'O' ) )
	{
		m_gameClock->StepSingleFrame();
	}

	bool doSlowMode;
	if ( g_theInput->GetKey( 'T' ) )
	{
		doSlowMode = true;
	}
	else
	{
		doSlowMode = false;
	}


	if ( doSlowMode )
	{
		m_gameClock->SetTimeScale( 0.1 );
	}
	else
	{
		m_gameClock->SetTimeScale( 1.0 );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	m_gameCamera.Update();
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_Attract()
{
	m_attractMenu->Update();

	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		g_theApp->HandleQuitRequested();
	}

	if ( g_theInput->GetKeyDown( KEYCODE_SPACE ) )
	{
		g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );
		GoToState( GameState::LEVEL_SELECT );
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_LevelSelect()
{
	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );
		GoToState( GameState::ATTRACT );
	}

	if ( g_theInput->GetKeyDown( KEYCODE_SPACE ) )
	{
		g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );
		GoToState( GameState::GAMEPLAY );
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_Gameplay()
{
	m_conductor->Update();
	m_player->Update();
	m_gameCamera.m_targetPosition = m_player->GetPositionAhead( 4 );

	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );
		GoToState( GameState::LEVEL_SELECT );
		return;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Game::Render_Attract() const
{
	g_theRenderer->ClearScreen( Rgba8::PASTEL_MAGENTA );
	g_theRenderer->BeginCamera( m_screenCamera );
	m_attractMenu->Render();
	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::Render_LevelSelect() const
{
	g_theRenderer->ClearScreen( Rgba8::PASTEL_BLUE );
	g_theRenderer->BeginCamera( m_screenCamera );
	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::Render_Gameplay() const
{
	g_theRenderer->ClearScreen( Rgba8::DARK_GRAY );
	g_theRenderer->BeginCamera( m_gameCamera );

	m_testPath->Render();
	m_testPath->DebugRender();
	m_player->Render();

	g_theRenderer->EndCamera( m_gameCamera );
	DebugRenderWorld( m_gameCamera );

	g_theRenderer->BeginCamera( m_screenCamera );

	int currentBeat = m_conductor->GetCurrentBeat();
	if ( currentBeat <= 0 && currentBeat > -4 )
	{
		// Render countdown
		TaggedString countdownText = Stringf( "<rainbow;shadow>%i<!>", -currentBeat + 1 );
		AABB2 countdownBounds = m_screenCamera.GetBoundingBox();
		countdownBounds.PadAllSides( -50.f );
		IndexedMesh textVerts;
		g_defaultFont->AddVertsForTextInBox2D( textVerts, countdownText, countdownBounds, 250.f, Rgba8::WHITE, .6f );

		g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::BILINEAR_WRAP );
		g_theRenderer->DrawIndexedMesh( textVerts );
	}

	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::OnExit_Attract()
{
}


//----------------------------------------------------------------------------------------------------------
void Game::OnExit_LevelSelect()
{

}


//----------------------------------------------------------------------------------------------------------
void Game::OnExit_Gameplay()
{
	delete m_player;
	m_player = nullptr;

	delete m_testPath;
	m_testPath = nullptr;

	delete m_conductor;
	m_conductor = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void Game::OnEnter_Attract()
{
	InitializeCameras();

	AABB2 buttonBounds = m_screenCamera.GetBoundingBox();
	buttonBounds.SetDimensions( 200, 100 );
}


//----------------------------------------------------------------------------------------------------------
void Game::OnEnter_LevelSelect()
{
	InitializeCameras();
}


//----------------------------------------------------------------------------------------------------------
void Game::OnEnter_Gameplay()
{
	InitializeCameras();
	DebugRenderSetVisible();

	//m_conductor = new Conductor( AK::EVENTS::PLAY_DEMOMUSIC );
	m_conductor = new Conductor( AK::EVENTS::PLAY_RIZU_STAGE );

	m_testPath = new Path( *m_conductor );
	//bool success = m_testPath->LoadFromFile( "Data/Paths/TestPath1.xml" );
	bool success = m_testPath->LoadFromFile( "Data/Paths/RizuStage.xml" );
	if ( !success )
	{
		ERROR_AND_DIE( "Failed to load TestPath1.xml!" );
	}

	PlanetSettings customization;
	customization.m_planetColors[0] = Rgba8::RED;
	customization.m_planetColors[1] = Rgba8::BLUE;
	m_player = new PlayerPlanets( *m_testPath, *m_conductor, customization );
	
	m_conductor->Play();
}
