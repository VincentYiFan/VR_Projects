#include <math.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

STATIC const Vec2 Vec2:: ZERO( 0.f, 0.f );
STATIC const Vec2 Vec2:: ONE( 1.f, 1.f );



//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{

}
 

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

void Vec2::SetFromText( const char* text )
{
	Strings strings = SplitStringOnDelimiter( text, ',' ); 

	const char *firstValue  = strings[0].c_str();
	const char *secondValue = strings[1].c_str();

	x = (float)atof(firstValue);
	y = (float)atof(secondValue);
}

const Vec2 Vec2::MakeFromPolarRadians( float directionRadians, float length )
{
	return Vec2( length * (float)cos(directionRadians) , length * (float)sin(directionRadians) );
}

const Vec2 Vec2::MakeFromPolarDegrees( float directionDegrees, float length )
{
	return Vec2( length * CosDegrees(directionDegrees) , length * SinDegrees( directionDegrees ));
}

float Vec2::GetLength() const
{
	return sqrtf( ( x * x ) + ( y * y ) );
}

float Vec2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}

float Vec2::GetAngleRadians() const
{
	return (float)atan2( y , x );
}

float Vec2::GetAngleDegrees() const
{
	return Atan2Degrees( y , x );
}

const Vec2 Vec2::GetRotated90Degrees() const
{
	//return GetRotatedDegrees(90.f);
	return Vec2( -y, x );
}

const Vec2 Vec2::GetRotatedMinus90Degrees() const
{
	//return GetRotatedDegrees(-90.f);
	return Vec2( y, -x );
}

const Vec2 Vec2::GetRotatedRadians( float deltaRadians ) const
{
	float R = GetLength();
	float newRadians = GetAngleRadians() + deltaRadians;
	return Vec2( R * (float)cos(newRadians) , R * (float)sin(newRadians));
}

const Vec2 Vec2::GetRotatedDegrees( float deltaDegrees ) const
{
	float R = GetLength();
	float newDegrees = GetAngleDegrees() + deltaDegrees;
	return Vec2( R * CosDegrees( newDegrees ), R * SinDegrees( newDegrees ) );
}

const Vec2 Vec2::GetClamped( float maxLength ) const
{
	float R = GetLength();
	if( R > maxLength ) 
	{
		return TransformPosition2D( Vec2( x , y ) , maxLength / R , 0.f , Vec2( 0.f, 0.f ) );
	}
	else
	{
		return Vec2( x , y );
	}
}

const Vec2 Vec2::GetNormalized() const
{
	float Length = Vec2( x , y ).GetLength();
	if( Length != 0.f ) 
	{
		return ( Vec2( x , y ) / Length );
	}
	else
	{
		return Vec2( x , y );
	}
	//Add a solution 2 using trig()
}

const Vec2 Vec2::Cross( float z ) const
{
	return Vec2( -1.f * y * z, x * z );
}

void Vec2::SetAngleRadians( float newAngleRadians )
{
	const Vec2& a = MakeFromPolarRadians( newAngleRadians , GetLength());
	x = a.x;
	y = a.y;
}

void Vec2::SetAngleDegrees( float newAngleDegrees )
{
	const Vec2& a = MakeFromPolarDegrees( newAngleDegrees, GetLength() );
	x = a.x;
	y = a.y;
}

void Vec2::SetPolarRadians( float newAngleRadians, float newLength )
{
	x = newLength * (float)cos(newAngleRadians);
	y = newLength * (float)sin(newAngleRadians);
}

void Vec2::SetPolarDegrees( float newAngleDegrees, float newLength )
{
	x = newLength * CosDegrees( newAngleDegrees );
	y = newLength * SinDegrees( newAngleDegrees );
}

void Vec2::Rotate90Degrees()
{
	Vec2 a = GetRotated90Degrees();
	x = a.x;
	y = a.y;
	
	// Wrong example
	// x = GetRotated90Degrees().x;
	// y = GetRotated90Degrees().y;
}

void Vec2::RotateMinus90Degrees()
{
	Vec2 a = GetRotatedMinus90Degrees();
	x = a.x;
	y = a.y;
}

void Vec2::RotateRadians( float deltaRadians )
{
	Vec2 a = GetRotatedRadians( deltaRadians );
	x = a.x;
	y = a.y;
}

void Vec2::RotateDegrees( float deltaDegrees )
{
	Vec2 a = GetRotatedDegrees( deltaDegrees );
	x = a.x;
	y = a.y;
}

void Vec2::SetLength( float newLength )
{
	float theRatio = newLength / GetLength();
	x *= theRatio;
	y *= theRatio;
}

void Vec2::ClampLength( float maxLength )
{
	Vec2 clampedVec2 = Vec2( x , y ).GetClamped(maxLength);
	x = clampedVec2.x;
	y = clampedVec2.y;

}

void Vec2::Normalize()
{
	float Length = Vec2( x, y ).GetLength();
	if( Length != 0.f )
	{
		x /= Length;
		y /= Length;
	}
}

float Vec2::NormalizeAndGetPreviousLength()
{
	const float previousLength = GetLength();
	Normalize();
	return previousLength;
}

void Vec2::Reflect( const Vec2& normalVector )
{
	Vec2 velNormal = GetProjectedOnto2D( Vec2( x, y), normalVector.GetNormalized() );
	Vec2 velTangent = Vec2( x, y ) - velNormal;
	x = velTangent.x - velNormal.x;
	y = velTangent.y - velNormal.y;
}

Vec2 Vec2::GetReflected( const Vec2& normalVector ) const
{
	Vec2 velNormal = GetProjectedOnto2D( Vec2( x, y ), normalVector.GetNormalized() );
	Vec2 velTangent = Vec2( x, y ) - velNormal;
	return Vec2( velTangent.x - velNormal.x, velTangent.y - velNormal.y);
}



//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x , y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{	



	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const            
{
	return Vec2( -x, -y );
}



//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale , y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x / inverseScale, y / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x = x * uniformScale;
	y = y * uniformScale;
	 
}
 
//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;

}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( uniformScale * vecToScale.x, uniformScale * vecToScale.y );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	if(x == compare.x && y == compare.y) return true;
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	if( x != compare.x || y!= compare.y ) return true;
	return false;
}

