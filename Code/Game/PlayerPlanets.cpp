#include "Game/PlayerPlanets.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Conductor.hpp"
#include "Game/TapManager.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"


//----------------------------------------------------------------------------------------------------------
PlayerPlanets::PlayerPlanets( Path const& path, Conductor const& conductor, PlanetSettings const& planetSettings )
	: m_path( path )
	, m_settings( planetSettings )
	, m_conductor( conductor )
{
	m_input = new TapManager();
	m_input->IgnoreKey( KEYCODE_ESC );
	m_input->IgnoreKey( KEYCODE_TILDE );
	m_input->IgnoreKey( KEYCODE_F1 );
	m_input->IgnoreKey( KEYCODE_F2 );
	m_input->IgnoreKey( KEYCODE_F3 );
	m_input->IgnoreKey( KEYCODE_F4 );
	m_input->IgnoreKey( KEYCODE_F5 );
	m_input->IgnoreKey( KEYCODE_F6 );
	m_input->IgnoreKey( KEYCODE_F7 );
	m_input->IgnoreKey( KEYCODE_F8 );
	m_input->IgnoreKey( KEYCODE_F9 );
	m_input->IgnoreKey( KEYCODE_F10 );
	m_input->IgnoreKey( KEYCODE_F11 );
	m_input->IgnoreKey( KEYCODE_F12 );

	GoToNextNode();
}


//----------------------------------------------------------------------------------------------------------
PlayerPlanets::~PlayerPlanets()
{
	delete m_input;
	m_input = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::Update()
{
	if ( m_isDead )
		return;

	Clock* gameClock = GetGameClock();
	if ( gameClock == nullptr )
		return;

 	float turnDirection = m_clockwise ? -1.f : 1.f;
	PathNode const* prevNode = GetPreviousNode();
	PathNode const* currNode = GetCurrentNode();
	PathNode const* nextNode = GetNextNode();

	float speed = currNode ? currNode->m_speed : 1.f;
	double timeInBeats = m_conductor.GetCurrentTimeInBeats();
	double prevInputTime = GetCurrentNode()->m_timeInBeats;
	double fractionUntilOneBeatAway = GetFractionWithinRange( timeInBeats, prevInputTime, prevInputTime + 1.f / speed );

	float angleDispFromPrevAngle = Interpolate( 0, turnDirection * 180.f, static_cast<float>( fractionUntilOneBeatAway ) );
	float inAngle = 0.f;
	if ( prevNode != nullptr )
	{
		inAngle = prevNode->m_angle;
	}

	m_angle = 180.f + inAngle + angleDispFromPrevAngle;
	m_angle = GetNormalizedAngle( m_angle );

	bool autoplay = g_gameConfigBlackboard.GetValue( "autoplay", false );
	if ( autoplay && m_active )
	{
		double targetTime = nextNode->m_timeInBeats;
		double deltaTimeToTarget = targetTime - timeInBeats;
		if ( deltaTimeToTarget < 0.0 )
		{
			m_input->PushTap();
		}
	}

	m_input->PollInput();

	if ( nextNode == nullptr )
		return;

	double targetTime = nextNode->m_timeInBeats;
	double currentTime = m_conductor.GetCurrentTimeInBeats();
	float beatDurationSeconds = m_conductor.GetBeatDuration();
	targetTime *= beatDurationSeconds;
	currentTime *= beatDurationSeconds;

	TimingJudgement judgement = GetTimingJudgment( targetTime, currentTime );
	if ( judgement == TimingJudgement::MISS )
	{
		DebugAddMessage( "Miss!", 2.f, Rgba8::DARK_RED, Rgba8::PASTEL_CYAN );
		bool nofail = g_gameConfigBlackboard.GetValue( "nofail", false );
		if ( nofail )
		{
			GoToNextNode();
		}
		else
		{
			Die();
		}
	}

	while ( m_input->PopIfTap() >= 0.0 )
	{
		HandleTap( judgement );

		nextNode = GetNextNode();
		if ( nextNode == nullptr )
			break;

		targetTime = nextNode->m_timeInBeats;
		targetTime *= beatDurationSeconds;
		judgement = GetTimingJudgment( targetTime, currentTime );
	}
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::Render() const
{
	if ( m_isDead )
		return;

	Vec2 planetPositions[MAX_PLANETS];
	planetPositions[m_currentPlanet] = m_position;

	int nextPlanet = ( m_currentPlanet + 1 ) % m_planetCount;
	float travelRadius = GetCurrentNode()->m_radius * 2;
	Vec2 toOtherPlanet = Vec2::MakeFromPolarDegrees( m_angle, travelRadius );
	planetPositions[nextPlanet] = m_position + toOtherPlanet;

	Mesh verts;
	for ( int planetIndex = 0; planetIndex < m_planetCount; planetIndex++ )
	{
		AddVertsForDisc2D( 
			verts, 
			planetPositions[planetIndex], 
			m_settings.m_planetRadius, 
			m_settings.m_planetColors[planetIndex],
			32
		);
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawVertexArray( verts );
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::HandleTap( TimingJudgement judgement )
{
	if ( judgement == TimingJudgement::MISS )
	{
		ERROR_RECOVERABLE( "Miss judgment on a tap! This shouldn't happen?" );
	}


	DebugAddMessage(
		Stringf( "%s", TimingJudgementToString( judgement ) ),
		1.f, TimingJudgementToColor( judgement ), Rgba8::PASTEL_CYAN
	);

	if ( IsJudgementAcceptable( judgement ) )
	{
		GoToNextNode();
		m_overloadCount--;
		if ( m_overloadCount < 0 )
		{
			m_overloadCount = 0;
		}
	}
	else
	{
		m_overloadCount++;
		int overloadThreshold = g_gameConfigBlackboard.GetValue( "overloadThreshold", 5 );
		if ( m_overloadCount >= overloadThreshold )
		{
			Overload();
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::GoToNextNode()
{
	if ( !m_active )
		return;

	m_currentNodeIndex++;
	PathNode const* currentNode = GetCurrentNode();
	m_clockwise = currentNode->m_clockwise;
	m_position = currentNode->GetPosition();

	g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );

	// #PICKUP here
	m_angle += 180.f;
	while ( m_angle <= 0.f )	m_angle += 360.f;
	while ( m_angle > 360.f )	m_angle -= 360.f;	// Angle must be in a (0,360] range.

	PathNode const* nextNode = GetNextNode();
	if ( nextNode == nullptr )
	{
		// Reached final node, level clear!
		m_active = false;
	}

	m_currentPlanet++;
	m_currentPlanet %= m_planetCount;
}


//----------------------------------------------------------------------------------------------------------
Vec2 const& PlayerPlanets::GetPosition() const
{
	return m_position;
}


//----------------------------------------------------------------------------------------------------------
Vec2 const& PlayerPlanets::GetPositionAhead( int nodeLookahead /*= 1 */ ) const
{
	PathNode const* potentialNode =  m_path.GetNode( m_currentNodeIndex + nodeLookahead );
	if ( potentialNode == nullptr )
	{
		potentialNode = m_path.GetLastNode();
		
	}

	return potentialNode->GetPosition();
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::Overload()
{
	DebugAddMessage( "OVERLOAD!!!", 1.f, Rgba8::DARK_RED, Rgba8::CYAN );
	Die();
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::Die()
{
	m_isDead = true;
	m_active = false;
}


//----------------------------------------------------------------------------------------------------------
PathNode const* PlayerPlanets::GetPreviousNode() const
{
	return m_path.GetNode( m_currentNodeIndex - 1 );
}


//----------------------------------------------------------------------------------------------------------
PathNode const* PlayerPlanets::GetCurrentNode() const
{
	return m_path.GetNode( m_currentNodeIndex );
}


//----------------------------------------------------------------------------------------------------------
PathNode const* PlayerPlanets::GetNextNode() const
{
	return m_path.GetNode( m_currentNodeIndex + 1 );
}
