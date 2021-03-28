#include "IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
#include <Engine\Core\StringUtils.hpp>

const IntVec2 IntVec2::ZERO = IntVec2( 0, 0 );
const IntVec2 IntVec2::ONE  = IntVec2( 1, 1 );

IntVec2::IntVec2( const IntVec2& copyFrom )
	:x(copyFrom.x),
	y(copyFrom.y)
{

}

IntVec2::IntVec2( int initialX, int initialY )
	:x(initialX)
	,y(initialY)
{

}

void IntVec2::SetFromText( const char* text )
{
	Strings strings = SplitStringOnDelimiter( text, ',' );

	const char* firstValue  = strings[0].c_str();
	const char* secondValue = strings[1].c_str();

	x = atoi( firstValue );
	y = atoi( secondValue );
}

float IntVec2::GetLength() const
{
	float result =  (float)sqrtf( ( (float)x * (float)x ) + ( (float)y * (float)y ) );
	return result;
}

int IntVec2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

float IntVec2::GetOrientationRadians() const
{
	return (float)atan2( y , x );
}

float IntVec2::GetOrientationDegrees() const
{
	return static_cast<float>( Atan2Degrees( (float)y , (float)x ) );
}

const IntVec2 IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y , x );
}

const IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	
	return IntVec2( y , -x );
}

void IntVec2::Rotate90Degrees()
{
	int temp = x; 
	x = -y;
	y = temp;
}

void IntVec2::RotateMinus90Degrees()
{
	int temp = x;
	x = y;
	y = -temp;
}

bool IntVec2::operator==( const IntVec2& compare ) const
{
	if( x == compare.x && y == compare.y ) 
	{
		return true;
	}
	else 
	{
		return false;
	}
}

bool IntVec2::operator!=( const IntVec2& compare ) const
{
	if( x != compare.x || y!= compare.y ) 
	{
		return true;
	}
	else{ return false; }
}

const IntVec2 IntVec2::operator+( const IntVec2& vecToAdd ) const
{
	return IntVec2( x + vecToAdd.x, y + vecToAdd.y );
}

const IntVec2 IntVec2::operator-( const IntVec2& vecToSubtract ) const
{
	return IntVec2( x - vecToSubtract.x, y - vecToSubtract.y );
}

const IntVec2 IntVec2::operator-() const
{
	return IntVec2( -x, -y );
}

const IntVec2 IntVec2::operator*( int uniformScale ) const
{
	return IntVec2( x * uniformScale, y * uniformScale );
}

const IntVec2 IntVec2::operator*( const IntVec2& vecToMultiply ) const
{
	return IntVec2( x * vecToMultiply.x,  y * vecToMultiply.y);
}

void IntVec2::operator+=( const IntVec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}

void IntVec2::operator-=( const IntVec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}

void IntVec2::operator*=( const int uniformScale )
{
	x *=  uniformScale;
	y *=  uniformScale;
}

void IntVec2::operator=( const IntVec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale )
{
	return vecToScale * uniformScale;
}
