#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8Gradient.hpp"


//----------------------------------------------------------------------------------------------------------
class VertexBuffer;
class IndexBuffer;
class Texture;
struct IndexedMesh;


//----------------------------------------------------------------------------------------------------------
class Prop
{
public:
	Prop( Vec2 const& position = Vec2::ZERO );
	Prop( Vec2 const& position, Rgba8Gradient const& colorGradient, float lifetime = -1.0f );
	~Prop();

	void SetRenderData( IndexedMesh const& meshToCopy, Texture* texture = nullptr );
	void Render() const;
	bool IsGarbage() const;

private:
	void ResetBuffers();

private:
	Texture*		m_texture = nullptr;
	VertexBuffer*	m_vbo = nullptr;
	IndexBuffer*	m_ibo = nullptr;
	unsigned int	m_vertCount = 0;
	float			m_lifetimeSeconds = -1.0;	// Negative means never destroy
	double			m_startTimeSeconds;
	bool			m_repeatLifetime = false;

	Vec2 m_position;
	Rgba8Gradient m_colorGradient = Rgba8Gradient( Rgba8::WHITE, Rgba8::TRANSPARENT_WHITE );
};