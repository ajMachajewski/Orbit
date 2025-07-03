#include "Game/Conductor.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRender.hpp"


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
Conductor::Conductor( SoundEventID musicEventID )
	: m_musicEventID( musicEventID )
{
}


//----------------------------------------------------------------------------------------------------------
Conductor::~Conductor()
{
	g_theAudio->StopEvent( m_music );
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Play()
{
	if ( g_theAudio->IsEventPlaying( m_music ) )
	{
		g_theAudio->StopEvent( m_music );
	}

	m_music = g_theAudio->PlayMusicEvent( m_musicEventID, (void*)this, OnBeat );
}


//----------------------------------------------------------------------------------------------------------
void Conductor::Update()
{
	float deltaSeconds = static_cast<float>( GetGameClock()->GetDeltaSeconds() );
	m_timeSinceLastBeat += deltaSeconds;
	m_timeUntilNextBeat -= deltaSeconds;

	if ( m_showDebugMessages )
	{
		DebugAddMessage( Stringf( "Conductor: %f", GetCurrentTimeInBeats() ), 0.f, Rgba8::PASTEL_RED, Rgba8::PASTEL_RED );
	}
}


//----------------------------------------------------------------------------------------------------------
int Conductor::GetCurrentBeat() const
{
	return m_elapsedBeats;
}


//----------------------------------------------------------------------------------------------------------
double Conductor::GetCurrentTimeInBeats() const
{
	double beatDuration = static_cast<double>( m_beatDurationSeconds );
	if ( beatDuration == 0.0 )
		return 0.0;

	double timeSinceLastBeat = static_cast<double>( m_timeSinceLastBeat );
	double beatInteger = static_cast<double>( m_elapsedBeats );
	double beatFraction = timeSinceLastBeat / beatDuration;
	return beatInteger + beatFraction;
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

	m_elapsedBeats++;
	if ( m_showDebugMessages )
	{
		DebugAddMessage( Stringf( "Beat #%i\n", m_elapsedBeats ), 0.8f, Rgba8::DARK_GRAY, Rgba8::CYAN );
	}

	m_beatDurationSeconds = g_theAudio->GetCurrentBeatDuration( m_music );
	m_timeSinceLastBeat = 0.f;
	m_timeUntilNextBeat = m_beatDurationSeconds;
}
