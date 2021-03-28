#pragma once
#include <vector>
constexpr float PI = 3.1415926535897932384626433832795f;

//Forward type declarations
struct Vec2;
struct Vec3;
struct IntVec2; 
struct Vertex_PCU;
struct AABB2;
struct AABB3;
struct OBB2;

struct Quaternion
{
	double w, x, y, z;
	Quaternion( double initialW, double initialX, double initialY, double initialZ ) 
		: w(initialW), x(initialX), y(initialY), z(initialZ) {}
};

// Lerp and Clamp
float	Interpolate( float a, float b, float fractionOfB ); // return weighted average of a & b given b's weight
Vec3	Interpolate( const Vec3& a, const Vec3& b, float t );
float	RangeMap( float inputRangeBegin, float inputRangeEnd, float outputRangeBegin, float outputRangeEnd, float inputValue );
float	Clamp( float value, float min, float max); //note: max may not be less than min
int		Clamp( int value, int min, int max ); //note: max may not be less than min
float	ClampZeroToOne( float value );
float	Round( float value );					// round to nearest whole float; x.5 rounds up to x+1
int		RoundDownToInt( float value );
int		RoundToNearestInt( float value );

//Angle utilities
float	ConvertDegreesToRadians( float degrees );
float	ConvertRadiansToDegrees( float radians );
float	CosDegrees( float degrees );
float	SinDegrees( float degrees );
float	Atan2Degrees ( float y, float x );
float	GetShortestAngularDisplacement ( float fromDegrees, float toDegrees);
float	GetTurnedToward( float currentDegrees, float goalegrees, float maxDeltaDegrees );
float	GetAngleBetweenVec3( const Vec3& a, const Vec3& b );

//Basic 2D & 3D utilities
float	GetDistance2D( const Vec2& positionA, const Vec2& positionB );
float	GetDistanceSquared2D( const Vec2& positionA, const Vec2& positionB );
float	GetDistance3D( const Vec3& positionA, const Vec3& positionB );
float	GetDistanceSquared3D( const Vec3& positionA, const Vec3& positionB);
float	GetDistanceXY3D( const Vec3& positionA, const Vec3& positionB );
float	GetDistanceXYSquared3D( const Vec3& positionA, const Vec3& positionB );
int		GetTaxicabDistance2D( const IntVec2& positionA, const IntVec2& positionB );

// Dot product
float	DotProduct2D( const Vec2& a, const Vec2& b );
float	DotProduct( const Vec3& a, const Vec3& b );
float	perpDot( Vec2 A, Vec2 B );

// Cross product
float	CrossProductLength( float Ax, float Ay, float Bx, float By, float Cx, float Cy );
float	CrossProduct2D( Vec2 A, Vec2 B );
Vec3	CrossProduct( Vec3 A, Vec3 B );
Vec3	GetTriNormal( Vec3 t1, Vec3 t2, Vec3 t3 );	// Generate a cross product normal for a triangle

float	SignFloat( float value );
float	GetSmallerValueBetweenTwoFloats( float a, float b );

Vec3	ToEulerAngles( Quaternion quaternion );

//----------------------------------------------------------------------------------
//Geometric query utilities
//----------------------------------------------------------------------------------
bool		DoDiscsOverlap( const Vec2& CenterA, float radiusA, const Vec2& CenterB, float radiusB);
bool		DoSpheresOverlap3D(  const Vec3& CenterA, float radiusA, const Vec3& CenterB, float radiusB);
bool		DoAABB3sOverlap3D( const AABB3& a, const AABB3& b );

void		PushDiscOutOfPoint2D       ( Vec2& centerPosition, float radius, const Vec2& PositionB );
void		PushDiscOutOfDisc2D	    ( Vec2& PositionA, float radiusA,  const Vec2& PositionB, float radiusB );
void		PushDiscsOutOfEachOther2D	( Vec2& PositionA, float radiusA, Vec2& PositionB, float radiusB );
void		PushDiscOutOfAABB2D		(  Vec2& centerPosition, float radius, const AABB2 aabb2 );
float		GetProjectedLength2D( const Vec2& sourceVector, const Vec2& ontoVector );
float		GetProjectedLength3D( const Vec3& sourceVector, const Vec3& ontoVector );
const Vec2  GetProjectedOnto2D( const Vec2& sourceVector, const Vec2& ontoVector ); 
const Vec3	GetProjectedOnto3D( const Vec3& sourceVector, const Vec3& ontoVector );
float		GetAngleDegreesBetweenVectors2D( const Vec2& vectorA, const Vec2& vectorB );

bool	    IsPointInsideDisc2D( const Vec2& point, const Vec2& discCenter, float discRadius );
bool		IsPointInsideAABB2D( const Vec2& point, const AABB2& box );
bool		IsPointInsideCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
bool		IsPointInsideOBB2D( const Vec2& point, const OBB2& box );
bool		IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );
bool		IsPointInTriangle( Vec3 point, Vec3 tri1, Vec3 tri2, Vec3 tri3 );

// test to see if P1 is on the same side as P2 of a line segment ab
bool		IsSameSide( Vec3 p1, Vec3 p2, Vec3 a, Vec3 b );

const Vec2  GetNearestPointOnDisc2D( const Vec2& PositionA, const Vec2& centerPosition, float radius );
const Vec2	GetNearestPointOnAABB2D( const Vec2& PositionA, const AABB2& aabb2 );
const Vec2  GetNearestPointOnInfiniteLine2D( const Vec2& refPos, const Vec2& somePointOnLine, const Vec2& anotherPointOnLine );
const Vec2  GetNearestPointOnLineSegment2D( const Vec2& refPos, const Vec2& start, const Vec2& end );
const Vec2  GetNearestPointOnCapsule2D( const Vec2& refPos, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
const Vec2	GetNearestPointOnOBB2D( const Vec2& refPos, const OBB2& box );

//Transform utilities
const Vec2 TransformPosition2D( const Vec2& position, float uniformScale, float rotationDegrees, const Vec2& translation );
const Vec2 TransformPosition2D( const Vec2& position, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation );
const Vec3 TransformPosition3DXY( const Vec3& position, float ScaleXY, float zRotationDegrees, const Vec2& translationXY);
const Vec3 TransformPosition3DXY( const Vec3& position, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translationXY );
const void TransfromVertex( Vertex_PCU& vertex, float uniformScale, float rotationDegrees, const Vec2& translation );
const void TransfromVertexesArray(const int& numVertexes ,Vertex_PCU* vertexesArray, float uniformScale, float rotationDegrees, const Vec2& translation );