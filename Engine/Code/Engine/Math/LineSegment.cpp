#include "LineSegment.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

LineSegment::LineSegment( const Vec2& start, const Vec2& forwardNormal, float length )
{
	m_start = start;
	m_forwardNormal = forwardNormal.GetNormalized();
	m_length = length;
}

const Vec2 LineSegment::GetEndPoint() const
{
	return m_start + m_forwardNormal * m_length;
}

const Vec2 LineSegment::GetNearestPoint( const Vec2& referencePoint ) const
{
	Vec2 endPosition = GetEndPoint();
	Vec2 startToPointDisp = referencePoint - m_start;
	Vec2 endToPointDisp = referencePoint - endPosition;

	if( DotProduct2D( startToPointDisp, m_forwardNormal ) < 0.f )
	{
		return m_start;
	}
	else if( DotProduct2D( endToPointDisp, m_forwardNormal ) > 0.f )
	{
		return endPosition;
	}
	else
	{
		Vec2 projectionOntoSEVector = GetProjectedOnto2D( startToPointDisp, m_forwardNormal.GetNormalized() );
		Vec2 nearestPointOnBone =  m_start + projectionOntoSEVector;
		return nearestPointOnBone;
	}
}

float LineSegment::GetMinX()
{
	return ( m_start.x < GetEndPoint().x ) ? m_start.x :  GetEndPoint().x;
}

float LineSegment::GetMaxX()
{
	return ( m_start.x > GetEndPoint().x ) ? m_start.x :  GetEndPoint().x;
}

float LineSegment::GetMinY()
{
	return ( m_start.y < GetEndPoint().y ) ? m_start.y :  GetEndPoint().y;
}

float LineSegment::GetMaxY()
{
	return ( m_start.y > GetEndPoint().y ) ? m_start.y : GetEndPoint().y;
}

float LineSegment::CalculateYForX( float x )
{
	// y = m * x + b
	float m = m_forwardNormal.x / m_forwardNormal.y;
	float b = m_start.y - ( m * m_start.x );

	float y =  m * x + b;
	return y;
}

void LineSegment::Translate( const Vec2& translation )
{
	UNUSED( translation );
}

void LineSegment::operator=( const LineSegment& assignFrom )
{
	m_start = assignFrom.m_start;
	m_forwardNormal = assignFrom.m_forwardNormal;
	m_length = assignFrom.m_length;
}
