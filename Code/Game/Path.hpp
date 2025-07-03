#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <string>
#include <vector>


//----------------------------------------------------------------------------------------------------------
class Conductor;


//----------------------------------------------------------------------------------------------------------
class PathNode
{
	friend class Path;

public:
	PathNode() = default;

private:
	void InitializeVerts( Vec2 const& inNormal, Vec2 const& outNormal, float width, float borderThickness, 
		bool spin = false, int speedChange = 0, Rgba8 const& baseColor = Rgba8::WHITE, Rgba8 const& borderColor = Rgba8::BLACK );

	void Render() const;
	void DebugRender() const;

public:
	Vec2 const& GetPosition() const;

private:
	VertexBuffer* m_vbo = nullptr;
	int m_vertCount = 0;
	Vec2 m_position = Vec2::ZERO;

public:
	bool m_clockwise = true;
	double m_timeInBeats = 0.f;
	float m_speed = 1.0;
	float m_durationInBeats = 1.f;
	float m_angle = 180.f;
	float m_radius = 5.f;
};


//----------------------------------------------------------------------------------------------------------
class Path
{
public:
	Path( Conductor const& conductor );
	~Path();

	bool LoadFromFile( const char* filepath );

	void Render() const;
	void DebugRender() const;

	void AddNode( NamedStrings& arguments );

	PathNode const* GetNode( int index ) const;
	PathNode const* GetLastNode() const;

private:
	Conductor const& m_conductor;
	std::vector<PathNode> m_nodes;
	std::string m_name;
	float m_scale = 1.f;
	float m_pathWidth = .8f;

	double m_totalTimeInBeats = 0.0;
};