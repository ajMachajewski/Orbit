#include "Game/Conductor.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------
/*static*/void Conductor::OnBeat( void* conductor, MusicCallbackInfo info )
{
	Conductor* conductorRef = static_cast<Conductor*>( conductor );
	if ( conductorRef != nullptr )
	{
		conductorRef->OnBeat( info );
	}
}


//----------------------------------------------------------------------------------------------------------
Conductor::Conductor( float bpm, SoundEventID musicEventID, SoundEventID slowEventID, int countInBeats )
	: m_musicEventID( musicEventID )
	, m_elapsedBeats( -countInBeats )
	, m_countInBeats( countInBeats )
	, m_beatDurationSeconds( 60.f / bpm )
	, m_slowEventID( slowEventID )
{
}


//----------------------------------------------------------------------------------------------------------
Conductor::~Conductor()
{
	Stop();
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Play()
{
	Stop();
	m_music = g_theAudio->PlayMusicEvent( m_musicEventID, (void*)this, OnBeat );

	m_timeSinceLastBeat = 0.0;
	m_timeUntilNextBeat = m_beatDurationSeconds;
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Play( double startTimeBeats )
{
	Stop();
	m_elapsedBeats = FloorToInt( startTimeBeats - m_countInBeats );

	double seekTimeSeconds = startTimeBeats * m_beatDurationSeconds;
	unsigned int seekTimeMS = FloorToInt( seekTimeSeconds * 1000 );
	m_music = g_theAudio->PlayMusicEventAt( m_musicEventID, seekTimeMS, (void*)this, OnBeat );

	double beatFraction = startTimeBeats - floor( startTimeBeats );
	if ( beatFraction >= 0.999 )
	{
		beatFraction = 0.0;
		m_elapsedBeats++;
	}

	m_timeSinceLastBeat = static_cast<float>( beatFraction ) * m_beatDurationSeconds;
	m_timeUntilNextBeat = static_cast<float>( 1 - beatFraction ) * m_beatDurationSeconds;
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Update()
{
	if ( m_incrementBeat )
	{
		m_elapsedBeats++;
		m_timeSinceLastBeat = 0.f;
		m_timeUntilNextBeat = m_beatDurationSeconds;
		m_incrementBeat = false;

		if ( m_showDebugMessages )
		{
			DebugAddMessage( Stringf( "Beat #%i\n", m_elapsedBeats + 1 ), 0.8f, Rgba8::DARK_GRAY, Rgba8::CYAN );
		}
	}

	float deltaSeconds = static_cast<float>( GetGameClock()->GetDeltaSeconds() );
	m_timeSinceLastBeat += deltaSeconds;
	m_timeUntilNextBeat -= deltaSeconds;

	if ( m_showDebugMessages )
	{
		DebugAddMessage( Stringf( "Conductor: %f", GetCurrentTimeInBeats() ), 0.f, Rgba8::PASTEL_RED, Rgba8::PASTEL_RED );
	}
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Stop()
{
	g_theAudio->StopEvent( m_music );
	g_theAudio->StopEvent( m_slow );
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Slow()
{
	Stop();
	//m_slow = g_theAudio->PlayEvent( m_slowEventID );
}


//----------------------------------------------------------------------------------------------------------
int Conductor::GetCurrentBeat() const
{
	return m_elapsedBeats;
}


//----------------------------------------------------------------------------------------------------------
double Conductor::GetCurrentTimeInBeats() const
{
	if ( GetBeatDuration() == 0.f )
		return 0.0;

	double beatInteger = static_cast<double>( m_elapsedBeats );
	double beatFraction = static_cast<double>( GetBeatFraction() );

	const double inputDelaySeconds = g_gameConfigBlackboard.GetValue( "inputDelaySeconds", 0.0 );
	double inputDelayBeats = inputDelaySeconds / m_beatDurationSeconds;

	return beatInteger + beatFraction - inputDelayBeats;
}


//----------------------------------------------------------------------------------------------------------
float Conductor::GetBeatFraction() const
{
	if ( m_beatDurationSeconds == 0.f )
		return 0.f;

	return m_timeSinceLastBeat / m_beatDurationSeconds;
}


//----------------------------------------------------------------------------------------------------------
float Conductor::GetBeatDuration() const
{
	return m_beatDurationSeconds;
}


//----------------------------------------------------------------------------------------------------------
void Conductor::OnBeat( MusicCallbackInfo const& info )
{
	// Filter out non-beat callbacks first
	MusicSyncType const& type = info.m_type;
	if ( type != MusicSyncType::BEAT )
		return;

	m_incrementBeat = true;
}
