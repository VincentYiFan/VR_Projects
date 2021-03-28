#include "Capsule2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule2::Capsule2( const Vec2& start, const Vec2& end, float radius )
{
	m_boneStart = start;
	m_boneEnd = end;
	m_radius = radius;
}

bool Capsule2::IsPointInside( const Vec2& point ) const
{
	Vec2 nearestPointOnLineSegment = GetNearestPointOnLineSegment2D( point, m_boneStart, m_boneEnd );
	return ( GetDistanceSquared2D( point, nearestPointOnLineSegment ) <= (m_radius * m_radius) );
}

const Vec2 Capsule2::GetNearestPoint( const Vec2& referencePoint ) const
{
	if( IsPointInside( referencePoint ) )
		return referencePoint;

	Vec2 startToPointDisp = referencePoint - m_boneStart;
	Vec2 endToPointDisp = referencePoint - m_boneEnd;
	Vec2 forwardDir = m_boneEnd - m_boneStart;
	 
	if( DotProduct2D( startToPointDisp, forwardDir ) < 0.f )
	{
		return m_boneStart + startToPointDisp.GetNormalized() * m_radius;
	}
	else if( DotProduct2D( endToPointDisp, forwardDir ) > 0.f )
	{
		return m_boneEnd + endToPointDisp.GetNormalized() * m_radius;
	}
	else
	{
		Vec2 projectionOntoSEVector = GetProjectedOnto2D( startToPointDisp, forwardDir.GetNormalized() );
		Vec2 nearestPointOnBone =  m_boneStart + projectionOntoSEVector ;
		Vec2 nearestPointToStartDisp = m_boneStart - nearestPointOnBone;
		return nearestPointOnBone + ( -projectionOntoSEVector + startToPointDisp ).GetNormalized() * m_radius;
	}
}

void Capsule2::Translate( const Vec2& translation )
{
	m_boneStart += translation;
	m_boneEnd += translation;
}

void Capsule2::RotateDegrees( float relativeRotationDegrees )
{
	Mat44 rotateMatrix = Mat44::CreateZRotationDegrees( relativeRotationDegrees );
	Vec2 newForwardVector = rotateMatrix.TransformVector2D( m_boneEnd - m_boneStart );
	m_boneEnd = m_boneStart + newForwardVector;
}

void Capsule2::SetStartPosition( const Vec2& newStartPosition )
{
	m_boneStart = newStartPosition;
}

void Capsule2::SetEndPosition( const Vec2& newEndPosition )
{
	m_boneEnd = newEndPosition;
}

void Capsule2::SetRadius( float newRadius )
{
	m_radius = newRadius;
}

void Capsule2::operator=( const Capsule2& assignFrom )
{
	m_boneStart = assignFrom.m_boneStart;
	m_boneEnd = assignFrom.m_boneEnd;
	m_radius = assignFrom.m_radius;
}
