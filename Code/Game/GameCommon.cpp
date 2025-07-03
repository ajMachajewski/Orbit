#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

//--------------------------------------------------------------------------------------------------------------
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color )
{
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;

	const float innerRadius = radius - 0.5f * thickness;
	const float outerRadius = radius + 0.5f * thickness;

	const float incrementDegrees = 360.f / NUM_SIDES;

	Vertex_PCU m_ringVerts[NUM_VERTS];

	for ( int sideIndex = 0; sideIndex < NUM_SIDES; sideIndex++ )
	{
		// Left and right are from the perspective of the ring's center.
		Vec2 pointOuterLeft, pointOuterRight, pointInnerLeft, pointInnerRight;
		float rightDegrees = incrementDegrees * sideIndex;
		float leftDegrees  = incrementDegrees * ( sideIndex + 1 );

		pointOuterLeft  = Vec2::MakeFromPolarDegrees( leftDegrees,  outerRadius );
		pointOuterRight = Vec2::MakeFromPolarDegrees( rightDegrees, outerRadius );
		pointInnerLeft  = Vec2::MakeFromPolarDegrees( leftDegrees,  innerRadius );
		pointInnerRight = Vec2::MakeFromPolarDegrees( rightDegrees, innerRadius );

		// Translate points to the ring's center
		pointOuterLeft  += center;
		pointOuterRight += center;
		pointInnerLeft  += center;
		pointInnerRight += center;

		// Populate verticies
		// Note: 2 triangles per side and 6 verticies per side
		
		// Inner Triangle
		m_ringVerts[sideIndex * 6 + 0].m_position = Vec3( pointInnerRight );
		m_ringVerts[sideIndex * 6 + 1].m_position = Vec3( pointOuterLeft  );
		m_ringVerts[sideIndex * 6 + 2].m_position = Vec3( pointInnerLeft  );
		// Outer Triangle
		m_ringVerts[sideIndex * 6 + 3].m_position = Vec3( pointInnerRight );
		m_ringVerts[sideIndex * 6 + 4].m_position = Vec3( pointOuterRight );
		m_ringVerts[sideIndex * 6 + 5].m_position = Vec3( pointOuterLeft  );
	}

	// Apply color
	for ( int i = 0; i < NUM_VERTS; i++ )
		m_ringVerts[i].m_color = color;

	// Render
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( NUM_VERTS, m_ringVerts );
}

//--------------------------------------------------------------------------------------------------------------
void DebugDrawLine( Vec2 const& posA, Vec2 const& posB, float thickness, Rgba8 const& color )
{
	constexpr int NUM_VERTS = 6;
	thickness *= 0.5f;

	Vertex_PCU m_lineVerts[NUM_VERTS];

	Vec2 displacement = posB - posA;
	Vec2 parallelOffset = thickness * displacement.GetNormalized();
	Vec2 perpendicularOffset = parallelOffset.GetRotated90Degrees();

	Vec2 pointALeft = posA + perpendicularOffset - parallelOffset;
	Vec2 pointARight = posA - perpendicularOffset - parallelOffset;
	Vec2 pointBLeft = posB + perpendicularOffset + parallelOffset;
	Vec2 pointBRight = posB - perpendicularOffset + parallelOffset;


	// Construct Triangles
	m_lineVerts[0].m_position = Vec3( pointALeft );
	m_lineVerts[1].m_position = Vec3( pointARight );
	m_lineVerts[2].m_position = Vec3( pointBLeft );

	m_lineVerts[3].m_position = Vec3( pointBLeft );
	m_lineVerts[4].m_position = Vec3( pointARight );
	m_lineVerts[5].m_position = Vec3( pointBRight );

	// Apply color
	for ( int i = 0; i < NUM_VERTS; i++ )
		m_lineVerts[i].m_color = color;

	// Render
	g_theRenderer->DrawVertexArray( NUM_VERTS, m_lineVerts );
}

//--------------------------------------------------------------------------------------------------------------
void DebugDrawCircle( Vec2 const& center, float radius, Rgba8 const& color )
{
	constexpr int NUM_TRIS = 12;
	constexpr int NUM_VERTS = NUM_TRIS * 3;

	Vertex_PCU m_circleVerts[NUM_VERTS];

	float angleStepDegrees = 360.f / NUM_TRIS;

	for ( int triIndex = 0; triIndex < NUM_TRIS; triIndex++ )
	{
		m_circleVerts[3 * triIndex].m_position = center;
		m_circleVerts[3 * triIndex + 1].m_position = center + Vec2::MakeFromPolarDegrees( triIndex * angleStepDegrees, radius );
		m_circleVerts[3 * triIndex + 2].m_position = center + Vec2::MakeFromPolarDegrees( ( triIndex + 1 ) * angleStepDegrees, radius );
		m_circleVerts[3 * triIndex].m_color = color;
		m_circleVerts[3 * triIndex + 1].m_color = color;
		m_circleVerts[3 * triIndex + 2].m_color = color;
	}

	g_theRenderer->DrawVertexArray( NUM_VERTS, m_circleVerts );
}


//----------------------------------------------------------------------------------------------------------
float GetNormalizedAngle( float angle )
{
	while ( angle < 0.f ) angle += 360.f;
	while ( angle >= 360.f ) angle -= 360.f;

	return angle;
}


//----------------------------------------------------------------------------------------------------------
float GetAngularDisplacement( float fromDegrees, float toDegrees, bool clockwise )
{
	if ( clockwise )
	{
		while ( toDegrees > fromDegrees )
		{
			toDegrees -= 360.f;
		}

		return toDegrees - fromDegrees;
	}
	else
	{
		while ( toDegrees < fromDegrees )
		{
			toDegrees += 360.f;
		}

		return toDegrees - fromDegrees;
	}
}


//----------------------------------------------------------------------------------------------------------
Clock* GetGameClock()
{
	if ( g_theApp == nullptr )
		return nullptr;

	if ( g_theApp->m_theGame == nullptr )
		return nullptr;

	return g_theApp->m_theGame->GetClock();
}
