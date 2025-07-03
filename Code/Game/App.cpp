#include "Game/App.hpp"
#include "Game/Game.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"
#include "Engine/Core/Clock.hpp"


App*				g_theApp = nullptr;

Renderer*			g_theRenderer = nullptr;
InputSystem*		g_theInput = nullptr;
AudioSystem_Wwise*	g_theAudio = nullptr;
Window*				g_theWindow = nullptr;
BitmapFont*			g_defaultFont = nullptr;

extern Clock* g_systemClock;

//----------------------------------------------------------------------------------------------------------
/*static*/bool App::RecieveWM_CLOSE( EventArgs& args )
{
	UNUSED( args );
	if ( g_theApp == nullptr )
		return false;

	return g_theApp->HandleQuitRequested();
}


//----------------------------------------------------------------------------------------------------------
/*static*/bool App::Command_Autoplay( EventArgs& args )
{
	UNUSED( args );
	bool currentAutoplayState = g_gameConfigBlackboard.GetValue( "autoplay", false );

	if ( currentAutoplayState == true )
	{
		g_gameConfigBlackboard.SetValue( "autoplay", "false" );
		if ( g_theDevConsole )
		{
			g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Autoplay turned <tint=red>OFF<!tint>" );
		}
	}
	else
	{
		g_gameConfigBlackboard.SetValue( "autoplay", "true" );
		if ( g_theDevConsole )
		{
			g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Autoplay turned <tint=green>ON<!tint>" );
		}
	}

	return true;
}


//----------------------------------------------------------------------------------------------------------
/*static*/bool App::Command_Nofail(EventArgs& args)
{
	UNUSED( args );
	bool currentAutoplayState = g_gameConfigBlackboard.GetValue( "nofail", false );

	if ( currentAutoplayState == true )
	{
		g_gameConfigBlackboard.SetValue( "nofail", "false" );
		if ( g_theDevConsole )
		{
			g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "No-Fail mode turned <tint=red>OFF<!tint>" );
		}
	}
	else
	{
		g_gameConfigBlackboard.SetValue( "nofail", "true" );
		if ( g_theDevConsole )
		{
			g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "No-Fail mode <tint=green>ON<!tint>" );
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
App::App()
{
}


//--------------------------------------------------------------------------------------------------------------
App::~App() 
{
}


//--------------------------------------------------------------------------------------------------------------
void App::Startup()
{
	// Create and startup all Engine subsystems.
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem( eventSystemConfig );

	WindowConfig windowConfig;
	windowConfig.m_aspectRatio = 2.0f;
	windowConfig.m_windowTitle = "Orbit";
	g_theWindow = new Window( windowConfig );

	InputConfig inputConfig;
	inputConfig.m_window = g_theWindow;
	g_theInput = new InputSystem( inputConfig );

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( rendererConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_bitmapFontFilePath = "Data/Images/RobotoMonoSemiBold128";
	devConsoleConfig.m_maxLinesVisible = 35.5;
	g_theDevConsole = new DevConsole( devConsoleConfig );

	AudioConfig_Wwise audioConfig;
	g_theAudio = new AudioSystem_Wwise( audioConfig );

	g_systemClock = new Clock();
	m_appCamera = new Camera();

	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theWindow->Startup();
	g_theInput->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	g_theAudio->LoadBank( "Main" );

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	debugRenderConfig.m_fontFilePathNoExtension = "Data/Images/RobotoMonoSemiBold128";
	debugRenderConfig.m_maxMessagesVisible = 24.5f;
	DebugRenderSystemStartup( debugRenderConfig );

	// Close window button support
	g_theEventSystem->SubscribeEventCallbackFunction( "quit", RecieveWM_CLOSE );
	g_theEventSystem->GetEventMetadata( "quit" ).m_isCommmand = true;
	g_theEventSystem->GetEventMetadata( "quit" ).m_shortDescription = "Quits the application.";
	g_theEventSystem->DefineAlias( "q", "quit" );
	g_theEventSystem->DefineAlias( "QUIT", "quit" );
	g_theEventSystem->DefineAlias( "exit", "quit" );

	g_theEventSystem->SubscribeEventCallbackFunction( "autoplay", Command_Autoplay );
	g_theEventSystem->GetEventMetadata( "autoplay" ).m_isCommmand = true;
	g_theEventSystem->GetEventMetadata( "autoplay" ).m_shortDescription = "Toggles on or off autoplay mode.";
	g_theEventSystem->GetEventMetadata( "autoplay" ).m_longDescription = "Autoplay mode will automatically get Perfect! ratings without any player input.";
	g_theEventSystem->DefineAlias( "ap", "autoplay" );

	g_theEventSystem->SubscribeEventCallbackFunction( "nofail", Command_Nofail );
	g_theEventSystem->GetEventMetadata( "nofail" ).m_isCommmand = true;
	g_theEventSystem->GetEventMetadata( "nofail" ).m_shortDescription = "Toggles on or off No-Fail mode.";
	g_theEventSystem->GetEventMetadata( "nofail" ).m_longDescription = "No-Fail will move the player to the next tile if you miss an input, rather than killing the player.";
	g_theEventSystem->DefineAlias( "nf", "nofail" );

	g_defaultFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Images/RobotoMonoSemiBold128" );

	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "App Startup" );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Press ESC to return to the previous screen." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Press any key to advance to the next screen." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Press any key when the orbiting planet intersects the next path tile." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "If you die, back out with ESC and then press any key to try again." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "\nAutoplay and NoFail modes are available via commands." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Type \"h\" or \"help\" for information." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "\nGreen tiles swap the direction of rotation." );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Magenta tiles indicate a speed up, cyan indicates a slow down." );

	// Initialize Game & Game Constants
	LoadGameConfig( "Data/GameConfig.xml" );
	m_theGame = new Game();
}


//----------------------------------------------------------------------------------------------------------
void App::RunMainLoop()
{
	while ( !m_isQuitting )
	{
		RunFrame();
	}
}


//--------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	g_defaultFont = nullptr;

	delete m_theGame;
	m_theGame = nullptr;
	
	g_theEventSystem->UnsubscribeEventCallbackFunction( "quit", RecieveWM_CLOSE );
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "App Shutdown" );

	DebugRenderSystemShutdown();

	g_theAudio->Shutdown();
	g_theDevConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theInput->Shutdown();
	g_theWindow->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_systemClock;
	g_systemClock = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}


//----------------------------------------------------------------------------------------------------------
void App::LoadGameConfig( char const* gameConfigXMLFilePath )
{
	XmlDocument gameConfig;
	XmlResult result = gameConfig.LoadFile( gameConfigXMLFilePath );
	if ( result == tinyxml2::XML_SUCCESS )
	{
		XmlElement* rootElement = gameConfig.RootElement();
		if ( rootElement )
		{
			g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *rootElement );
			g_theDevConsole->AddLine( DevConsole::INFO_MINOR, Stringf( "Successfully loaded game config from \"%s\".", gameConfigXMLFilePath ) );
		}
		else
		{
			g_theDevConsole->AddLine( DevConsole::WARNING, Stringf( "Game config loaded from \"%s\" was invalid (missing a root node!)", gameConfigXMLFilePath ) );
		}
	}
	else
	{
		g_theDevConsole->AddLine( DevConsole::WARNING, Stringf( "Failed to load game config from file \"%s\"", gameConfigXMLFilePath ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested() 
{
	g_theDevConsole->AddLine( DevConsole::INFO_MAJOR, "Quitting Application..." );

	m_isQuitting = true;
	return m_isQuitting;
}


//--------------------------------------------------------------------------------------------------------------
void App::BeginFrame() 
{
	Clock::TickSystemClock();
	m_appCamera->SetOrthoView( AABB2( Vec2::ZERO, Vec2( g_theWindow->GetClientDimensions() ) ) );

	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	DebugRenderBeginFrame();
	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
}


//--------------------------------------------------------------------------------------------------------------
void App::Update()
{
	if ( g_theInput->GetKeyDown( KEYCODE_F1 ) )
	{
		m_doDebugRendering = !m_doDebugRendering;
		if ( m_doDebugRendering )	DebugRenderSetVisible();
		else						DebugRenderSetHidden();
	}

	if ( g_theInput->GetKeyDown( KEYCODE_TILDE ) )
	{
		g_theDevConsole->ToggleMode( DevConsoleMode::OPEN_FULL );
	}

	m_theGame->Update();
}


//--------------------------------------------------------------------------------------------------------------
void App::Render() const
{
	m_appCamera->SetOrthoView( g_theWindow->GetClientBounds() );
	m_theGame->Render();

	g_theRenderer->BeginCamera( *m_appCamera );
	g_theDevConsole->Render( AABB2( Vec2::ZERO, Vec2( g_theWindow->GetClientDimensions() ) ) );
	g_theRenderer->EndCamera( *m_appCamera );
}


//--------------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	DebugRenderEndFrame();
	g_theDevConsole->EndFrame();
	g_theAudio->EndFrame();
}


//--------------------------------------------------------------------------------------------------------------
bool App::IsQuitting() const 
{ 
	return m_isQuitting; 
}
