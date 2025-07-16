#pragma once
#include "Engine/Input/InputSystem.hpp"


//----------------------------------------------------------------------------------------------------------
class TapManager
{
public:
	TapManager();

	void IgnoreKey( unsigned char keycode );

	void PollInput();
	void PopAllTaps();

	void PushTap();
	bool PopIfTap();

	void ToggleActive();
	void SetActive( bool active );

private:
	std::vector<double> m_taps;
	bool m_ignoreKey[MAX_KEYBOARD_KEYS];
	bool m_active = true;
};