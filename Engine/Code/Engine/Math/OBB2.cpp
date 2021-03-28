#include "OBB2.hpp"
#include "Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

const OBB2 OBB2::ZERO_TO_ONE( Vec2( 0.5f, 0.5f ), Vec2( 1.f, 1.f ), Vec2( 1.f, 0.f ) );

void OBB2::RotateByDegrees( float deltaRelativeDegrees )
{
	float c = CosDegrees( deltaRelativeDegrees );
	float s = SinDegrees( deltaRelativeDegrees );

	float x = m_iBasis.x;
	float y = m_iBasis.y;
	m_iBasis.x = ( c * x ) - ( s * y );
	m_iBasis.y = ( s * x ) + ( c * y );
}

void OBB2::StretchToIncludePoint( const Vec2& point )
{
	UNUSED( point );
	//#SD1ToDo
}

void OBB2::Fix()
{
	m_halfDimensions.x = fabsf( m_halfDimensions.x );
	m_halfDimensions.y = fabsf( m_halfDimensions.y );

	m_iBasis.Normalize();
	if( m_iBasis == Vec2::ZERO )
	{
		m_iBasis = Vec2( 1.f, 0.f );
	}
}

void OBB2::SetOrientationDegrees( float completelyNewAbsoluteOrientation )
{
	m_iBasis = Vec2::MakeFromPolarDegrees( completelyNewAbsoluteOrientation );
}

OBB2::OBB2( const OBB2& copyFrom )
{
	m_center = copyFrom.m_center;
	m_iBasis = copyFrom.m_iBasis;
	m_halfDimensions = copyFrom.m_halfDimensions;

	Fix();
}

OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, const Vec2& iBasisNormal )
{
	m_center = center;
	m_iBasis = iBasisNormal;
	m_halfDimensions = 0.5f * fullDimensions;

	Fix();
}

OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, float orientationDegrees )
{
	m_center = center;
	RotateByDegrees( orientationDegrees );
	m_halfDimensions = 0.5f * fullDimensions;

	Fix();
}

OBB2::OBB2( const AABB2& asAxisAlignedBox, float orientationDegrees )
{
	m_center = asAxisAlignedBox.GetCenter();
	RotateByDegrees( orientationDegrees );
	m_halfDimensions = 0.5f * asAxisAlignedBox.GetDimensions();

	Fix();
}

bool OBB2::IsPointInside( const Vec2& point ) const
{
	Vec2 dispFromCenterToPoint = point - m_center;
	Vec2 jBasis = GetJBasisNormal();
	float projectionOntoIBasis = DotProduct2D( dispFromCenterToPoint, m_iBasis );
	float projectionOntoJBasis = DotProduct2D( dispFromCenterToPoint, jBasis );

	return( (fabsf( projectionOntoIBasis ) < m_halfDimensions.x) && (fabsf( projectionOntoJBasis ) < m_halfDimensions.y) );
}

const Vec2 OBB2::GetCenter() const
{
	return m_center;
}

const Vec2 OBB2::GetDimensions() const
{
	Vec2 dimensions = m_halfDimensions * 2.f;
	return dimensions;
}

const Vec2 OBB2::GetIBasisNormal() const
{
	return m_iBasis;
}

const Vec2 OBB2::GetJBasisNormal() const
{
	return Vec2( -m_iBasis.y, m_iBasis.x );
}

float OBB2::GetOrientationDegrees() const
{
	return m_iBasis.GetAngleDegrees();
}

const Vec2 OBB2::GetNearestPoint( const Vec2& referencePos ) const
{
	if( IsPointInside( referencePos ) )
	{
		return referencePos;
	}

	Vec2 dispFromCenterToPoint = referencePos - m_center;
	Vec2 jBasis = GetJBasisNormal();
	float projectionOntoIBasis = DotProduct2D( dispFromCenterToPoint, m_iBasis );
	float projectionOntoJBasis = DotProduct2D( dispFromCenterToPoint, jBasis );

	float nearestPointDirectionI = Clamp( projectionOntoIBasis, -m_halfDimensions.x, m_halfDimensions.x );
	float nearestPointDirectionJ = Clamp( projectionOntoJBasis, -m_halfDimensions.y, m_halfDimensions.y );

	Vec2 nearestPoint = m_center + ( nearestPointDirectionI * m_iBasis ) + ( nearestPointDirectionJ * jBasis );
	return nearestPoint;
}

const Vec2 OBB2::GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const
{
	Vec2 jBasis = GetJBasisNormal();
	Vec2 mins = m_center - ( m_halfDimensions.x * m_iBasis ) - ( m_halfDimensions.y * jBasis );

	Vec2 point = mins + ( uvCoordsZeroToOne.x * m_iBasis * m_halfDimensions.x * 2.f ) + ( uvCoordsZeroToOne.y * jBasis * m_halfDimensions.y * 2.f );
	return point;
}

const Vec2 OBB2::GetUVForPoint( const Vec2& point ) const
{
	Vec2 jBasis = GetJBasisNormal();
	Vec2 mins = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * jBasis);

	Vec2 dimensions = GetDimensions();
	float uvCoordsX = (point.x - mins.x) / dimensions.x;
	float uvCoordsY = (point.y - mins.y) / dimensions.y;

	return Vec2( uvCoordsX, uvCoordsY );
}

float OBB2::GetOuterRadius() const
{
	float outerRadius = m_halfDimensions.x + m_halfDimensions.y;
	return outerRadius;
}

float OBB2::GetInnerRadius() const
{
	if( m_halfDimensions.x > m_halfDimensions.y )
	{
		return m_halfDimensions.y;
	}
	else 
	{
		return m_halfDimensions.x;
	}
}

void OBB2::GetCornerPositions( Vec2* out_fourPoints ) const
{
	UNUSED (out_fourPoints);
}

std::vector<Vec2> OBB2::GetFourCornerPositions()
{
	std::vector<Vec2> positions; 
	Vec2 botLeft  = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * GetJBasisNormal() );
	Vec2 botRight = m_center + (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * GetJBasisNormal() );
	Vec2 topLeft  = m_center - (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * GetJBasisNormal() );
	Vec2 topRight = m_center + (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * GetJBasisNormal() );

	positions.push_back( botLeft );
	positions.push_back( botRight );
	positions.push_back( topLeft );
	positions.push_back( topRight );

	return positions;
}

void OBB2::Translate( const Vec2& translation )
{
	m_center += translation;
}

void OBB2::SetCenter( const Vec2& newCenter )
{
	m_center = newCenter;
}

void OBB2::SetDimensions( const Vec2& newDimensions )
{
	m_halfDimensions = 0.5f * newDimensions;
}

void OBB2::operator=( const OBB2& assignFrom )
{
	m_center = assignFrom.m_center;
	m_iBasis = assignFrom.m_iBasis;
	m_halfDimensions = assignFrom.m_halfDimensions;
}
