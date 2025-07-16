#include "Game/Menu.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"


//----------------------------------------------------------------------------------------------------------
Menu::Menu( std::string const& name, Texture* backgroundTexture )
	: m_name( name )
	, m_backgroundTexture( backgroundTexture )
{
}


//----------------------------------------------------------------------------------------------------------
Menu::Menu( std::string const& name, std::string const& backgroundImageFilepath )
	: m_name( name )
	, m_backgroundTexture( nullptr )
{
	if ( backgroundImageFilepath == "" )
		return;

	m_backgroundTexture = g_theRenderer->CreateOrGetTextureFromFile( backgroundImageFilepath.c_str() );
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
	if		( g_theInput->GetKeyDown( KEYCODE_RIGHT ) )	direction = EAST;
	else if ( g_theInput->GetKeyDown( KEYCODE_UP ) )	direction = NORTH;
	else if ( g_theInput->GetKeyDown( KEYCODE_LEFT ) )	direction = WEST;
	else if ( g_theInput->GetKeyDown( KEYCODE_DOWN ) )	direction = SOUTH;
	if ( direction != DIRECTION_INVALID )
	{
		if ( m_selectedButton == nullptr )
		{
			m_selectedButton = &m_buttons[0];
			m_selectedButton->m_selected = true;
			return;
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
		if ( m_selectedButton == nullptr )
		{
			m_selectedButton = &m_buttons[0];
			m_selectedButton->m_selected = true;
		}

		m_selectedButton->OnPress();
	}
}


//----------------------------------------------------------------------------------------------------------
void Menu::Render( AABB2 const& screenBounds ) const
{
	if ( m_backgroundTexture != nullptr )
	{
		AABB2 backgroundBounds = screenBounds;
		backgroundBounds.GrowToAspect( m_backgroundTexture->GetAspect() );

		IndexedMesh backgroundVerts;
		AddVertsForAABB2D( backgroundVerts, backgroundBounds, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 1.f );

		g_theRenderer->BindTexture( m_backgroundTexture );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
		g_theRenderer->DrawIndexedMesh( backgroundVerts );
	}

	for ( Button const& button : m_buttons )
	{
		button.Render();
	}
}


//----------------------------------------------------------------------------------------------------------
void Menu::Reset()
{
	for ( Button& button : m_buttons )
	{
		button.Reset();
	}


}
