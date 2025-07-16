#include "Game/Path.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Conductor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"


//----------------------------------------------------------------------------------------------------------
void PathNode::InitializeVerts( Vec2 const& inNormal, Vec2 const& outNormal, float width, float borderThickness, 
	bool spin, int speedChange, Rgba8 const& baseColor, Rgba8 const& borderColor )
{
	Mesh mesh;
	float halfWidth = .5f * width;
	bool is360 = ( inNormal + outNormal ).GetLengthSquared() < 0.001f;

	Vec2 const& nodeCenter = m_position;
	Vec2 inTangent = inNormal.GetRotated90Degrees();	// Tangent to the inward edge, orthogonal to inNormal
	Vec2 outTangent = outNormal.GetRotated90Degrees();	// Tangent to the outward edge, othrogonal to outNormal
	Vec2 halfTangent = ( inTangent + outTangent ).GetNormalized();
	
	Vec2 inCenter	= nodeCenter - ( m_radius * inNormal );
	Vec2 inLeft		= inCenter + ( halfWidth * inTangent );
	Vec2 inRight	= inCenter - ( halfWidth * inTangent );

	Vec2 outCenter	= nodeCenter + ( m_radius * outNormal );
	Vec2 outLeft	= outCenter + ( halfWidth * outTangent );
	Vec2 outRight	= outCenter - ( halfWidth * outTangent );

	Vec2 centerDisplacement = outCenter - inCenter;
	Vec2 leftDisplacement = outLeft - inLeft;
	Vec2 rightDisplacement = outRight - inRight;
	float centerDistance = centerDisplacement.GetLength();
	float leftDistance = leftDisplacement.GetLength();
	float rightDistance = rightDisplacement.GetLength();
	float distToSideLengthRatio = m_radius / centerDistance;
	float leftSideLength = leftDistance * distToSideLengthRatio;
	float rightSideLength = rightDistance * distToSideLengthRatio;
	Vec2 cornerLeft = inLeft + ( leftSideLength * inNormal );
	Vec2 cornerRight = inRight + ( rightSideLength * inNormal );

	Vec2 innerInLeft		= inLeft + borderThickness * ( .5f * inNormal - inTangent );
	Vec2 innerInRight		= inRight + borderThickness * ( .5f * inNormal + inTangent );
	Vec2 innerOutLeft		= outLeft + borderThickness * ( -.5f * outNormal - outTangent );
	Vec2 innerOutRight		= outRight + borderThickness * ( -.5f * outNormal + outTangent );

	Vec2 innerLeftDisplacement = innerOutLeft - innerInLeft;
	Vec2 innerRightDisplacement = innerOutRight - innerInRight;
	float innerLeftDistance = innerLeftDisplacement.GetLength();
	float innerRightDistance = innerRightDisplacement.GetLength();
	float innerLeftLength = innerLeftDistance * distToSideLengthRatio;
	float innerRightLength = innerRightDistance * distToSideLengthRatio;
	Vec2 innerCornerLeft = innerInLeft + ( innerLeftLength * inNormal );
	Vec2 innerCornerRight	= innerInRight + ( innerRightLength * inNormal );

	if ( is360 )
	{
		Vec2 centerLeft = m_position + ( halfWidth * inTangent );
		Vec2 centerRight = m_position - ( halfWidth * inTangent );
		Vec2 innerCenterLeft = centerLeft - ( borderThickness * inTangent );
		Vec2 innerCenterRight = centerRight + ( borderThickness * inTangent );

		AddVertsForDisc2D( mesh, m_position, halfWidth, borderColor );
		AddVertsForQuad2D( mesh, inLeft, inRight, centerRight, centerLeft, borderColor );
		AddVertsForDisc2D( mesh, m_position, halfWidth - borderThickness, baseColor );
		AddVertsForQuad2D( mesh, innerInLeft, innerInRight, innerCenterRight, innerCenterLeft, baseColor );
	}
	else
	{
		AddVertsForQuad2D( mesh, inLeft, inRight, cornerRight, cornerLeft, borderColor );
		AddVertsForQuad2D( mesh, cornerLeft, cornerRight, outRight, outLeft, borderColor );
		AddVertsForQuad2D( mesh, innerInLeft, innerInRight, innerCornerRight, innerCornerLeft, baseColor );
		AddVertsForQuad2D( mesh, innerCornerLeft, innerCornerRight, innerOutRight, innerOutLeft, baseColor );
	}

	// DEBUG RENDER STUFF:
// 	AddVertsForLineSegment2D( mesh, inLeft, inRight, .125f * width, Rgba8( 0, 255, 0, 127 ) );
// 	AddVertsForLineSegment2D( mesh, outLeft, outRight, .125f * width, Rgba8( 255, 0, 0, 127 ) );
	Rgba8 dotColor	= Rgba8( 0, 0, 0, 200 );
	float dotRadius	= .1f * width;
	if ( speedChange > 0 )		// Fast
	{
		dotColor = Rgba8( 255, 127, 0 );	// Orange
		dotRadius = .25f * width;
	}
	else if ( speedChange < 0 )	// Slow
	{
		dotColor = Rgba8( 0, 127, 255 );	// Dark cyan
		dotRadius = .25f * width;
	}
	else if ( spin )
	{
		dotColor = Rgba8( 0, 255, 75, 255 );
		dotRadius = .25f * width;
	}
	else if ( m_checkpoint )
	{
		dotColor = Rgba8::PASTEL_CYAN;
		dotRadius = .3f * width;
	}

 	AddVertsForDisc2D( mesh, m_position, dotRadius, dotColor, 16 );

	m_vertCount = static_cast<int>( mesh.size() );
	m_vbo = g_theRenderer->CreateVertexBuffer( m_vertCount * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( mesh.data(), m_vertCount * sizeof( Vertex_PCU ), m_vbo );
}


//----------------------------------------------------------------------------------------------------------
void PathNode::Render() const
{
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexBuffer( m_vbo, m_vertCount );
}


//----------------------------------------------------------------------------------------------------------
void PathNode::DebugRender() const
{
	std::string info = Stringf( "%3.2f", m_timeInBeats );
	Mat44 transform = Mat44::MakeTranslation3D( Vec3( m_position + Vec2::UP * m_radius, 1 ) );
	transform.AppendZRotation( -90 );
	transform.AppendYRotation( -90 );

	DebugAddWorldText( info, transform, m_radius * .5f, Vec2( 0.5, 0.5 ), 0.f, Rgba8::DARK_GREEN, Rgba8::DARK_GREEN );
}


//----------------------------------------------------------------------------------------------------------
Vec2 const& PathNode::GetPosition() const
{
	return m_position;
}


//----------------------------------------------------------------------------------------------------------
Path::Path( Conductor const& conductor )
	: m_conductor( conductor )
{
}


//----------------------------------------------------------------------------------------------------------
Path::~Path()
{
	for ( PathNode& node : m_nodes )
	{
		delete node.m_vbo;
		node.m_vbo = nullptr;
	}
}


//----------------------------------------------------------------------------------------------------------
bool Path::LoadFromFile( const char* filepath )
{
	// Load File
	XmlDocument document;
	XmlResult result = document.LoadFile( filepath );
	if ( result != tinyxml2::XML_SUCCESS )
		return false;

	XmlElement const* rootElement = document.RootElement();
	if ( rootElement == nullptr )
		return false;

	// Parse global path info
	NamedStrings pathArgs;
	pathArgs.PopulateFromXmlElementAttributes( *rootElement );

	m_name		= pathArgs.GetValue( "name", filepath );
	m_pathWidth = pathArgs.GetValue( "width", .8f );
	m_scale		= pathArgs.GetValue( "scale", 1.f );

	int nodeCount = rootElement->ChildElementCount( "Node" );
	m_nodes.reserve( nodeCount );
	XmlElement const* nodeElement = rootElement->FirstChildElement( "Node" );
	while ( nodeElement != nullptr )
	{
		NamedStrings nodeArguments;
		nodeArguments.PopulateFromXmlElementAttributes( *nodeElement );
		AddNode( nodeArguments );
		nodeElement = nodeElement->NextSiblingElement( "Node" );
	}

	return true;
}


//----------------------------------------------------------------------------------------------------------
void Path::Render() const
{
	int nodeCount = static_cast<int>( m_nodes.size() );
	for ( int nodeIndex = nodeCount - 1; nodeIndex >= 0; nodeIndex-- )
	{
		PathNode const& node = m_nodes[nodeIndex];
		node.Render();
	}
}


//----------------------------------------------------------------------------------------------------------
void Path::DebugRender() const
{
	int nodeCount = static_cast<int>( m_nodes.size() );
	for ( int nodeIndex = nodeCount - 1; nodeIndex >= 0; nodeIndex-- )
	{
		PathNode const& node = m_nodes[nodeIndex];
		node.DebugRender();
	}
}


//----------------------------------------------------------------------------------------------------------
void Path::AddNode( NamedStrings& arguments )
{
	float timeInBeats = arguments.GetValue( "beat", 1.f );
	float deltaAngle = RangeMap( timeInBeats, 2.f, 0.f, -180.f, 180.f );

	if ( m_nodes.size() == 0 )
	{
		m_nodes.emplace_back();
		PathNode& newNode = m_nodes.back();
		newNode.m_durationInBeats = timeInBeats;
		newNode.m_angle = 0.f;
		newNode.m_speed = 1.f;
		newNode.m_radius = .5f * m_scale;
		Vec2 outNormal = Vec2::MakeFromPolarDegrees( deltaAngle );
		newNode.InitializeVerts( Vec2::RIGHT, outNormal, m_pathWidth, 0.125f * m_pathWidth );

		m_totalTimeInBeats += timeInBeats;
		return;
	}

	PathNode& prevNode = m_nodes.back();
	bool spin = arguments.GetValue( "spin", false );
	float speed = arguments.GetValue( "speed", prevNode.m_speed );
	timeInBeats /= speed;

#if defined( _DEBUG )
	if ( deltaAngle > 360.f )
	{
		ERROR_RECOVERABLE( "Tried to add a path node with a change in angle over 360 degrees! This may lead to desync!" );
	}
#endif

	bool isClockwise = spin ? !prevNode.m_clockwise : prevNode.m_clockwise;
	float turnDirection = isClockwise ? 1.f : -1.f;
	float prevAngle = prevNode.m_angle;
	float angle = prevAngle + ( turnDirection * deltaAngle );
	angle = GetNormalizedAngle( angle );

	Vec2 prevPosition = prevNode.m_position;
	Vec2 inDirection = Vec2::MakeFromPolarDegrees( prevAngle );
	Vec2 position = prevPosition + ( inDirection * m_scale );

	m_nodes.emplace_back();
	PathNode& newNode = m_nodes.back();
	newNode.m_position = position;
	newNode.m_durationInBeats = timeInBeats;
	newNode.m_timeInBeats = m_totalTimeInBeats;
	newNode.m_angle = angle;
	newNode.m_radius = .5f * m_scale;
	newNode.m_clockwise = isClockwise;
	newNode.m_speed = speed;
	newNode.m_checkpoint = arguments.GetValue( "checkpoint", false );

	Vec2 outDirection = Vec2::MakeFromPolarDegrees( angle );
	int speedChange = 0;
	if ( speed > prevNode.m_speed )			speedChange = 1;
	else if ( speed < prevNode.m_speed )	speedChange = -1;
	newNode.InitializeVerts( inDirection, outDirection, m_pathWidth, 0.125f * m_pathWidth, spin, speedChange );

	m_totalTimeInBeats += timeInBeats;
}


//----------------------------------------------------------------------------------------------------------
PathNode const* Path::GetNode( int index ) const
{
	if ( index < 0 || index >= static_cast<int>( m_nodes.size() ) )
		return nullptr;

	return &m_nodes[index];
}


//----------------------------------------------------------------------------------------------------------
PathNode const* Path::GetLastNode() const
{
	int nodeCount = static_cast<int>( m_nodes.size() );
	if ( nodeCount == 0 )
		return nullptr;

	return &m_nodes[nodeCount - 1];
}


//----------------------------------------------------------------------------------------------------------
unsigned int Path::GetNodeCount() const
{
	return static_cast<unsigned int>( m_nodes.size() );
}


//----------------------------------------------------------------------------------------------------------
float Path::GetWidth() const
{
	return m_pathWidth;
}
