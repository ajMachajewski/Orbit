#include "Game/Button.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Renderer.hpp"


//----------------------------------------------------------------------------------------------------------
CardinalDirection GetOppositeDirection( CardinalDirection direction )
{
	switch ( direction )
	{
		case EAST:	return WEST;
		case WEST:	return EAST;
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		default:	return direction;
	}
}



//----------------------------------------------------------------------------------------------------------
Button::Button( AABB2 const& bounds, std::string const& eventName, std::string const& label )
	: m_bounds( bounds )
	, m_eventName( eventName )
	, m_label( label )
{
}


//----------------------------------------------------------------------------------------------------------
void Button::LinkTo( Button& otherButton, CardinalDirection direction, bool oneWay )
{
	m_neighbors[direction] = &otherButton;
	if ( oneWay )
		return;

	CardinalDirection oppositeDirection = GetOppositeDirection( direction );
	otherButton.m_neighbors[oppositeDirection] = this;
}


//----------------------------------------------------------------------------------------------------------
void Button::Reset()
{
	m_pressed = false;
	m_hovered = false;
	m_selected = false;
	m_timePressed = -100.f;
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
	Rgba8 color;
	if		( m_pressed )	color = m_pressedColor;
	else if ( m_hovered )	color = m_hoveredColor;
	else if ( m_selected )	color = m_selectedColor;
	else					color = m_defaultColor;

	AddVertsForAABB2D( verts, m_bounds, color );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::READ_WRITE_LESS_EQUAL );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->DrawVertexArray( verts );

	IndexedMesh labelMesh;
	AABB2 textBoxBounds = m_bounds;
	textBoxBounds.PadAllSides( -10.f );
	float const& height = m_bounds.GetDimensions().y;
	g_defaultFont->AddVertsForTextInBox2D( labelMesh, m_label, textBoxBounds, height, Rgba8::BLACK, .66f );
	g_theRenderer->BindTexture( &g_defaultFont->GetTexture() );
	g_theRenderer->DrawIndexedMesh( labelMesh );
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
	
		EventArgs arguments;
		arguments.SetValue( "buttonEventName", m_eventName );
		g_theEventSystem->FireEvent( BUTTON_PRESS_EVENT_NAME, arguments );
	}
}
