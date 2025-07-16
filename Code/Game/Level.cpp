#include "Game/Level.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Conductor.hpp"
#include "Game/PlayerPlanets.hpp"
#include "Game/Path.hpp"
#include "Game/Prop.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameCamera.hpp"
#include "Game/TapManager.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/TaggedString.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------
Level::Level( const char* xmlFilePath )
{
	m_camera = new GameCamera();
	float aspect = g_theWindow->GetAspect();
	float gameSize = g_gameConfigBlackboard.GetValue( "gameSize", 10.f );
	Vec2 gameCameraDimensions;
	gameCameraDimensions.y = gameSize;
	gameCameraDimensions.x = gameSize * aspect;
	AABB2 gameCameraBounds = AABB2( Vec2::ZERO, gameCameraDimensions );
	gameCameraBounds.SetCenter( Vec2::ZERO );
	m_camera->SetOrthoView( gameCameraBounds );

	double inputLockTime = g_gameConfigBlackboard.GetValue( "inputLockTime", 1.0 );
	m_inputLockTimer = new Timer( inputLockTime, GetGameClock() );

	m_tapInput = new TapManager();
	m_tapInput->IgnoreKey( KEYCODE_ESC );
	m_tapInput->IgnoreKey( KEYCODE_TILDE );
	m_tapInput->IgnoreKey( KEYCODE_F1 );
	m_tapInput->IgnoreKey( KEYCODE_F2 );
	m_tapInput->IgnoreKey( KEYCODE_F3 );
	m_tapInput->IgnoreKey( KEYCODE_F4 );
	m_tapInput->IgnoreKey( KEYCODE_F5 );
	m_tapInput->IgnoreKey( KEYCODE_F6 );
	m_tapInput->IgnoreKey( KEYCODE_F7 );
	m_tapInput->IgnoreKey( KEYCODE_F8 );
	m_tapInput->IgnoreKey( KEYCODE_F9 );
	m_tapInput->IgnoreKey( KEYCODE_F10 );
	m_tapInput->IgnoreKey( KEYCODE_F11 );
	m_tapInput->IgnoreKey( KEYCODE_F12 );

	LoadFromXML( xmlFilePath );
}


//----------------------------------------------------------------------------------------------------------
Level::Level()
{
	m_camera = new GameCamera();
	float aspect = g_theWindow->GetAspect();
	float gameSize = g_gameConfigBlackboard.GetValue( "gameSize", 10.f );
	Vec2 gameCameraDimensions;
	gameCameraDimensions.y = gameSize;
	gameCameraDimensions.x = gameSize * aspect;
	AABB2 gameCameraBounds = AABB2( Vec2::ZERO, gameCameraDimensions );
	gameCameraBounds.SetCenter( Vec2::ZERO );
	m_camera->SetOrthoView( gameCameraBounds );

	double inputLockTime = g_gameConfigBlackboard.GetValue( "inputLockTime", 1.0 );
	m_inputLockTimer = new Timer( inputLockTime, GetGameClock() );

	m_tapInput = new TapManager();
}


//----------------------------------------------------------------------------------------------------------
Level::~Level()
{
	delete m_tapInput;
	m_tapInput = nullptr;

	for ( Prop* prop : m_judgementProps )
	{
		if ( prop == nullptr )
			continue;

		delete prop;
		prop = nullptr;
	}


	delete m_path;
	m_path = nullptr;

	delete m_conductor;
	m_conductor = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void Level::LoadFromXML( const char* xmlFilePath )
{
	XmlDocument levelDoc;
	XmlResult result = levelDoc.LoadFile( xmlFilePath );
	if ( result != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "Failed to load \"%s\"", xmlFilePath ) );
	}

	XmlElement* rootElement = levelDoc.RootElement();
	if ( rootElement == nullptr )
	{
		ERROR_AND_DIE( Stringf( "Level file \"%s\" is missing a root element!", xmlFilePath ) );
	}

	NamedStrings attributes;
	attributes.PopulateFromXmlElementAttributes( *rootElement );

	m_countdownLength = attributes.GetValue( "countdownLength", 4 );
	float bpm = attributes.GetValue( "bpm", 120.f );
	std::string musicPlay = attributes.GetValue( "musicPlayEvent", "" );
	std::string musicStop = attributes.GetValue( "musicStopEvent", "" );
	SoundEventID musicPlayEvent = g_theAudio->GetEventID( musicPlay );
	SoundEventID musicStopEvent = g_theAudio->GetEventID( musicStop );
	m_conductor = new Conductor( bpm, musicPlayEvent, musicStopEvent, m_countdownLength );

	m_path = new Path( *m_conductor );
	std::string pathFilePath = attributes.GetValue( "path", "" );
	bool success = m_path->LoadFromFile( pathFilePath.c_str() );
	if ( !success )
	{
		ERROR_AND_DIE( Stringf( "Failed to load \"%s\"", pathFilePath.c_str() ) );
	}

	m_info.m_name = attributes.GetValue( "name", "" );
	m_info.m_source = attributes.GetValue( "source", "" );
	m_info.m_difficulty = attributes.GetValue( "difficulty", 0.f ); 
}


//----------------------------------------------------------------------------------------------------------
void Level::Startup()
{
	GoToState( LevelState::COUNTDOWN );
}


//----------------------------------------------------------------------------------------------------------
void Level::Update()
{
	m_tapInput->PollInput();
	m_camera->Update();
	m_conductor->Update();
	m_player->Update();

	switch ( m_state )
	{
		case LevelState::COUNTDOWN:	Update_Countdown();	break;
		case LevelState::PLAYING:	Update_Playing();	break;
		case LevelState::FAIL:		Update_Fail();		break;
		case LevelState::WIN:		Update_Win();		break;
		case LevelState::INACTIVE:	Update_Inactive();	break;
		default:					ERROR_AND_DIE( "Unhandled Level State in Level::Update()!" );
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::Render() const
{
	g_theRenderer->BeginCamera( *m_camera );

	m_path->Render();
	m_path->DebugRender();
	m_player->Render();

	for ( Prop* prop : m_judgementProps )
	{
		if ( prop == nullptr )
			continue;

		prop->Render();
	}

	g_theRenderer->EndCamera( *m_camera );
	DebugRenderWorld( *m_camera );
}


//----------------------------------------------------------------------------------------------------------
void Level::Shutdown()
{
	GoToState( LevelState::INACTIVE );
}


//----------------------------------------------------------------------------------------------------------
void Level::GoToState( LevelState newState )
{
	if ( newState == m_state )
		return;

	m_inputLockTimer->Start();
	switch ( m_state )
	{
		case LevelState::COUNTDOWN:	OnExit_Countdown();		break;
		case LevelState::PLAYING:	OnExit_Playing();		break;
		case LevelState::FAIL:		OnExit_Fail();			break;
		case LevelState::WIN:		OnExit_Win();			break;
		case LevelState::INACTIVE:	OnExit_Inactive();		break;
		default:					ERROR_AND_DIE( "Unhandled Level State in Level::GoToState()!" );
	}

	m_state = newState;
	switch ( m_state )
	{
		case LevelState::COUNTDOWN:	OnEnter_Countdown();	break;
		case LevelState::PLAYING:	OnEnter_Playing();		break;
		case LevelState::FAIL:		OnEnter_Fail();			break;
		case LevelState::WIN:		OnEnter_Win();			break;
		case LevelState::INACTIVE:	OnEnter_Inactive();		break;
		default:					ERROR_AND_DIE( "Unhandled Level State in Level::GoToState()!" );
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::ResetCheckpoints()
{
	m_checkpointNodeIndex = 0;
	m_lastCheckpointMetrics = LevelMetrics();
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD( AABB2 const& screenBounds ) const
{
	switch ( m_state )
	{
		case LevelState::COUNTDOWN:	RenderHUD_Countdown( screenBounds );	break;
		case LevelState::PLAYING:	RenderHUD_Playing( screenBounds );		break;
		case LevelState::FAIL:		RenderHUD_Fail( screenBounds );			break;
		case LevelState::WIN:		RenderHUD_Win( screenBounds );			break;
		case LevelState::INACTIVE:	RenderHUD_Inactive( screenBounds );		break;
		default:					ERROR_AND_DIE( "Unhandled Level State in Level::RenderHUD()!" );
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderInfo( AABB2 const& bounds ) const
{
	AABB2 titleBounds = bounds;
	titleBounds.ScaleHeight( .5f, 1.f );

	std::string titleText = m_info.m_name;
	float textHeight = titleBounds.GetDimensions().x * 0.0625f;

	IndexedMesh textVerts;
	g_defaultFont->AddVertsForTextInBox2D( textVerts, m_info.m_name, titleBounds, textHeight, 
		Rgba8::WHITE, .75f, Vec2( .5f, 0.f ) );

	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawIndexedMesh( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Level::SetPlayerSettings( PlanetSettings const& settings )
{
	m_player->m_settings = settings;
}


//----------------------------------------------------------------------------------------------------------
void Level::ReportTimingJudgement( Vec2 position, TimingJudgement judgement )
{
	// Increment corresponding judgement count
	m_currentMetrics.m_judgementCounts[(int)judgement]++;
	m_currentMetrics.m_totalJudgements++;

	// Create timing judgement prop
	Rgba8 judgementColor = TimingJudgementToColor( judgement );
	Rgba8Gradient propGradient = Rgba8Gradient( judgementColor, judgementColor.GetTransparent( 0 ) );
	Prop* newProp = new Prop( position, propGradient, 2.f );

	IndexedMesh vertData;
	const char* judgementText = TimingJudgementToString( judgement );
	g_defaultFont->AddVertsForTextInBox2D( vertData, judgementText, AABB2::ZEROS, .22f, 
		Rgba8::WHITE, .5f, Vec2( .5f, .5f ), TextBoxMode::OVERRUN );

	newProp->SetRenderData( vertData, &g_defaultFont->GetTexture() );
	AddProp( m_judgementProps, newProp );
}


//----------------------------------------------------------------------------------------------------------
void Level::ReportCheckpoint( unsigned int checkpointNodeIndex )
{
	m_lastCheckpointMetrics = m_currentMetrics;
	m_checkpointNodeIndex = checkpointNodeIndex;
}


//----------------------------------------------------------------------------------------------------------
TapManager& Level::GetTapManager()
{
	return *m_tapInput;
}


//----------------------------------------------------------------------------------------------------------
Path const* Level::GetPath() const
{
	return m_path;
}


//----------------------------------------------------------------------------------------------------------	
bool Level::IsPlaying() const
{
	return m_state == LevelState::PLAYING;
}


//----------------------------------------------------------------------------------------------------------
void Level::OnEnter_Countdown()
{
	if ( m_player != nullptr )
	{
		delete m_player;
		m_player = nullptr;
	}

	PlanetSettings customization;
	customization.m_planetColors[0] = Rgba8::RED;
	customization.m_planetColors[1] = Rgba8::BLUE;
	customization.m_planetRadius = m_path->GetWidth() * .4f;
	m_player = new PlayerPlanets( *this, *m_conductor, customization, m_checkpointNodeIndex );

	PathNode const* startingNode = m_path->GetNode( m_checkpointNodeIndex + 1 );
	m_startTimeBeats = startingNode ? startingNode->m_timeInBeats : 0.0;
	m_conductor->Play( m_startTimeBeats );

	m_camera->m_targetPosition = m_player->GetPosition();
	m_camera->Reset();
}


//----------------------------------------------------------------------------------------------------------
void Level::OnEnter_Playing()
{
	if ( m_checkpointNodeIndex != 0 )
	{
		m_currentMetrics = m_lastCheckpointMetrics;
		m_currentMetrics.m_checkpointsUsed++;
	}
	else
	{
		m_currentMetrics = LevelMetrics();
	}

	m_tapInput->PopAllTaps();
	m_player->Enable();
}


//----------------------------------------------------------------------------------------------------------
void Level::OnEnter_Fail()
{
	m_conductor->Slow();

	unsigned int totalNodes = m_path->GetNodeCount();
	unsigned int lastSuccesfulNode = m_player->GetNodeIndex();
	m_currentMetrics.m_percentClear = static_cast<float>( lastSuccesfulNode ) / ( static_cast<float>( totalNodes ) - 1.f );
}


//----------------------------------------------------------------------------------------------------------
void Level::OnEnter_Win()
{
	m_currentMetrics.m_percentClear = 1.f;
	ResetCheckpoints();
}


//----------------------------------------------------------------------------------------------------------
void Level::OnEnter_Inactive()
{	
	m_conductor->Stop();

	delete m_player;
	m_player = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void Level::OnExit_Countdown()
{
}


//----------------------------------------------------------------------------------------------------------
void Level::OnExit_Playing()
{
	m_player->Disable();
	m_tapInput->PopAllTaps();
}


//----------------------------------------------------------------------------------------------------------
void Level::OnExit_Fail()
{
}


//----------------------------------------------------------------------------------------------------------
void Level::OnExit_Win()
{

}


//----------------------------------------------------------------------------------------------------------
void Level::OnExit_Inactive()
{

}


//----------------------------------------------------------------------------------------------------------
void Level::Update_Countdown()
{
	m_camera->m_targetPosition = m_player->GetPosition();
	double timeUntilStartBeats = m_startTimeBeats - m_conductor->GetCurrentTimeInBeats();
	if ( timeUntilStartBeats < 0.25 )
	{
		GoToState( LevelState::PLAYING );
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::Update_Playing()
{
	m_camera->m_targetPosition = m_player->GetPositionAhead( 4 );
}


//----------------------------------------------------------------------------------------------------------
void Level::Update_Fail()
{
	m_camera->m_targetPosition = m_player->GetPosition();
	if ( m_inputLockTimer->HasPeriodElapsed() && m_tapInput->PopIfTap() )
	{
		GoToState( LevelState::COUNTDOWN );
	}
	else
	{
		m_tapInput->PopAllTaps();
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::Update_Win()
{
	m_camera->m_targetPosition = m_player->GetPosition();
	if ( m_inputLockTimer->HasPeriodElapsed() && m_tapInput->PopIfTap() )
	{
		g_theApp->m_theGame->GoToState( GameState::LEVEL_SELECT );
	}
	else
	{
		m_tapInput->PopAllTaps();
	}
}


//----------------------------------------------------------------------------------------------------------
void Level::Update_Inactive()
{
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD_Countdown( AABB2 const& screenBounds ) const
{
	double timeUntilStartBeats = m_startTimeBeats - m_conductor->GetCurrentTimeInBeats();
	int beatsUntilStart = CeilToInt( timeUntilStartBeats );
	int countdownLabel = beatsUntilStart;
	if ( countdownLabel > 0 && countdownLabel <= m_countdownLength )
	{
		// Render countdown
		TaggedString countdownText = Stringf( "<rainbow;shadow>%i<!>", countdownLabel );
		AABB2 countdownBounds = screenBounds;
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
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD_Playing( AABB2 const& screenBounds ) const
{
	double timeUntilStartBeats = m_startTimeBeats - m_conductor->GetCurrentTimeInBeats();
	int beatsUntilStart = CeilToInt( timeUntilStartBeats );
	int countdownLabel = beatsUntilStart;
	if ( countdownLabel > 0 && countdownLabel <= m_countdownLength )
	{
		// Render countdown
		TaggedString countdownText = Stringf( "<rainbow;shadow>%i<!>", countdownLabel );
		AABB2 countdownBounds = screenBounds;
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
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD_Fail( AABB2 const& screenBounds ) const
{
	float percentClear = m_currentMetrics.m_percentClear * 100.f;
	TaggedString failText = TaggedString( Stringf( "<shadow>%2.0f%% Complete<!shadow>", percentClear ) );
	AABB2 countdownBounds = screenBounds;
	countdownBounds.PadAllSides( -50.f );
	IndexedMesh textVerts;
	g_defaultFont->AddVertsForTextInBox2D( textVerts, failText, countdownBounds, 250.f, Rgba8::DARK_RED, .6f );

	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::BILINEAR_WRAP );
	g_theRenderer->DrawIndexedMesh( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD_Win( AABB2 const& screenBounds ) const
{
	const char* winMessageRaw = "Level Clear!";
	if ( m_currentMetrics.IsPurePerfect() )
	{
		winMessageRaw = "<rainbow;wave=1>Pure Perfect!<!wave;!rainbow>";
	}
	else if ( m_currentMetrics.IsFullCombo() )
	{
		winMessageRaw = "<wave=.75>Full Combo!<!wave>";
	}

	TaggedString winMessageText = TaggedString( Stringf( "<shadow>%s<!shadow>", winMessageRaw ) );
	std::string metricsTextRaw = m_currentMetrics.GetAsRawString();
	TaggedString metricsText = TaggedString( Stringf( "<shadow>%s<!shadow>", metricsTextRaw.c_str() ) );
	TaggedString scoreText = TaggedString( Stringf( "<shadow>Score: %3.1f%%<!shadow>", m_currentMetrics.GetScore() ) );
	
	AABB2 countdownBounds = screenBounds;
	countdownBounds.PadAllSides( -50.f );
	AABB2 metricsBounds = countdownBounds.ChopOffBottom( .66f );
	AABB2 scoreBounds = metricsBounds.ChopOffTop( .4f );

	IndexedMesh textVerts;
	g_defaultFont->AddVertsForTextInBox2D( textVerts, winMessageText, countdownBounds, 200.f, Rgba8::PASTEL_GREEN, .6f, Vec2( .5f, 0.f ) );
	g_defaultFont->AddVertsForTextInBox2D( textVerts, scoreText, scoreBounds, 75.f, Rgba8::PASTEL_RED, .5f );
	g_defaultFont->AddVertsForTextInBox2D( textVerts, metricsText, metricsBounds, 50.f, Rgba8::PASTEL_BLUE, .5f, Vec2( .5f, 1.f ) );

	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::BILINEAR_WRAP );
	g_theRenderer->DrawIndexedMesh( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Level::RenderHUD_Inactive( AABB2 const& screenBounds ) const
{
	UNUSED( screenBounds );
}


//----------------------------------------------------------------------------------------------------------
void Level::AddProp( std::vector<Prop*>& propList, Prop* newProp )
{
	for ( Prop* prop : propList )
	{
		if ( prop != nullptr )
			continue;

		prop = newProp;
		break;
	}

	propList.push_back(	newProp );
}


//----------------------------------------------------------------------------------------------------------
void Level::ClearGarbageProps( std::vector<Prop*>& propList )
{
	for ( Prop* prop : propList )
	{
		if ( prop == nullptr )
			continue;

		if ( !prop->IsGarbage() )
			continue;

		delete prop;
		prop = nullptr;
	}
}
