#pragma once
#include "Engine/Audio/AudioSystem_Wwise.hpp"


//----------------------------------------------------------------------------------------------------------
class Conductor
{
public:
	static void OnBeat( void* conductor, MusicCallbackInfo info );

public:
	Conductor( SoundEventID musicEventID );
	~Conductor();

	void Play();
	void Update();

	int GetCurrentBeat() const;
	double GetCurrentTimeInBeats() const;
	float GetBeatFraction() const;
	float GetBeatDuration() const;

private:
	void OnBeat( MusicCallbackInfo const& info );

private:
	SoundPlaybackID m_music;
	SoundEventID m_musicEventID;

	float m_beatDurationSeconds	= 0.f;
	float m_timeSinceLastBeat	= 0.f;
	float m_timeUntilNextBeat	= 0.f;
	int m_elapsedBeats			= -4;
	bool m_showDebugMessages = false;
};
