#include "Game/TapManager.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"


//----------------------------------------------------------------------------------------------------------
void TapManager::IgnoreKey( unsigned char keycode )
{
	m_ignoreKey[keycode] = true;
}


//----------------------------------------------------------------------------------------------------------
void TapManager::PollInput()
{
	if ( !m_active )
		return;

	for ( int keyIndex = 0; keyIndex < MAX_KEYBOARD_KEYS; keyIndex++ )
	{
		if ( m_ignoreKey[keyIndex] )
			continue;

		unsigned char keycode = static_cast<unsigned char>( keyIndex );
		if ( g_theInput->GetKeyDown( keycode ) )
		{
			PushTap();
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void TapManager::PushTap()
{
	m_taps.push_back( GetCurrentTimeSeconds() );
}


//----------------------------------------------------------------------------------------------------------
double TapManager::PopIfTap()
{
	if ( m_taps.size() == 0 )
		return -1.0;

	double tapTime = m_taps.back();
	m_taps.pop_back();
	return tapTime;
}


//----------------------------------------------------------------------------------------------------------
void TapManager::ToggleActive()
{
	SetActive( !m_active );
}


//----------------------------------------------------------------------------------------------------------
void TapManager::SetActive( bool active )
{
	m_active = active;
}
