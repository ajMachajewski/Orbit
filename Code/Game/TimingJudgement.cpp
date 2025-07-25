#include "Game/TimingJudgement.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"


//----------------------------------------------------------------------------------------------------------
TimingJudgement GetTimingJudgment( double targetSeconds, double actualSeconds )
{
	const float perfectThreshold = g_gameConfigBlackboard.GetValue( "perfectThresholdSeconds", 0.05f );
	const float nearPerfectThreshold = g_gameConfigBlackboard.GetValue( "nearPerfectThresholdSeconds", 0.25f );
	const float acceptableThreshold = g_gameConfigBlackboard.GetValue( "acceptedThresholdSeconds", 0.40f );
	const float deathThreshold = g_gameConfigBlackboard.GetValue( "deathThresholdSeconds", 0.40f );

	const float timeSinceTarget = static_cast<float>( actualSeconds - targetSeconds );
	const float timeOffset = fabsf( timeSinceTarget );
	if ( timeOffset <= perfectThreshold )
	{
		return TimingJudgement::PERFECT;
	}

	bool isEarly = timeSinceTarget < 0.f;
	if ( timeOffset <= nearPerfectThreshold )
	{
		return isEarly ? TimingJudgement::EPERFECT : TimingJudgement::LPERFECT;
	}

	if ( timeOffset <= acceptableThreshold )
	{
		return isEarly ? TimingJudgement::EARLY : TimingJudgement::LATE;
	}

	if ( !isEarly && timeOffset >= deathThreshold )
	{
		return TimingJudgement::DEATH;
	}

	return isEarly ? TimingJudgement::TOO_EARLY : TimingJudgement::TOO_LATE;
}


//----------------------------------------------------------------------------------------------------------
bool IsJudgementAcceptable( TimingJudgement judgement )
{
	switch ( judgement )
	{
		case TimingJudgement::TOO_EARLY:	return false;
		case TimingJudgement::EARLY:		return true;
		case TimingJudgement::EPERFECT:		return true;
		case TimingJudgement::PERFECT:		return true;
		case TimingJudgement::LPERFECT:		return true;
		case TimingJudgement::LATE:			return true;
		case TimingJudgement::TOO_LATE:		return false;
		case TimingJudgement::DEATH:		return false;

		default:
		{
			ERROR_RECOVERABLE( "Unhandled Timing Judgement in IsJudgementAcceptable()!" );
			return false;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
const char* TimingJudgementToString( TimingJudgement judgement )
{
	switch ( judgement )
	{
		case TimingJudgement::TOO_EARLY:	return "Too Early!";
		case TimingJudgement::EARLY:		return "Early";
		case TimingJudgement::EPERFECT:		return "E-Perfect";
		case TimingJudgement::PERFECT:		return "Perfect!";
		case TimingJudgement::LPERFECT:		return "L-Perfect";
		case TimingJudgement::LATE:			return "Late";
		case TimingJudgement::TOO_LATE:		return "Miss!";
		case TimingJudgement::DEATH:		return "";

		default:							return "???";
	}
}


//----------------------------------------------------------------------------------------------------------
Rgba8 TimingJudgementToColor( TimingJudgement judgement )
{
	switch ( judgement )
	{
		case TimingJudgement::TOO_EARLY:	return Rgba8( 175, 0, 0, 255 );		// Dark Red
		case TimingJudgement::EARLY:		return Rgba8( 255, 50, 0, 255 );	// Bright Red
		case TimingJudgement::EPERFECT:		return Rgba8( 200, 200, 0, 255 );	// Yellow
		case TimingJudgement::PERFECT:		return Rgba8( 0, 255, 50, 255 );	// Green
		case TimingJudgement::LPERFECT:		return Rgba8( 200, 200, 0, 255 );	// Yellow
		case TimingJudgement::LATE:			return Rgba8( 255, 50, 0, 255 );	// Bright Red
		case TimingJudgement::TOO_LATE:		return Rgba8( 175, 0, 0, 255 );		// Dark Red
		case TimingJudgement::DEATH:		return Rgba8( 0, 0, 0, 0 );

		default:							return Rgba8::BLACK;
	}
}
