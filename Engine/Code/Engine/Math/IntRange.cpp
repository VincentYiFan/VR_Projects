#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <string>

//---------------------------------------------------------------------------

IntRange::IntRange( int minAndMax )
{
	minimum = minAndMax;
	maximum = minAndMax;
}

IntRange::IntRange( int min, int max )
{
	if( min > max )
	{
		ERROR_AND_DIE( "FloatRange Initializer failure! min is bigger than max." );
	}

	minimum = min;
	maximum = max;
}

IntRange::IntRange( const char* asText )
{
	Strings floatRange = SplitStringOnDelimiter( asText, '~' );
	int numFloats = (int)floatRange.size();
	if( numFloats > 2 )
	{
		ERROR_AND_DIE( "IntRange Initializer failure!" );
	}

	minimum = std::stoi( floatRange[0] );
	maximum = std::stoi( floatRange[1] );
}

bool IntRange::IsInRange( int value ) const
{
	if( value >= minimum && value <= maximum )
	{
		return true;
	}

	return false;
}

bool IntRange::DoesOverlap( const IntRange& otherRange ) const
{
	if( minimum > otherRange.maximum || maximum < otherRange.minimum )
	{
		return false;
	}

	return true;
}

std::string IntRange::GetAsString() const
{
	std::string newString = Stringf( "%s~%s", std::to_string( minimum ).c_str(), std::to_string( maximum ).c_str() );
	return newString;
}

int IntRange::GetRandomInRange( RandomNumberGenerator& rng ) const
{
	int randomNumberInRange = rng.RollRandomIntInRange( minimum, maximum );
	return randomNumberInRange;
}

void IntRange::Set( int newMinimum, int newMaximum )
{
	minimum = newMinimum;
	maximum = newMaximum;
}

bool IntRange::SetFromText( const char* asText )
{
	std::string text( asText );
	Strings floatRange = SplitStringOnDelimiter( text, '~' );
	int numFloats = (int)floatRange.size();
	if( numFloats > 2 )
	{
		ERROR_AND_DIE( "FloatRange Initializer failure!" );
	}

	minimum = std::stoi( floatRange[0] );
	maximum = std::stoi( floatRange[1] );

	return false;
}
