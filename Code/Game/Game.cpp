#include "Game/Game.hpp"

#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Path.hpp"
#include "Game/PlayerPlanets.hpp"
#include "Game/Conductor.hpp"
#include "Game/Menu.hpp"

#include "Engine/Core/FileUtils.hpp"
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

	if ( buttonEventName == "GOTO_GAMEPLAY" )
	{
		theGame->GoToState( GameState::GAMEPLAY );
		return true;
	}

	if ( buttonEventName == "GOTO_CREDITS" )
	{
		theGame->GoToState( GameState::CREDITS );
		return true;
	}

	if ( buttonEventName == "SELECT_NEXT_LEVEL" )
	{
		theGame->SelectNextLevel();
		return true;
	}

	if ( buttonEventName == "SELECT_PREV_LEVEL" )
	{
		theGame->SelectNextLevel();
		return true;
	}

	if ( buttonEventName == "RESET_CHECKPOINTS" )
	{
		theGame->GetCurrentLevel().ResetCheckpoints();
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

	std::string creditsRawText;
	FileReadToString( creditsRawText, "Data/Credits.txt" );
	m_credits = TaggedString( creditsRawText );

	LoadLevelData();
	InitializeMenus();
	OnEnter_Attract();
}

//--------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	delete[] m_levels;
	m_levels = nullptr;

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

	switch ( m_currentState )
	{
		case GameState::ATTRACT:		Update_Attract();		break;
		case GameState::LEVEL_SELECT:	Update_LevelSelect();	break;
		case GameState::GAMEPLAY:		Update_Gameplay();		break;
		case GameState::CREDITS:		Update_Credits();		break;
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
		case GameState::CREDITS:		Render_Credits();		break;
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
	if ( state == m_currentState )
		return;

	g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		OnExit_Attract();		break;
		case GameState::LEVEL_SELECT:	OnExit_LevelSelect();	break;
		case GameState::GAMEPLAY:		OnExit_Gameplay();		break;
		case GameState::CREDITS:		OnExit_Credits();		break;
	}

	m_currentState = state;
	switch ( m_currentState )
	{
		case GameState::ATTRACT:		OnEnter_Attract();		break;
		case GameState::LEVEL_SELECT:	OnEnter_LevelSelect();	break;
		case GameState::GAMEPLAY:		OnEnter_Gameplay();		break;
		case GameState::CREDITS:		OnEnter_Credits();		break;
	}
}


//----------------------------------------------------------------------------------------------------------
Level& Game::GetCurrentLevel()
{
	return m_levels[m_currentLevelIndex];
}


//----------------------------------------------------------------------------------------------------------
Level const& Game::GetCurrentLevel() const
{
	return m_levels[m_currentLevelIndex];
}


//----------------------------------------------------------------------------------------------------------
void Game::LoadLevelData()
{
	XmlDocument levelConfigDoc;
	XmlResult result = levelConfigDoc.LoadFile( "Data/LevelConfig.xml" );
	if ( result != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( "Failed to load \"Data/LevelConfig.xml\"! Is the file missing?" );
	}

	XmlElement* rootElement = levelConfigDoc.RootElement();
	if ( rootElement == nullptr )
	{
		ERROR_AND_DIE( "Invalid LevelConfig.xml! No root element!" );
	}

	m_levelCount = rootElement->ChildElementCount( "Level" );
	if ( m_levelCount <= 0 )
	{
		ERROR_AND_DIE( "LevelConfig.xml doesn't have any levels in it!" );
	}

	m_levels = new Level[m_levelCount];
	unsigned int levelIndex = 0;
	XmlElement* levelElement = rootElement->FirstChildElement( "Level" );
	while ( levelElement && levelIndex < m_levelCount )
	{
		NamedStrings levelAttributes;
		levelAttributes.PopulateFromXmlElementAttributes( *levelElement );
		std::string const& xmlPath = levelAttributes.GetValue( "xmlPath", "" );

		m_levels[levelIndex].LoadFromXML( xmlPath.c_str() );
		levelElement = levelElement->NextSiblingElement( "Level" );
		levelIndex++;
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	AABB2 screenCameraBounds = g_theWindow->GetClientBounds();
	m_screenCamera.SetOrthoView( screenCameraBounds );
}


//----------------------------------------------------------------------------------------------------------
void Game::InitializeMenus()
{
	InitializeAttractMenu();
	InitalizeLevelSelectMenu();
}


//----------------------------------------------------------------------------------------------------------
void Game::InitializeAttractMenu()
{
	AABB2 screenBounds = g_theWindow->GetClientBounds();
	Vec2 screenDimensions = g_theWindow->GetClientDimensions();

	std::string attractBackgroundFilepath = g_gameConfigBlackboard.GetValue( "attractBackground", "" );
	m_attractMenu = new Menu( "Attract", attractBackgroundFilepath );
	m_attractMenu->m_buttons.reserve( 3 );

	AABB2 buttonRowBounds = screenBounds;		// The "button row" is a temporary container for the buttons
	Vec2 buttonRowDimensions = Vec2( screenDimensions.x * .8f, screenDimensions.y * .25f );
	buttonRowBounds.SetDimensions( buttonRowDimensions, Vec2( .5f, .125f ) );

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

	startButton.LinkTo( creditsButton, EAST );
	startButton.LinkTo( settingsButton, WEST );
}


//----------------------------------------------------------------------------------------------------------
void Game::InitalizeLevelSelectMenu()
{
	AABB2 screenBounds = g_theWindow->GetClientBounds();
	Vec2 screenDimensions = g_theWindow->GetClientDimensions();

	// Level Select Menu
	std::string levelBackgroundFilepath = g_gameConfigBlackboard.GetValue( "levelSelectBackground", "" );
	m_levelSelectMenu = new Menu( "Level Select", levelBackgroundFilepath );
	m_levelSelectMenu->m_buttons.reserve( 4 );

	AABB2 buttonRowBounds = screenBounds;
	buttonRowBounds.ChopOffTop( .6667f );
	buttonRowBounds.ScaleDimensions( Vec2( .667f, .5f ) );

	AABB2 playButtonBounds = buttonRowBounds;
	playButtonBounds.ShrinkToAspect( 2.f );
	playButtonBounds.ScaleHeight( 1.25f );
	AABB2 resetCheckpointBounds = playButtonBounds.ChopOffBottom( .2f );
	playButtonBounds.ScaleDimensions( .95f );
	Button& playButton = m_levelSelectMenu->m_buttons.emplace_back( playButtonBounds, "GOTO_GAMEPLAY", "Play" );
	
	resetCheckpointBounds.ScaleDimensions( .95f );
	Button& resetCheckpointButton = m_levelSelectMenu->m_buttons.emplace_back( resetCheckpointBounds, "RESET_CHECKPOINTS", "Reset Checkpoints" );

	AABB2 leftButtonBounds = buttonRowBounds;
	leftButtonBounds.ScaleDimensions( .75f );
	leftButtonBounds.ShrinkToAspect( .5f, Vec2( 0.f, .5f ) );
	Button& leftButton = m_levelSelectMenu->m_buttons.emplace_back( leftButtonBounds, "SELECT_PREV_LEVEL", "<" );
	
	AABB2 rightButtonBounds = buttonRowBounds;
	rightButtonBounds.ScaleDimensions( .75f );
	rightButtonBounds.ShrinkToAspect( .5f, Vec2( 1.f, .5f ) );
	Button& rightButton = m_levelSelectMenu->m_buttons.emplace_back( rightButtonBounds, "SELECT_NEXT_LEVEL", ">" );

	playButton.LinkTo( rightButton, EAST );
	playButton.LinkTo( leftButton, WEST );
	playButton.LinkTo( resetCheckpointButton, SOUTH );

	resetCheckpointButton.LinkTo( rightButton, EAST, true );
	resetCheckpointButton.LinkTo( leftButton, WEST, true );
}


//----------------------------------------------------------------------------------------------------------
void Game::SelectNextLevel()
{
	m_currentLevelIndex++;
	m_currentLevelIndex %= m_levelCount;
}


//----------------------------------------------------------------------------------------------------------
void Game::SelectPrevLevel()
{
	if ( m_currentLevelIndex == 0 )
	{
		m_currentLevelIndex = m_levelCount - 1;
	}
	else
	{
		m_currentLevelIndex--;
	}
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


//----------------------------------------------------------------------------------------------------------
void Game::Update_Attract()
{
	m_attractMenu->Update();

	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		g_theApp->HandleQuitRequested();
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_LevelSelect()
{
	m_levelSelectMenu->Update();

	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		GoToState( GameState::ATTRACT );
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_Gameplay()
{
	GetCurrentLevel().Update();

	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		GoToState( GameState::LEVEL_SELECT );
		return;
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::Update_Credits()
{
	if ( g_theInput->GetKeyDown( KEYCODE_ESC ) )
	{
		GoToState( GameState::ATTRACT );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Game::Render_Attract() const
{	
	g_theRenderer->ClearScreen( Rgba8::PASTEL_MAGENTA );
	g_theRenderer->BeginCamera( m_screenCamera );

	AABB2 const& screenBounds = m_screenCamera.GetBoundingBox();
	Vec2 screenDimensions = screenBounds.GetDimensions();
	Vec2 planetsCenter = screenBounds.GetPointAtUV( Vec2( .5f, .5f ) );
	float planetRotationRadius = screenDimensions.GetMin() * .25f;
	float planetRadius = planetRotationRadius * .333f;
	float bluePlanetAngle = GetNormalizedAngle( static_cast<float>( m_gameClock->GetTotalSeconds() ) * 120.f );
	float redPlanetAngle = GetNormalizedAngle( bluePlanetAngle + 180.f );
	Vec2 bluePlanetOffset = Vec2::MakeFromPolarDegrees( bluePlanetAngle, planetRotationRadius );
	Vec2 redPlanetOffset = Vec2::MakeFromPolarDegrees( redPlanetAngle, planetRotationRadius );

	Mesh planetVerts;
	AddVertsForDisc2D( planetVerts, planetsCenter + bluePlanetOffset, planetRadius, Rgba8::BLUE, 24 );
	AddVertsForDisc2D( planetVerts, planetsCenter + redPlanetOffset, planetRadius, Rgba8::RED, 24 );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawVertexArray( planetVerts );

	AABB2 titleBounds = screenBounds;
	titleBounds.ChopOffBottom( .5f );
	titleBounds.PadAllSides( -25.f );
	Vec2 titleDimensions = titleBounds.GetDimensions();

	IndexedMesh titleVerts;
	g_defaultFont->AddVertsForTextInBox2D( titleVerts, "ORBIT", titleBounds, 99999.f, Rgba8::WHITE, .7f );
	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawIndexedMesh( titleVerts );

	m_attractMenu->Render( screenBounds );

	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::Render_LevelSelect() const
{
	g_theRenderer->ClearScreen( Rgba8::PASTEL_BLUE );
	g_theRenderer->BeginCamera( m_screenCamera );
	AABB2 const& screenBounds = m_screenCamera.GetBoundingBox();

	AABB2 levelInfoBounds = screenBounds;
	levelInfoBounds.ChopOffBottom( .3333f );
	levelInfoBounds.ScaleWidth( .75f );
	GetCurrentLevel().RenderInfo( levelInfoBounds );

	m_levelSelectMenu->Render( screenBounds );
	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::Render_Gameplay() const
{
	g_theRenderer->ClearScreen( Rgba8::DARK_GRAY );

	GetCurrentLevel().Render();

	g_theRenderer->BeginCamera( m_screenCamera );
	GetCurrentLevel().RenderHUD( m_screenCamera.GetBoundingBox() );
	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void Game::Render_Credits() const
{
	g_theRenderer->ClearScreen( Rgba8( 25, 25, 30, 255 ) );
	g_theRenderer->BeginCamera( m_screenCamera );

	AABB2 const& screenBounds = m_screenCamera.GetBoundingBox();
	Vec2 screenDimensions = screenBounds.GetDimensions();
	AABB2 titleBounds = screenBounds;
	titleBounds.PadAllSides( -25.f );
	AABB2 textBounds = titleBounds.ChopOffBottom( .75f );
	Vec2 titleDimensions = titleBounds.GetDimensions();

	IndexedMesh textVerts;
	g_defaultFont->AddVertsForTextInBox2D( textVerts, "Credits", titleBounds, 99999.f, Rgba8::WHITE, .7f );
	g_defaultFont->AddVertsForTextInBox2D( textVerts, m_credits, textBounds, 99999.f, Rgba8::WHITE, .6f );
	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawIndexedMesh( textVerts );

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
	GetCurrentLevel().Shutdown();
}


//----------------------------------------------------------------------------------------------------------
void Game::OnExit_Credits()
{

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
	GetCurrentLevel().Startup();
}


//----------------------------------------------------------------------------------------------------------
void Game::OnEnter_Credits()
{

}
