#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <limits>

void PolygonCollider2D::UpdateWorldShape()
{
	if( m_rigidbody != nullptr ) {

		m_worldPosition = m_rigidbody->m_worldPosition;

		if( !isPosInitialized ) {
			m_lastFrameWorldPosition = m_worldPosition;
			isPosInitialized = true;
		}

		Vec2 frameDeltaPos = m_worldPosition - m_lastFrameWorldPosition;

		for( int pointIdx = 0; pointIdx < (int)m_points.size(); pointIdx++ ) {
			m_points[pointIdx] += frameDeltaPos;
		}

		m_lastFrameWorldPosition = m_worldPosition;
	}

	else { // No rigidbody attached
		m_worldPosition = m_localPosition;
	}

	m_worldBounds = GetWorldBounds();

	// Update vertexes positions according to the rotation
	UpdateVertexes();
}

void PolygonCollider2D::UpdateVertexes()
{
	float deltaRadians = m_rigidbody->GetRotationInRadians() - m_lastFrameRotationInRadians;

	for( int idx = 0; idx < (int)m_points.size(); ++idx )
	{
		Vec2 translation = m_points[idx] - m_rigidbody->m_worldPosition; // Displacement from the origin

		float newX = translation.x * cosf( deltaRadians ) - translation.y * sinf( deltaRadians );
		float newY = translation.x * sinf( deltaRadians ) + translation.y * cosf( deltaRadians );

		translation = Vec2( newX, newY );
		m_points[idx] = m_rigidbody->m_worldPosition + translation;
	}

	m_lastFrameRotationInRadians = m_rigidbody->GetRotationInRadians();
}

AABB2 PolygonCollider2D::GetWorldBounds() const
{
	float lowestPoint = 0.f; 
	float highestPoint = 0.f;
	float leftestPoint = 0.f; 
	float rightestPoint = 0.f;

	for( int pointIndex = 0; pointIndex < (int)m_points.size(); pointIndex++ ) {
		if( pointIndex == 0 ) { 
			// Initialize
			highestPoint	= m_points[0].y;
			lowestPoint		= m_points[0].y;
			leftestPoint	= m_points[0].x;
			rightestPoint	= m_points[0].x;
		}
		if( m_points[pointIndex].y > highestPoint ) {
			highestPoint = m_points[pointIndex].y;
		}
		if( m_points[pointIndex].y < lowestPoint ) {
			lowestPoint = m_points[pointIndex].y;
		}
		if( m_points[pointIndex].x < leftestPoint ) {
			leftestPoint = m_points[pointIndex].x;
		}
		if( m_points[pointIndex].x > rightestPoint ) {
			rightestPoint = m_points[pointIndex].x;
		}
	}

	AABB2 polygonBoundingBox = AABB2( Vec2( leftestPoint, lowestPoint ), Vec2( rightestPoint, highestPoint ) );
	return polygonBoundingBox;
}

bool PolygonCollider2D::IsValid() const
{
	return ( (int) m_points.size() >= 3 );
}

bool PolygonCollider2D::IsConvex() const
{
	return true;
}

bool PolygonCollider2D::Contains( Vec2 point ) const
{
	int numPoints = (int) m_points.size();
	int i, j = 0;
	bool result = false;
	for( i = 0, j = numPoints - 1; i < numPoints; j = i++ ) {
		if( ((m_points[i].y > point.y) != (m_points[j].y > point.y)) &&
			(point.x < (m_points[j].x - m_points[i].x)  * (point.y - m_points[i].y) / (m_points[j].y - m_points[i].y) + m_points[i].x) ) {
			result = !result;
		}
	}
	return result;
}

float PolygonCollider2D::Calculatemoment( float mass )
{
	float sum1 = 0.f;
	float sum2 = 0.f;

	for( int i = 0; i < (int)m_points.size(); ++i )
	{
		
		Vec2 p1 = m_points[i] - m_worldPosition;
		Vec2 p2 = m_points[(i+1) % (int)m_points.size()] - m_worldPosition;
		float cross = CrossProduct2D( p1, p2 );
		sum1 += cross * ( DotProduct2D(p1, p1) + DotProduct2D(p1, p2) + DotProduct2D(p2, p2) );
		sum2 += cross;
	}

	return mass / 6.f * sum1 / sum2;
}


float PolygonCollider2D::GetDistance( Vec2 point ) const
{
	return GetDistance2D( point, m_worldPosition );
}

Vec2 PolygonCollider2D::GetClosestPoint( Vec2 point ) const
{
	std::vector<Vec2> closestPoints;
	for( int pointIndex = 0; pointIndex < (int)m_points.size() - 1; pointIndex++ )
	{
		LineSegment* lineSegment = new LineSegment( m_points[pointIndex], m_points[pointIndex + 1] - m_points[pointIndex], GetDistance2D( m_points[pointIndex], m_points[pointIndex + 1]) );
		closestPoints.push_back( lineSegment->GetNearestPoint( point ) );
	}

	// Line between the first point and the last point
	LineSegment* lineSegment = new LineSegment( m_points[0], m_points[GetVertexCount() - 1] - m_points[0], GetDistance2D( m_points[0], m_points[GetVertexCount() - 1]) );
	closestPoints.push_back( lineSegment->GetNearestPoint( point ) );

	Vec2 closestPoint = Vec2::ZERO;

	for( int index = 0; index < (int) closestPoints.size(); index++ )
	{
		if( index == 0 ) {
			closestPoint = closestPoints[0];
		}
		if( GetDistanceSquared2D( point, closestPoints[index] ) < GetDistanceSquared2D( point, closestPoint ) )
		{
			closestPoint = closestPoints[index];
		}
	}

	return closestPoint;
}

int PolygonCollider2D::GetVertexCount() const
{
	return (int) m_points.size();
}

int PolygonCollider2D::GetEdgeCount() const
{
	return (int) m_points.size();
}

void PolygonCollider2D::GetEdge( int index, Vec2* outStart, Vec2* outEnd )
{
	UNUSED(index);
	UNUSED(outStart);
	UNUSED(outEnd);
}

Vec2 PolygonCollider2D::GetCenter() const
{
	Vec2 sum = Vec2::ZERO;

	for( int idx = 0; idx < (int)m_points.size(); ++idx ) 
	{
		sum += m_points[idx];
	}

	return sum / (float)m_points.size();
}

void PolygonCollider2D::DebugRender( RenderContext* ctx, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	// Render the polygon shape
	ctx->DrawPolygon( m_points, fillColor );

	// Render the border
	int numPoints = GetVertexCount();

	for( int pointIndex = 0; pointIndex < numPoints - 1; pointIndex++ )
	{
		ctx->DrawLine( m_points[pointIndex], m_points[pointIndex + 1], borderColor, 0.3f );
	}
	ctx->DrawLine( m_points[numPoints - 1], m_points[0], borderColor, 0.3f );

	float rotationRadians = m_rigidbody->GetRotationInRadians();

	// Render the rigidbody's world position
	Vec2 topRight    =  m_worldPosition + Vec2( .5f, .5f ).GetRotatedRadians( rotationRadians );
	Vec2 topLeft	 =  m_worldPosition + Vec2( -.5f, .5f ).GetRotatedRadians( rotationRadians );
	Vec2 bottomLeft  =  m_worldPosition + Vec2( -.5f, -.5f ).GetRotatedRadians( rotationRadians );
	Vec2 bottomRight =  m_worldPosition + Vec2( .5f, -.5f ).GetRotatedRadians( rotationRadians );

	Rgba8 rigidbodyPosColor = Rgba8::BLUE;
	if( !m_rigidbody->m_isEnabled ) {
		rigidbodyPosColor = Rgba8::RED;
	}
	ctx->DrawLine( bottomLeft, topRight, rigidbodyPosColor, 0.3f );
	ctx->DrawLine( bottomRight, topLeft, rigidbodyPosColor, 0.3f );

	// Render the world bounds
	//ctx->DrawAABB2( m_worldBounds, Rgba8( 150, 150, 150, 100 ) );
}

void PolygonCollider2D::Destroy()
{
	m_isGarbage = true;
}

Vec2 PolygonCollider2D::Support( Vec2 dir ) const
{
	float furthestDistance = -( std::numeric_limits<float>::infinity() );
	Vec2 furthestPoint = Vec2::ZERO;

	int numOfPoints = (int)m_points.size();

	for( int idx = 0; idx < numOfPoints; ++idx )
	{
		float currentDistance = DotProduct2D( m_points[idx], dir );

		if( currentDistance > furthestDistance )
		{
			furthestDistance = currentDistance;
			furthestPoint = m_points[idx];
		}
	}

	return furthestPoint;
}

Edge PolygonCollider2D::BestEdge( Vec2 dir ) const
{
	// step 1
	// find the farthest vertex the in polygon along the separation normal
	float max = -(std::numeric_limits<float>::infinity());
	int index = 0;
	int c = (int)m_points.size();
	for( int i = 0; i < c; ++i )
	{
		float projection = DotProduct2D( dir, m_points[i] );
		if( projection > max )
		{
			max = projection;
			index = i;
		}
	}

	// step 2
	//now we need to use the edge that is most perpendicular, either the right or the left
	Vec2 v = m_points[index];

	int nextIndex = index + 1;
	if( nextIndex >= c ) {
		nextIndex = 0;
	}
	Vec2 v1 = m_points[nextIndex];

	int prevIndex = index - 1;
	if( prevIndex < 0 ) {
		prevIndex = c - 1;
	}
	Vec2 v0 = m_points[prevIndex];

	// v1 to v
	Vec2 l = v - v1;
	// v0 to v
	Vec2 r = v - v0;

	// normalize
	l.Normalize();
	r.Normalize();


	// the edge that is most perpendicular
	// to n will have a dot product closer to zero
	LineSegment bestEdge;
	if( DotProduct2D( r, dir ) <= DotProduct2D( l, dir ) ) {
		return Edge( v, v0, v );
	}
	else {
		return Edge( v, v, v1 );
	}
}

PolygonCollider2D::PolygonCollider2D( std::vector<Vec2> points /*, uint pointCount */)
{
	m_points = points;

	GUARANTEE_OR_DIE( IsValid(), "Polygon Invalid: Failed to create polygon." );
	GUARANTEE_OR_DIE( IsConvex(), "Polygon isn't convex." );
}

PolygonCollider2D::~PolygonCollider2D()
{
}

