#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <string>

FloatRange::FloatRange( float minAndMax )
{
	minimum = minAndMax;
	maximum = minAndMax;
}

FloatRange::FloatRange( float min, float max )
{
	if( min > max )
	{
		ERROR_AND_DIE( "FloatRange Initializer failure! min is bigger than max." );
	}

	minimum = min;
	maximum = max;
}

FloatRange::FloatRange( const char* asText )
{
	Strings floatRange = SplitStringOnDelimiter( asText, '~' );
	int numFloats = (int) floatRange.size();
	if( numFloats > 2 )
	{
		ERROR_AND_DIE( "FloatRange Initializer failure!" );
	}
	//g_theConsole->PrintString( Rgba8::RED, floatRange[0] );
	//g_theConsole->PrintString( Rgba8::RED, floatRange[1] );

	minimum = std::stof( floatRange[0] );
	maximum = std::stof( floatRange[1] );
}

bool FloatRange::IsInRange( float value ) const
{
	if( value >= minimum && value <= maximum )
	{
		return true;
	}
	
	return false;
}

bool FloatRange::DoesOverlap( const FloatRange& otherRange ) const
{
	if( minimum > otherRange.maximum || maximum < otherRange.minimum )
	{
		return false;
	}

	return true;
}

std::string FloatRange::GetAsString() const
{
	std::string newString = Stringf( "%s~%s", std::to_string( minimum ).c_str(), std::to_string( maximum ).c_str() );
	return newString;
}

float FloatRange::GetRandomInRange( RandomNumberGenerator& rng ) const
{
	float randomNumberInRange = rng.RollRandomFloatInRange( minimum, maximum );
	return randomNumberInRange;
}

void FloatRange::Set( float newMinimum, float newMaximum )
{
	minimum = newMinimum;
	maximum = newMaximum;
}

bool FloatRange::SetFromText( const char* asText )
{
	std::string text( asText );
	Strings floatRange = SplitStringOnDelimiter( text,'~' );
	int numFloats = (int) floatRange.size();
	if( numFloats > 2 )
	{
		ERROR_AND_DIE( "FloatRange Initializer failure!" );
	}

	minimum = std::stof( floatRange[0] );
	maximum = std::stof( floatRange[1] );

	return false;
}
