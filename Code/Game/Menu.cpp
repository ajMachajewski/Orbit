#include "Game/Menu.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"


//----------------------------------------------------------------------------------------------------------
Menu::Menu( std::string const& name )
	: m_name( name )
{
}


//----------------------------------------------------------------------------------------------------------	
void Menu::Update()
{
	if ( m_buttons.size() == 0 )
		return;

	Vec2 cursorPosition = g_theInput->GetCursorClientPosition();
	bool clickedThisFrame = g_theInput->GetKeyDown( KEYCODE_LMB );
	for ( Button& button : m_buttons )
	{
		button.Update( cursorPosition );
		if ( clickedThisFrame )
		{
			button.TryClick();
		}
	}

	CardinalDirection direction = DIRECTION_INVALID;
	if ( g_theInput->GetKeyDown( KEYCODE_RIGHT ) )		direction = EAST;
	else if ( g_theInput->GetKeyDown( KEYCODE_UP ) )	direction = NORTH;
	else if ( g_theInput->GetKeyDown( KEYCODE_LEFT ) )	direction = WEST;
	else if ( g_theInput->GetKeyDown( KEYCODE_DOWN ) )	direction = SOUTH;
	if ( direction != DIRECTION_INVALID )
	{
		if ( m_selectedButton == nullptr )
		{
			m_selectedButton = &m_buttons[0];
			m_selectedButton->m_selected = true;
		}

		Button* neighborButton = m_selectedButton->m_neighbors[direction];
		if ( neighborButton != nullptr )
		{
			m_selectedButton->m_selected = false;
			neighborButton->m_selected = true;
			m_selectedButton = neighborButton;
		}
	}

	if ( g_theInput->GetKeyDown( KEYCODE_SPACE ) || g_theInput->GetKeyDown( KEYCODE_ENTER ) )
	{
		if ( m_selectedButton != nullptr )
		{
			m_selectedButton->OnPress();
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void Menu::Render() const
{
	for ( Button const& button : m_buttons )
	{
		button.Render();
	}
}
