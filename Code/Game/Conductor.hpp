#pragma once
#include "Engine/Audio/AudioSystem_Wwise.hpp"


//----------------------------------------------------------------------------------------------------------
class Conductor
{
public:
	static void OnBeat( void* conductor, MusicCallbackInfo info );

public:
	Conductor( float bpm, SoundEventID musicEventID, SoundEventID slowEventID, int countInBeats = 4 );
	~Conductor();

	void Play();
	void Play( double startTimeBeats );
	void Update();
	void Stop();
	void Slow();

	int GetCurrentBeat() const;
	double GetCurrentTimeInBeats() const;
	float GetBeatFraction() const;
	float GetBeatDuration() const;

private:
	void OnBeat( MusicCallbackInfo const& info );

private:
	SoundPlaybackID m_music;
	SoundPlaybackID m_slow;
	SoundEventID m_musicEventID;
	SoundEventID m_slowEventID;

	float	m_beatDurationSeconds	= 0.f;
	float	m_timeSinceLastBeat		= 0.f;
	float	m_timeUntilNextBeat		= 0.f;
	int		m_countInBeats			= 4;
	int		m_elapsedBeats			= -4;
	bool	m_showDebugMessages		= true;
	bool	m_incrementBeat			= false;
};
