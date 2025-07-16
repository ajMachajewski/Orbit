#pragma once

//----------------------------------------------------------------------------------------------------------
struct Rgba8;


//----------------------------------------------------------------------------------------------------------
enum class TimingJudgement
{
	TOO_EARLY,
	EARLY,
	EPERFECT,
	PERFECT,
	LPERFECT,
	LATE,
	TOO_LATE,
	DEATH,

	COUNT
};


//----------------------------------------------------------------------------------------------------------
TimingJudgement GetTimingJudgment( double target, double actual );
bool IsJudgementAcceptable( TimingJudgement judgement );
const char* TimingJudgementToString( TimingJudgement judgement );
Rgba8 TimingJudgementToColor( TimingJudgement judgement );