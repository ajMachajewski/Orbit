#pragma once
#include "Engine/Input/InputSystem.hpp"


//----------------------------------------------------------------------------------------------------------
class TapManager
{
public:
	void IgnoreKey( unsigned char keycode );

	void PollInput();

	void PushTap();
	double PopIfTap();

	void ToggleActive();
	void SetActive( bool active );

private:
	std::vector<double> m_taps;
	bool m_ignoreKey[MAX_KEYBOARD_KEYS];
	bool m_active = true;
};