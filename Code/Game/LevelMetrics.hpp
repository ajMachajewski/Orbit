#pragma once
#include "Game/TimingJudgement.hpp"
#include <string>


//----------------------------------------------------------------------------------------------------------
struct LevelMetrics
{
	unsigned int	m_totalJudgements = 0;
	unsigned int	m_judgementCounts[(int)TimingJudgement::COUNT] = {};
	unsigned int	m_checkpointsUsed = 0;
	float			m_percentClear = 0.f;
	float			m_score;

public:
	float			GetScore() const;
	std::string		GetAsRawString() const;
	bool			IsPurePerfect() const;
	bool			IsFullCombo() const;
};
