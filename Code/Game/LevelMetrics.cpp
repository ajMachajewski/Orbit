#include "Game/LevelMetrics.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"


//----------------------------------------------------------------------------------------------------------
float LevelMetrics::GetScore() const
{
	unsigned int const& perfectCount	= m_judgementCounts[(int)TimingJudgement::PERFECT];
	unsigned int const& ePerfectCount	= m_judgementCounts[(int)TimingJudgement::EPERFECT];
	unsigned int const& lPerfectCount	= m_judgementCounts[(int)TimingJudgement::LPERFECT];
	unsigned int const& earlyCount		= m_judgementCounts[(int)TimingJudgement::EARLY];
	unsigned int const& lateCount		= m_judgementCounts[(int)TimingJudgement::LATE];

	const float perfectMultiplier		= g_gameConfigBlackboard.GetValue( "perfectMultiplier", 1.f );
	const float nearPerfectMultiplier	= g_gameConfigBlackboard.GetValue( "nearPerfectMultiplier", 1.f );
	const float nonPerfectMultiplier	= g_gameConfigBlackboard.GetValue( "nonPerfectMultiplier", 0.5f );
	const float perCheckpointPenalty	= g_gameConfigBlackboard.GetValue( "checkpointScorePenalty", 0.9f );

	float perfectScore		= perfectMultiplier * perfectCount;
	float nearPerfectScore	= nearPerfectMultiplier * ( ePerfectCount + lPerfectCount );
	float nonPerfectScore	= nonPerfectMultiplier * ( earlyCount + lateCount );
	float percentScore		= ( perfectScore + nearPerfectScore + nonPerfectScore ) / m_totalJudgements;

	float totalCheckpointPenalty = 1.f;
	for ( unsigned int i = 0; i < m_checkpointsUsed; i++ )
	{
		totalCheckpointPenalty *= perCheckpointPenalty;
	}

	return totalCheckpointPenalty * percentScore * 100.f;
}


//----------------------------------------------------------------------------------------------------------
std::string LevelMetrics::GetAsRawString() const
{
	unsigned int const& perfectCount	= m_judgementCounts[(int)TimingJudgement::PERFECT];
	unsigned int const& ePerfectCount	= m_judgementCounts[(int)TimingJudgement::EPERFECT];
	unsigned int const& lPerfectCount	= m_judgementCounts[(int)TimingJudgement::LPERFECT];
	unsigned int const& earlyCount		= m_judgementCounts[(int)TimingJudgement::EARLY];
	unsigned int const& lateCount		= m_judgementCounts[(int)TimingJudgement::LATE];
	unsigned int const& tooEarlyCount	= m_judgementCounts[(int)TimingJudgement::TOO_EARLY];
	unsigned int const& missCount		= m_judgementCounts[(int)TimingJudgement::TOO_LATE];

	std::string out;
	out += Stringf( "Perfects: %i\n", perfectCount );
	out += Stringf( "E-Perfect: %i | L-Perfect: %i\n", ePerfectCount, lPerfectCount );
	out += Stringf( "Too Early: %i | Early: %i | Late: %i | Too Late: %i\n", tooEarlyCount, earlyCount, lateCount, missCount );
	out += "\n";
	out += Stringf( "Checkpoints Used: %i", m_checkpointsUsed );
	return out;
}


//----------------------------------------------------------------------------------------------------------
bool LevelMetrics::IsPurePerfect() const
{
	if ( m_checkpointsUsed > 0 )
		return false;

	for ( unsigned int judgementIndex = 0; judgementIndex < static_cast<unsigned int>( TimingJudgement::COUNT ); judgementIndex++ )
	{
		TimingJudgement judgement = static_cast<TimingJudgement>( judgementIndex );
		if ( judgement == TimingJudgement::PERFECT )
			continue;

		unsigned int const& judgementCount = m_judgementCounts[judgementIndex];
		if ( judgementCount != 0 )
			return false;
	}

	return true;
}


//----------------------------------------------------------------------------------------------------------
bool LevelMetrics::IsFullCombo() const
{
	if ( m_checkpointsUsed > 0 )
		return false;

	for ( unsigned int judgementIndex = 0; judgementIndex < static_cast<unsigned int>( TimingJudgement::COUNT ); judgementIndex++ )
	{
		TimingJudgement judgement = static_cast<TimingJudgement>( judgementIndex );
		if ( judgement == TimingJudgement::PERFECT )
			continue;
		
		if ( judgement == TimingJudgement::EPERFECT )
			continue;		
		
		if ( judgement == TimingJudgement::LPERFECT )
			continue;

		unsigned int const& judgementCount = m_judgementCounts[judgementIndex];
		if ( judgementCount != 0 )
			return false;
	}

	return true;
}
