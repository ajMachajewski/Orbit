#pragma once
#include "Engine/Core/Rgba8.hpp"

//----------------------------------------------------------------------------------------------------------
class Renderer;
class Game;
class Camera;
class NamedStrings;
class EventArgs;


//----------------------------------------------------------------------------------------------------------
class App 
{
public:
	static bool RecieveWM_CLOSE( EventArgs& args );
	static bool Command_Autoplay( EventArgs& args );
	static bool Command_Nofail( EventArgs& args );
	static bool Command_Delay( EventArgs& args );

public:
	App();
	~App();
	void Startup();
	void RunMainLoop();
	void Shutdown();
	void RunFrame();

	void LoadGameConfig( char const* gameConfigXMLFilePath );
	bool HandleQuitRequested();
	bool IsQuitting() const;

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

private:
	bool m_isQuitting = false;
	bool m_doDebugRendering = false;
	Camera* m_appCamera;

public:
	Game*	m_theGame;
};
