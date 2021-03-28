#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <cmath>
#include <math.h>


float ConvertDegreesToRadians( float degrees )
{
	return ( degrees / 180.f ) * PI;
}

float ConvertRadiansToDegrees( float radians )
{
	return ( radians / PI ) * 180.f;
}

float CosDegrees( float degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}

float SinDegrees( float degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}

float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y , x ) );
}

float GetShortestAngularDisplacement( float fromDegrees, float toDegrees )
{
	float dif = toDegrees - fromDegrees;

	while( dif < -180.f )
	{
		dif += 360.f;
	}
	while( dif > 180.f )
	{
		dif -= 360.f;
	}
	return dif;
}

float GetTurnedToward( float currentDegrees, float goalDegrees, float maxDeltaDegrees )
{
	float angularDisp = GetShortestAngularDisplacement( currentDegrees, goalDegrees );
	if( fabsf(angularDisp) < fabsf(maxDeltaDegrees) )
	{
		return goalDegrees;
	}

	if( angularDisp < 0.f )
	{
		currentDegrees -= maxDeltaDegrees;
		return currentDegrees;
	}
	else
	{
		currentDegrees += maxDeltaDegrees;
		return currentDegrees;
	}
}

float GetAngleBetweenVec3( const Vec3& a, const Vec3& b )
{
	float angle = DotProduct( a, b );
	angle /= ( a.GetLength() * b.GetLength() );
	return angle = acosf( angle );
}

float GetDistance2D( const Vec2& positionA, const Vec2& positionB )
{
	const Vec2& A = positionA, B = positionB;
	return sqrtf( (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y) );
}

float GetDistanceSquared2D( const Vec2& positionA, const Vec2& positionB )
{
	const Vec2& A = positionA, B = positionB;
	return (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y);
}

float GetDistance3D( const Vec3& positionA, const Vec3& positionB )
{
	const Vec3& A = positionA, B = positionB;
	return sqrtf( (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y) + (A.z - B.z)*(A.z - B.z) );
}

float GetDistanceSquared3D( const Vec3& positionA, const Vec3& positionB )
{
	const Vec3& A = positionA, B = positionB;
	return (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y) + (A.z - B.z)*(A.z - B.z);
}

float GetDistanceXY3D( const Vec3& positionA, const Vec3& positionB )
{
	const Vec3& A = positionA, B = positionB;
	return sqrtf( (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y) );
}

float GetDistanceXYSquared3D( const Vec3& positionA, const Vec3& positionB )
{
	const Vec3& A = positionA, B = positionB;
	return ( (A.x - B.x)*(A.x - B.x) +  (A.y - B.y)*(A.y - B.y) );
}

int GetTaxicabDistance2D( const IntVec2& positionA, const IntVec2& positionB )
{
	return abs(positionA.x - positionB.x) + abs(positionA.y - positionB.y);
}

float DotProduct2D( const Vec2& a, const Vec2& b )
{
	return (a.x*b.x) + (a.y*b.y);
}
float DotProduct( const Vec3& a, const Vec3& b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}
float CrossProductLength( float Ax, float Ay, float Bx, float By, float Cx, float Cy )
{
	// Get the vectors' coordinates.
	float BAx = Ax - Bx;
	float BAy = Ay - By;
	float BCx = Cx - Bx;
	float BCy = Cy - By;

	// Calculate the Z coordinate of the cross product.
	return (BAx * BCy - BAy * BCx);
}

float CrossProduct2D( Vec2 A, Vec2 B )
{
	float result = (A.x * B.y) - (A.y * B.x);
	return result;
}

float perpDot( Vec2 A, Vec2 B )
{
	return (A.x * B.y) - (A.y * B.x);
}

Vec3 CrossProduct( Vec3 A, Vec3 B )
{
	Vec3 C;
	C.x = A.y * B.z - A.z * B.y;
	C.y = -( A.x * B.z - A.z * B.x );
	C.z = A.x * B.y - A.y * B.x;

	return C;
}

Vec3 GetTriNormal( Vec3 t1, Vec3 t2, Vec3 t3 )
{
	Vec3 u = t2 - t1;
	Vec3 v = t3 - t1;

	Vec3 normal = CrossProduct( u, v );
	return normal;
}

float SignFloat( float value )
{
	return ( value >= 0.f ) ? 1.f : -1.f;
}

float GetSmallerValueBetweenTwoFloats( float a, float b )
{
	return ( a > b ) ? a : b;
}

Vec3 ToEulerAngles( Quaternion quaternion )
{
	//////////////////////////////////////////////////////////////////////////
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	//////////////////////////////////////////////////////////////////////////
	Vec3 eulerAngles;

	Quaternion& q1 = quaternion;
	//// roll (x-axis rotation)
	//double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	//double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	//eulerAngles.x = (float) atan2(sinr_cosp, cosr_cosp);

	//// pitch( y-axis rotation )
	//double sinp = 2 * (q.w * q.y - q.z * q.x);
	//if( std::abs( sinp ) >= 1 )
	//	eulerAngles.y = (float) std::copysign( PI / 2, sinp ); // use 90 degrees if out of range
	//else
	//	eulerAngles.y = (float) std::asin( sinp );

	//// yaw (z-axis rotation)
	//double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	//double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	//eulerAngles.z = (float) std::atan2( siny_cosp, cosy_cosp );

	double heading, attitude, bank;
	double test = q1.x*q1.y + q1.z*q1.w;
	if( test > 0.499 ) { // singularity at north pole
		heading = 2 * atan2( q1.x, q1.w );
		attitude = PI/2;
		bank = 0;
		return Vec3((float)heading, (float)attitude, (float)bank);
	}
	if( test < -0.499 ) { // singularity at south pole
		heading = -2 * atan2( q1.x, q1.w );
		attitude = -PI/2;
		bank = 0;
		return Vec3((float)heading, (float)attitude, (float)bank);
	}
	double sqx = q1.x*q1.x;
	double sqy = q1.y*q1.y;
	double sqz = q1.z*q1.z;
	heading = atan2( 2*q1.y*q1.w-2*q1.x*q1.z, 1 - 2*sqy - 2*sqz );
	attitude = asin( 2*test );
	bank = atan2( 2*q1.x*q1.w-2*q1.y*q1.z, 1 - 2*sqx - 2*sqz );

	return Vec3((float)heading, (float)attitude, (float)bank);
	//return eulerAngles;
}

bool IsPointInsideDisc2D( const Vec2& point, const Vec2& discCenter, float discRadius )
{
	return ( GetDistanceSquared2D( point, discCenter ) < discRadius*discRadius );
}

bool IsPointInsideAABB2D( const Vec2& point, const AABB2& box )
{
	return ( ( point.x > box.mins.x && point.x < box.maxs.x ) &&  ( point.y > box.mins.y && point.y < box.maxs.y ) );
}

bool IsPointInsideCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	Vec2 nearestPointOnLineSegment = GetNearestPointOnLineSegment2D( point, capsuleMidStart, capsuleMidEnd );
	return ( GetDistanceSquared2D( point, nearestPointOnLineSegment ) <= (capsuleRadius * capsuleRadius) );
}

bool IsPointInsideOBB2D( const Vec2& point, const OBB2& box )
{
	Vec2 dispFromCenterToPoint = point - box.m_center;
	Vec2 jBasis = Vec2( -box.m_iBasis.y, box.m_iBasis.x );
	float projectionOntoIBasis = DotProduct2D( dispFromCenterToPoint, box.m_iBasis );
	float projectionOntoJBasis = DotProduct2D( dispFromCenterToPoint, jBasis );

	return ( ( fabsf( projectionOntoIBasis ) <= box.m_halfDimensions.x ) && ( fabsf( projectionOntoJBasis ) <= box.m_halfDimensions.y ) );
}

bool IsSameSide( Vec3 p1, Vec3 p2, Vec3 a, Vec3 b )
{
	Vec3 cp1 = CrossProduct( b - a, p1 - a );
	Vec3 cp2 = CrossProduct( b - a, p2 - a );

	if( DotProduct( cp1, cp2 ) >= 0.f ) {
		return true;
	}
	else {
		return false;
	}
}

const Vec2 GetNearestPointOnDisc2D( const Vec2& referencePos, const Vec2& centerPosition, float radius )
{
	if( IsPointInsideDisc2D( referencePos, centerPosition, radius ) )
		return referencePos;
	else
	{
		Vec2 displacementCenterToPoint = referencePos - centerPosition;
		Vec2 displacementFromCenter = displacementCenterToPoint.GetNormalized() * radius;
		Vec2 nearestPointOnDisc2D = centerPosition + displacementFromCenter;
		return nearestPointOnDisc2D;
	}
}


float GetAngleDegreesBetweenVectors2D( const Vec2& vectorA, const Vec2& vectorB )
{
	return fabsf(  Atan2Degrees( vectorA.y, vectorA.x ) -  Atan2Degrees( vectorB.y, vectorB.x ) );
}

bool IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	if( GetDistance2D( point, observerPos ) > maxDist ) 
		return false;
	
	Vec2 dispToPoint = point - observerPos;
	float dispAngleDegrees = dispToPoint.GetAngleDegrees();
	float shortestAngle = fabsf( GetShortestAngularDisplacement( dispAngleDegrees, forwardDegrees ) );
	return ( shortestAngle <= ( apertureDegrees * 0.5f ) ); 
}

bool IsPointInTriangle( Vec3 point, Vec3 tri1, Vec3 tri2, Vec3 tri3 )
{
	// Test to see if it is within an infinite prism that the triangle outlines.
		bool within_tri_prisim = IsSameSide( point, tri1, tri2, tri3 ) && IsSameSide( point, tri2, tri1, tri3 )
		&& IsSameSide( point, tri3, tri1, tri2 );

	// If it isn't it will never be on the triangle
	if( !within_tri_prisim )
		return false;

	// Calculate Triangle's Normal
	Vec3 n = GetTriNormal( tri1, tri2, tri3 );

	// Project the point onto this normal
	Vec3 proj = GetProjectedOnto3D( point, n );

	// If the distance from the triangle to the point is 0
	//	it lies on the triangle
	if( proj.GetLengthSquared() == 0 )	// use squared for performance
		return true;
	else
		return false;
}

const Vec2 GetNearestPointOnAABB2D( const Vec2& PositionA, const AABB2& aabb2 )
{
	Vec2 nearestPointOnAABB2D = Vec2::ZERO;
	// X Axis
	if( PositionA.x < aabb2.mins.x ) 
	{
		nearestPointOnAABB2D.x = aabb2.mins.x;
	}
	else if( PositionA.x > aabb2.maxs.x )
	{
		nearestPointOnAABB2D.x = aabb2.maxs.x;
	}
	else 
	{
		nearestPointOnAABB2D.x = PositionA.x;
	}

	// Y Axis
	if( PositionA.y < aabb2.mins.y )
	{
		nearestPointOnAABB2D.y = aabb2.mins.y;
	}
	else if( PositionA.y > aabb2.maxs.y )
	{
		nearestPointOnAABB2D.y = aabb2.maxs.y;
	}
	else
	{
		nearestPointOnAABB2D.y = PositionA.y;
	}

	return nearestPointOnAABB2D;
}

const Vec2 GetNearestPointOnInfiniteLine2D( const Vec2& refPos, const Vec2& somePointOnLine, const Vec2& anotherPointOnLine )
{
	Vec2 forwardVector = anotherPointOnLine - somePointOnLine;
	Vec2 dispSomePointToRefPos = refPos - somePointOnLine;
	return somePointOnLine + GetProjectedOnto2D( dispSomePointToRefPos, forwardVector.GetNormalized() );
}

const Vec2 GetNearestPointOnLineSegment2D( const Vec2& refPos, const Vec2& start, const Vec2& end )
{
	Vec2 startToPointDisp = refPos - start;
	Vec2 endToPointDisp = refPos - end;
	Vec2 forwardDir = end - start;

	if( DotProduct2D( startToPointDisp, forwardDir ) < 0.f )
	{
		return start;
	}
	else if( DotProduct2D( endToPointDisp, forwardDir ) > 0.f )
	{
		return end;
	}
	else
	{
		return start + GetProjectedOnto2D( startToPointDisp, forwardDir.GetNormalized() );
	}
}

const Vec2 GetNearestPointOnCapsule2D( const Vec2& refPos, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	Capsule2 capsule2 = Capsule2( capsuleMidStart, capsuleMidEnd, capsuleRadius );
	if( capsule2.IsPointInside( refPos ) )
		return refPos;

	Vec2 startToPointDisp = refPos - capsuleMidStart;
	Vec2 endToPointDisp = refPos - capsuleMidStart;
	Vec2 forwardDir = capsuleMidEnd - capsuleMidStart;

	if( DotProduct2D( startToPointDisp, forwardDir ) < 0.f )
	{
		return capsuleMidStart + startToPointDisp.GetNormalized() * capsuleRadius;
	}
	else if( DotProduct2D( endToPointDisp, forwardDir ) > 0.f )
	{
		return capsuleMidEnd + endToPointDisp.GetNormalized() * capsuleRadius;
	}
	else
	{
		Vec2 projectionOntoSEVector = GetProjectedOnto2D( startToPointDisp, forwardDir.GetNormalized() );
		Vec2 nearestPointOnBone =  capsuleMidStart + projectionOntoSEVector;
		Vec2 nearestPointToStartDisp = capsuleMidStart - nearestPointOnBone;
		return nearestPointOnBone + (-projectionOntoSEVector + startToPointDisp).GetNormalized() * capsuleRadius;
	}
}

const Vec2 GetNearestPointOnOBB2D( const Vec2& refPos, const OBB2& box )
{
	if( box.IsPointInside( refPos ) )
	{
		return refPos;
	}

	Vec2 dispFromCenterToPoint = refPos - box.m_center;
	Vec2 jBasis = box.GetJBasisNormal();
	float projectionOntoIBasis = DotProduct2D( dispFromCenterToPoint, box.m_iBasis );
	float projectionOntoJBasis = DotProduct2D( dispFromCenterToPoint, jBasis );

	float nearestPointDirectionI = Clamp( projectionOntoIBasis, -box.m_halfDimensions.x, box.m_halfDimensions.x );
	float nearestPointDirectionJ = Clamp( projectionOntoJBasis, -box.m_halfDimensions.y, box.m_halfDimensions.y );

	Vec2 nearestPoint = box.m_center + (nearestPointDirectionI * box.m_iBasis) + (nearestPointDirectionJ * jBasis);
	return nearestPoint;
}

void PushDiscOutOfDisc2D( Vec2& PositionA, float radiusA, const Vec2& PositionB, float radiusB )
{
	if( GetDistance2D( PositionA, PositionB ) > radiusA + radiusB ) return;

	Vec2 displacementAB = PositionA - PositionB;
	Vec2 displacementABNormalized = displacementAB.GetNormalized();
	float overlapDistance = radiusA + radiusB - GetDistance2D( PositionA, PositionB );
	PositionA += displacementABNormalized * overlapDistance;
}

void PushDiscsOutOfEachOther2D( Vec2& PositionA, float radiusA, Vec2& PositionB, float radiusB )
{
	if( GetDistance2D( PositionA, PositionB ) > radiusA + radiusB ) return;

	Vec2 displacementAB = PositionA - PositionB;
	Vec2 displacementABNormalized = displacementAB.GetNormalized();
	float overlapDistance = radiusA + radiusB - GetDistance2D( PositionA, PositionB );
	Vec2 CorrectionA = displacementABNormalized * overlapDistance * 0.5f;
	Vec2 CorrectionB = -CorrectionA;
	PositionA += CorrectionA;
	PositionB += CorrectionB;
}

void PushDiscOutOfPoint2D( Vec2& centerPosition, float radius, const Vec2& PositionB )
{
	if( GetDistance2D( centerPosition, PositionB) > radius ) return;

	Vec2 DisplacementBC = centerPosition - PositionB;
	centerPosition = PositionB + DisplacementBC.GetNormalized() * radius;
}

void PushDiscOutOfAABB2D( Vec2& centerPosition, float radius, const AABB2 aabb2 )
{
	//Vec2 nearestPointOnAABB2 = GetNearestPointOnAABB2D( centerPosition, aabb2 );
	Vec2 nearestPointOnAABB2 = aabb2.GetNearestPoint( centerPosition ); // This line is incorrect!
	Vec2 displacementNearestPointToCenter = centerPosition - nearestPointOnAABB2; 
	if( displacementNearestPointToCenter.GetLength() < radius )
	{
		Vec2 moveDisplacement =  displacementNearestPointToCenter.GetNormalized() * radius;
		centerPosition = nearestPointOnAABB2 + moveDisplacement;
	}
}

const Vec2 GetProjectedOnto2D( const Vec2& sourceVector, const Vec2& ontoVector )
{
	Vec2 normalizedOntoVector = ontoVector.GetNormalized();
	return DotProduct2D( normalizedOntoVector, sourceVector ) * normalizedOntoVector;

	// Fast way
	// float d = DotProduct2D( sourceVector, ontoVector );
	// float ontoLength = ontoVector.GetLegnthSquared();
	// return ( d * ontoVector ) / ontoLength;
}

const Vec3 GetProjectedOnto3D( const Vec3& sourceVector, const Vec3& ontoVector )
{
	Vec3 bn = ontoVector.GetNormalized();
	return bn * DotProduct( sourceVector, bn );
}

float GetProjectedLength2D( const Vec2& sourceVector, const Vec2& ontoVector )
{
	float dotProduct = DotProduct2D( ontoVector, sourceVector );
	return dotProduct / ontoVector.GetLength();
}

float GetProjectedLength3D( const Vec3& sourceVector, const Vec3& ontoVector )
{
	float dotProduct = DotProduct( sourceVector, ontoVector );
	return dotProduct / ontoVector.GetLength();
}

bool DoDiscsOverlap( const Vec2& CenterA, float radiusA, const Vec2& CenterB, float radiusB )
{
	return ( GetDistanceSquared2D(CenterA , CenterB) < (radiusA + radiusB)*(radiusA + radiusB) );
}

bool DoSpheresOverlap3D( const Vec3& CenterA, float radiusA, const Vec3& CenterB, float radiusB )
{
	return( GetDistanceSquared3D( CenterA, CenterB ) < (radiusA + radiusB)*(radiusA + radiusB) );
}

bool DoAABB3sOverlap3D( const AABB3& a, const AABB3& b )
{
	if( a.mins.x >= b.maxs.x ) return false;
	if( a.maxs.x <= b.mins.x ) return false;
	if( a.mins.y >= b.maxs.y ) return false;
	if( a.maxs.y <= b.mins.y ) return false;
	if( a.mins.z >= b.maxs.z ) return false;
	if( a.maxs.z <= b.mins.z ) return false;

	return true;
}


const Vec2 TransformPosition2D( const Vec2& position, float uniformScale, float rotationDegrees, const Vec2& translation )
{
	Vec2 s = position * uniformScale;
	Vec2 r = s.GetRotatedDegrees(rotationDegrees);
	return r + translation;
}

const Vec2 TransformPosition2D( const Vec2& position, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation )
{
	float newX = position.x * iBasis.x + position.y * jBasis.x + translation.x;
	float newY = position.x * iBasis.y + position.y * jBasis.y + translation.y;
	return Vec2( newX, newY );
}

const Vec3 TransformPosition3DXY( const Vec3& position, float ScaleXY, float zRotationDegrees, const Vec2& translationXY )
{
	Vec3 s = Vec3( position.x * ScaleXY , position.y * ScaleXY , position.z );
	Vec3 r = s.GetRotatedAboutZDegrees( zRotationDegrees );
	return r + Vec3( translationXY.x , translationXY.y , 0.f );
}

const Vec3 TransformPosition3DXY( const Vec3& position, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translationXY )
{
	Vec2 vector2D = TransformPosition2D( Vec2( position.x, position.y), iBasis, jBasis, translationXY );
	return Vec3( vector2D.x, vector2D.y, position.z );

}

const void TransfromVertex( Vertex_PCU& vertex, float uniformScale, float rotationDegrees, const Vec2& translation )
{
	Vec3 newPosition = TransformPosition3DXY( vertex.m_position, uniformScale, rotationDegrees, translation );
	vertex.m_position.x = newPosition.x;
	vertex.m_position.y = newPosition.y;

	/*const Vec2& vertexPosition = Vec2( vertex.m_position.x, vertex.m_position.y );
	const Vec2& newPosition = TransformPosition2D( vertexPosition, uniformScale, rotationDegrees, translation );
	vertex.m_position = Vec3( newPosition.x, newPosition.y, vertex.m_position.z );*/
}

const void TransfromVertexesArray( const int& numVertexes, Vertex_PCU* vertexesArray, float uniformScale, float rotationDegrees, const Vec2& translation )
{
	for( int index = 0; index < numVertexes; index++ ) 
	{
		TransfromVertex( vertexesArray[index], uniformScale, rotationDegrees, translation );
	}
}



float Interpolate( float a, float b, float fractionOfB )
{
	return a + fractionOfB*(b-a);
}

Vec3 Interpolate( const Vec3& a, const Vec3& b, float t )
{
	return a + ( b - a ) * t;
}

float RangeMap( float inputRangeBegin, float inputRangeEnd, float outputRangeBegin, float outputRangeEnd, float inputValue )
{
	float inDisplacement = inputValue - inputRangeBegin; 
	float InRange = inputRangeEnd - inputRangeBegin;
	float fraction = inDisplacement / InRange;
	float outRange = outputRangeEnd - outputRangeBegin;
	float outDisplacement = fraction * outRange;
	float outValue = outputRangeBegin + outDisplacement;
	return outValue;
}

float Clamp( float value, float min, float max )
{
	if( value > max ) 
	{
		return max;
	}
	else if( value < min )
	{
		return min;
	}
	return value;
}

int Clamp( int value, int min, int max )
{
	if( value > max )
	{
		return max;
	}
	else if( value < min )
	{
		return min;
	}
	return value;
}

float ClampZeroToOne( float value )
{
	return Clamp( value, 0.f, 1.f);
}

float Round( float value )
{
	return (float)(int)value;
}

int RoundDownToInt( float value )
{
	return (int)floor( value );
}

int RoundToNearestInt( float value )
{
	return ( value>(int)value?(int)value+1:(int)value );
}
