#include "Game/Button.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Renderer.hpp"


//----------------------------------------------------------------------------------------------------------
Button::Button( AABB2 const& bounds, std::string const& eventName, std::string const& label )
	: m_bounds( bounds )
	, m_eventName( eventName )
	, m_label( label )
{
}


//----------------------------------------------------------------------------------------------------------
void Button::Update( Vec2 const& cursorPosition )
{
	CheckCursorOverlap( cursorPosition );
	if ( m_pressed ) UpdatePressedState();
}


//----------------------------------------------------------------------------------------------------------
void Button::Render() const
{
	Mesh verts;
	Rgba8 color = ( m_hovered || m_selected ) ? m_selectedColor : m_defaultColor;
	if ( m_pressed ) color = m_pressedColor;
	AddVertsForAABB2D( verts, m_bounds, color );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawVertexArray( verts );
}


//----------------------------------------------------------------------------------------------------------
bool Button::TryClick()
{
	if ( m_hovered )
	{
		OnPress();
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
void Button::OnPress()
{
	m_pressed = true;
	m_timePressed = static_cast<float>( GetGameClock()->GetTotalSeconds() );

	EventArgs arguments;
	arguments.SetValue( "buttonEventName", m_eventName );
	g_theEventSystem->FireEvent( BUTTON_PRESS_EVENT_NAME, arguments );
}


//----------------------------------------------------------------------------------------------------------
bool Button::CheckCursorOverlap( Vec2 const& cursorPosition )
{
	m_hovered = m_bounds.IsPointInside( cursorPosition );
	return m_hovered;
}


//----------------------------------------------------------------------------------------------------------
void Button::UpdatePressedState()
{
	float currentTime = static_cast<float>( GetGameClock()->GetTotalSeconds() );
	if ( currentTime - m_timePressed > BUTTON_PRESS_TIME )
	{
		m_pressed = false;
	}
}

