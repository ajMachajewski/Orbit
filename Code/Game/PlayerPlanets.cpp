#include "Game/PlayerPlanets.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Conductor.hpp"
#include "Game/TapManager.hpp"
#include "Game/Path.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Audio/AudioSystem_Wwise.hpp"


//----------------------------------------------------------------------------------------------------------
PlayerPlanets::PlayerPlanets( Level& level, Conductor const& conductor, PlanetSettings const& planetSettings, int startingIndex )
	: m_level( level )
	, m_path( *level.GetPath() )
	, m_settings( planetSettings )
	, m_conductor( conductor )
	, m_currentNodeIndex( startingIndex - 1 )
{
	GoToNextNode();
}


//----------------------------------------------------------------------------------------------------------
PlayerPlanets::~PlayerPlanets()
{

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

	if ( !m_level.IsPlaying() )
		return;

	bool autoplay = g_gameConfigBlackboard.GetValue( "autoplay", false );
	if ( autoplay && m_active )
	{
		double targetTime = nextNode->m_timeInBeats;
		double deltaTimeToTarget = targetTime - timeInBeats;
		if ( deltaTimeToTarget < 0.0 )
		{
			m_level.GetTapManager().PushTap();
		}
	}

	if ( nextNode == nullptr )
		return;

	double targetTime = nextNode->m_timeInBeats;
	double currentTime = m_conductor.GetCurrentTimeInBeats();
	currentTime = m_conductor.GetCurrentTimeInBeats();

	float beatDurationSeconds = m_conductor.GetBeatDuration();
	double targetTimeSeconds = targetTime * beatDurationSeconds;
	double currentTimeSeconds = currentTime * beatDurationSeconds;

	if ( beatDurationSeconds < 0.f )
 		return;

	TimingJudgement judgement = GetTimingJudgment( targetTimeSeconds, currentTimeSeconds );
	bool nofail = g_gameConfigBlackboard.GetValue( "nofail", false );
	if ( nofail && ( judgement == TimingJudgement::DEATH || judgement == TimingJudgement::TOO_LATE ) )
	{
		m_level.ReportTimingJudgement( GetOrbitingPlanetPosition(), judgement );
		GoToNextNode();
		return;
	}

	if ( judgement == TimingJudgement::DEATH )
	{
		Die();
		return;
	}

	while ( m_level.GetTapManager().PopIfTap() )
	{
		HandleTap( judgement );

		nextNode = GetNextNode();
		if ( nextNode == nullptr )
			break;

		targetTime = nextNode->m_timeInBeats;
		targetTimeSeconds = targetTime * beatDurationSeconds;
		judgement = GetTimingJudgment( targetTimeSeconds, currentTimeSeconds );
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
void PlayerPlanets::Enable()
{
	m_active = true;
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::Disable()
{
	m_active = false;
}


//----------------------------------------------------------------------------------------------------------
void PlayerPlanets::HandleTap( TimingJudgement judgement )
{
	if ( IsJudgementAcceptable( judgement ) )
	{
		GoToNextNode();
		m_overloadCount--;
		if ( m_overloadCount < 0 )
		{
			m_overloadCount = 0;
		}

		m_level.ReportTimingJudgement( GetPosition() + Vec2( 0.f, .6f ), judgement );
	}
	else
	{
		m_overloadCount++;
		int overloadThreshold = g_gameConfigBlackboard.GetValue( "overloadThreshold", 5 );
		if ( m_overloadCount >= overloadThreshold )
		{
			Overload();
		}

		m_level.ReportTimingJudgement( GetOrbitingPlanetPosition(), judgement );
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

	//g_theAudio->PlayEvent( AK::EVENTS::PLAY_TESTCLICK );

	m_angle += 180.f;
	while ( m_angle <= 0.f )	m_angle += 360.f;
	while ( m_angle > 360.f )	m_angle -= 360.f;	// Angle must be in a (0,360] range.

	PathNode const* nextNode = GetNextNode();
	if ( nextNode == nullptr )
	{
		// Reached final node, level clear!
		m_active = false;
		m_level.GoToState( LevelState::WIN );
	}

	m_currentPlanet++;
	m_currentPlanet %= m_planetCount;

	if ( currentNode->m_checkpoint )
	{
		m_level.ReportCheckpoint( m_currentNodeIndex - 1 );
	}
}


//----------------------------------------------------------------------------------------------------------
unsigned int PlayerPlanets::GetNodeIndex() const
{
	return m_currentNodeIndex;
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
Vec2 PlayerPlanets::GetOrbitingPlanetPosition() const
{
	float travelRadius = GetCurrentNode()->m_radius * 2;
	Vec2 toOtherPlanet = Vec2::MakeFromPolarDegrees( m_angle, travelRadius );
	return m_position + toOtherPlanet;
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
	g_theAudio->PlayEvent( AK::EVENTS::PLAY_PLAYERDEATH );
	m_level.GoToState( LevelState::FAIL );
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
