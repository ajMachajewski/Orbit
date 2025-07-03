#pragma once

class App;
class RandomNumberGenerator;
class Renderer;
class InputSystem;
class AudioSystem_Wwise;
class Window;
class Clock;
class BitmapFont;

struct Vec2;
struct Rgba8;

// GLOBAL OBJECTS
extern App* g_theApp;
extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem_Wwise* g_theAudio;
extern BitmapFont* g_defaultFont;


// DEBUG DRAWING FUNCTIONS
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& posA, Vec2 const& posB, float thickness, Rgba8 const& color );
void DebugDrawCircle( Vec2 const& center, float radius, Rgba8 const& color );

float GetNormalizedAngle( float angle );
float GetAngularDisplacement( float fromDegrees, float toDegrees, bool clockwise );

Clock* GetGameClock();
