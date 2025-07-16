#include "Game/Prop.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"


//----------------------------------------------------------------------------------------------------------
Prop::Prop( Vec2 const& position /*= Vec2::ZERO */ )
	: m_position( position )
{
	Clock* clock = GetGameClock();
	if ( clock == nullptr )
	{
		ERROR_AND_DIE( "ERROR: Trying to create a prop without having made the game clock first!" );
	}

	m_startTimeSeconds = clock->GetTotalSeconds();
}


//----------------------------------------------------------------------------------------------------------
Prop::Prop( Vec2 const& position, Rgba8Gradient const& colorGradient, float lifetime )
	: m_position( position )
	, m_colorGradient( colorGradient )
	, m_lifetimeSeconds( lifetime )
{
	Clock* clock = GetGameClock();
	if ( clock == nullptr )
	{
		ERROR_AND_DIE( "ERROR: Trying to create a prop without having made the game clock first!" );
	}

	m_startTimeSeconds = clock->GetTotalSeconds();
}


//----------------------------------------------------------------------------------------------------------
Prop::~Prop()
{
	ResetBuffers();
}


//----------------------------------------------------------------------------------------------------------
void Prop::SetRenderData( IndexedMesh const& meshToCopy, Texture* texture )
{
	ResetBuffers();
	m_vertCount = g_theRenderer->CreateNewBuffersFromIndexedMesh( meshToCopy, &m_vbo, &m_ibo );
	m_texture = texture;
}


//----------------------------------------------------------------------------------------------------------
void Prop::Render() const
{
	if ( m_vbo == nullptr || m_ibo == nullptr )
		return;

	Clock* clock = GetGameClock();
	if ( clock == nullptr )
		return;

	double currentTime = clock->GetTotalSeconds();
	float timeSinceStart = static_cast<float>( currentTime - m_startTimeSeconds );
	float lifetimeFraction = timeSinceStart / m_lifetimeSeconds;
	Rgba8 color = m_colorGradient.GetColor( lifetimeFraction );

	Mat44 transform = Mat44::MakeTranslation2D( m_position );
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( transform, color );
	g_theRenderer->DrawIndexedVertexBuffer( m_vbo, m_ibo, m_vertCount );
}


//----------------------------------------------------------------------------------------------------------
bool Prop::IsGarbage() const
{
	if ( m_lifetimeSeconds < 0.f )	// Negative lifetime props are never garbage
		return false;
	
	if ( m_repeatLifetime )			// Similarly, props that repeat their lifetime are never garbage
		return false;

	Clock* clock = GetGameClock();
	if ( clock == nullptr )
		return true;				// If the game clock doesn't exist, we can't meaningfully track lifetimes. Default to garbage.

	double currentTime = clock->GetTotalSeconds();
	float timeSinceStart = static_cast<float>( currentTime - m_startTimeSeconds );
	if ( timeSinceStart > m_lifetimeSeconds )
		return true;				// Props that have lived longer than their lifetime are garbage

	return false;
}


//----------------------------------------------------------------------------------------------------------
void Prop::ResetBuffers()
{
	if ( m_vbo != nullptr )
	{
		delete m_vbo;
		m_vbo = nullptr;
	}

	if ( m_ibo != nullptr )
	{
		delete m_ibo;
		m_ibo = nullptr;
	}
}
