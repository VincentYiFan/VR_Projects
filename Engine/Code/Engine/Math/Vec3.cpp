#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>
//#include "Engine/Core/EngineCommon.hpp"

const Vec3 Vec3:: ZERO( 0.f, 0.f, 0.f );

//-----------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec3& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
{
}

Vec3::Vec3( const Vec2& copyFrom, float z )
	:x( copyFrom.x )
	,y( copyFrom.y )
	,z( z )
{
}
 

//-----------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY, float initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}

void Vec3::SetFromText( const char* text )
{
	Strings strings = SplitStringOnDelimiter( text, ',' );

	const char* firstValue  = strings[0].c_str();
	const char* secondValue = strings[1].c_str();
	const char* thirdValue  = strings[2].c_str();

	x = (float)atof( firstValue );
	y = (float)atof( secondValue );
	z = (float)atof( thirdValue );
}

float Vec3::GetLength() const
{
	return sqrtf( ( x * x ) + ( y * y ) + ( z * z ) );
}

float Vec3::GetLengthXY() const
{
	return sqrtf( ( x * x ) + ( y * y ) );
}

float Vec3::GetLengthSquared() const
{
	return ( ( x * x ) + ( y * y ) + ( z * z ) );
}

float Vec3::GetLengthXYSquared() const
{
	return ( ( x * x ) + ( y * y ) );
}

float Vec3::GetAngleAboutZRadians() const
{
	return (float)atan2( y , x );
}

float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees( y , x );
}

const Vec3 Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	float R = GetLengthXY();
	float newRadians = GetAngleAboutZRadians() + deltaRadians;
	return Vec3( R * (float)cos(newRadians) , R * (float)sin(newRadians) , z );
}

const Vec3 Vec3::GetRotatedAboutZDegrees( float deltaDegrees ) const
{
	float R = GetLengthXY();
	float newDegrees = GetAngleAboutZDegrees() + deltaDegrees;
	return Vec3( R * CosDegrees( newDegrees ), R * SinDegrees( newDegrees ), z );
}

//const Vec3 Vec3::GetClamped( float maxLength ) const
//{
//	return Vec3();
//}

const Vec3 Vec3::GetNormalized() const
{
	float Length = Vec3( x, y, z ).GetLength();
	if( Length != 0.f )
	{
		return (Vec3( x, y, z ) / Length );
	}
	else
	{
		return Vec3( x, y, z );
	}
}

Vec3 Vec3::Cross( const Vec3& b )
{
	return Vec3(
		y * b.z - z * b.y,
		z * b.x - x * b.z,
		x * b.y - y * b.x );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + ( const Vec3& vecToAdd ) const
{
	return Vec3( x + vecToAdd.x , y + vecToAdd.y, z + vecToAdd.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const            
{
	return Vec3( -x, -y, -z );
}



//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const
{
	return Vec3( x * uniformScale , y * uniformScale, z * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const
{
	return Vec3( x / inverseScale, y / inverseScale, z / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=( const float uniformScale )
{
	x = x * uniformScale;
	y = y * uniformScale;
	z = z * uniformScale;
	{

	};
	 
}
 
//-----------------------------------------------------------------------------------------------
void Vec3::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z  =copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToScale )
{
	return Vec3( uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z );
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const
{
	if(x == compare.x && y == compare.y && z == compare.z) return true;
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const
{
	if( x != compare.x || y!= compare.y || z!= compare.z ) return true;
	return false;
}

