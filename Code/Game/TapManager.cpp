#include "Game/TapManager.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"


//----------------------------------------------------------------------------------------------------------
TapManager::TapManager()
{
	IgnoreKey( KEYCODE_ESC );
	IgnoreKey( KEYCODE_TILDE );
	IgnoreKey( KEYCODE_F1 );
	IgnoreKey( KEYCODE_F2 );
	IgnoreKey( KEYCODE_F3 );
	IgnoreKey( KEYCODE_F4 );
	IgnoreKey( KEYCODE_F5 );
	IgnoreKey( KEYCODE_F6 );
	IgnoreKey( KEYCODE_F7 );
	IgnoreKey( KEYCODE_F8 );
	IgnoreKey( KEYCODE_F9 );
	IgnoreKey( KEYCODE_F10 );
	IgnoreKey( KEYCODE_F11 );
	IgnoreKey( KEYCODE_F12 );
	IgnoreKey( KEYCODE_VOLUME_UP );
	IgnoreKey( KEYCODE_VOLUME_DOWN );
	IgnoreKey( KEYCODE_VOLUME_MUTE );
}


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
void TapManager::PopAllTaps()
{
	m_taps.clear();
}


//----------------------------------------------------------------------------------------------------------
void TapManager::PushTap()
{
	if ( !m_active )
		return;

	m_taps.push_back( GetCurrentTimeSeconds() );
}


//----------------------------------------------------------------------------------------------------------
bool TapManager::PopIfTap()
{
	if ( m_taps.size() == 0 )
		return false;

	m_taps.pop_back();
	return true;
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
	if ( !m_active )
	{
		m_taps.clear();
	}
}
